using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using System.Windows.Data;
using System.Globalization;
using System.ComponentModel;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reflection;

namespace MoonTest.System.Windows.Data
{
	public class ClrPropertyNoDP : FrameworkElement {
		public int MyProperty {
			get;
			set;
		}
	}

	public class FakeConverter : TypeConverter {
		public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
		{
			return true;
		}
		public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
		{
			return true;
		}
		public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			return new MyControl ();
		}
		public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			return new MyControl ();
		}
	}

	public class AttachedDPPropertyClash : DependencyObject {
		public static DependencyProperty AttachedProperty =
			DependencyProperty.RegisterAttached("Attached", typeof(MyControl), typeof(AttachedDPPropertyClash), null);

		[TypeConverter (typeof (FakeConverter))]
		public MyControl Attached {
			get; set;
		}

		public static MyControl GetAttached(DependencyObject o)
		{
			return (MyControl) o.GetValue(AttachedProperty);
		}

		public static void SetAttached(DependencyObject o, MyControl value)
		{
			o.SetValue(AttachedProperty, value);
		}
	}

	public class CustomDependencyObject : DependencyObject
	{
		public static DependencyProperty WidthProperty =
			DependencyProperty.Register("Width", typeof(double), typeof(CustomDependencyObject), new PropertyMetadata(0.0));

		public double Width {
			get { return (double)GetValue(WidthProperty); }
			set { SetValue(WidthProperty, value); }
		}
	}

	public class DPWithDefaultValueDependencyObject : DependencyObject
	{
		public static readonly object Value = new object ();
		public static DependencyProperty DefaultValueProperty =
			DependencyProperty.Register ("DefaultValue", typeof (object), typeof (DPWithDefaultValueDependencyObject), new PropertyMetadata (Value));
	}

	public class DPWithDefaultValueFrameworkElement : FrameworkElement
	{
		public static readonly object Value = new object ();
		public static DependencyProperty DefaultValueProperty =
			DependencyProperty.Register ("DefaultValue", typeof (object), typeof (DPWithDefaultValueFrameworkElement), new PropertyMetadata (Value));
	}

	public class MentorElement : FrameworkElement
	{
		public static readonly DependencyProperty MentorProperty =
			DependencyProperty.Register("MentoredCollection", typeof(DependencyObjectCollection<object>), typeof(MentorElement), null);

		public DependencyObjectCollection<object> MentoredCollection {
			get { return (DependencyObjectCollection<object>)GetValue(MentorProperty); }
			set { SetValue(MentorProperty, value); }
		}
	}

	public class OpacityObject : FrameworkElement
	{
		public static DependencyProperty OpacityDPProperty =
			DependencyProperty.Register ("OpacityDP", typeof (double), typeof (OpacityObject), null);
		public double OpacityDP
		{
			get { return (double) GetValue (OpacityDPProperty); }
			set { SetValue (OpacityDPProperty, value); }
		}
	}

	public interface ILinked
	{
		ILinked Next { get; set; }
		double Value { get; set; }
	}

	public class NonINPC : ILinked
	{
		public ILinked Next {
			get; set;
		}

		public double Value {
			get; set;
		}
	}

	public class INPC : INotifyPropertyChanged, ILinked
	{
		ILinked next;
		double value;

		public double GetterException {
			get { throw new Exception ("BOOM!"); }
			set {  }
		}

		public double SetterException {
			get { return 100; }
			set { throw new ArgumentException(); }
		}

		public double Value
		{
			get { return value; }
			set
			{
				if (this.value != value) {
					this.value = value;
					if (PropertyChanged != null)
						PropertyChanged (this, new PropertyChangedEventArgs ("Value"));
				}
			}
		}

		public ILinked Next {
			get { return next; }
			set {
				if (next != value) {
					next = value;
					if (PropertyChanged != null)
						PropertyChanged (this, new PropertyChangedEventArgs ("Next"));
				}
			}
		}

		public event PropertyChangedEventHandler PropertyChanged;
	}

	public class TextProp : FrameworkElement
	{
		public static readonly DependencyProperty MyBindingExpressionProperty =
			DependencyProperty.Register ("MyBindingExpression", typeof (BindingExpression), typeof (TextProp), null);

		public BindingExpression MyBindingExpression
		{
			get { return (BindingExpression) GetValue (MyBindingExpressionProperty); }
			set { SetValue (MyBindingExpressionProperty, value); }
		}

		public static readonly DependencyProperty MyTextProperty = DependencyProperty.Register ("MyText", typeof (string), typeof (TextProp), null);
		public string MyText
		{
			get { return (string)GetValue (MyTextProperty); }
			set { SetValue (MyTextProperty, value); }
		}
	}
	
	public class OpacityTest
	{
		public OpacityTest ()
		{
			Opacity = 0.5f;
		}

		public float Opacity {
			get; set;
		}
	}

	public class UnbackedDPs : FrameworkElement
	{
		public static readonly DependencyProperty DPNoPropProperty = DependencyProperty.Register (
			"DPNoProp", typeof (int), typeof (UnbackedDPs), null);

		public static readonly DependencyProperty WrongPropertyName = DependencyProperty.Register (
			"SomeDP", typeof (int), typeof (UnbackedDPs), null);

		public static readonly DependencyProperty DPWithPropProperty = DependencyProperty.Register (
			"DPWithProp", typeof (int), typeof (UnbackedDPs), null);

		public UnbackedDPs ()
		{
			SetValue (DPNoPropProperty, 5);
			SetValue (WrongPropertyName, 5);
			SetValue (DPWithPropProperty, 5);
		}
	}

	public class Data
	{
		public Brush Brush
		{
			get;
			set;
		}
		public Data InnerData
		{
			get;
			set;
		}
		public double Opacity
		{
			get;
			set;
		}
        public bool ThrowExceptionsOnUpdate {
            get { return false; }
            set { throw new Exception("Testing"); }
        }
		public List<double> DoubleList { get; set; }
		public IList IList { get; set; }
		public IList ObservableDoubles { get; set; }

		public Data()
		{
			Brush = new SolidColorBrush(Colors.Brown);
			Opacity = 0.5f;
			DoubleList = new List<double> ();
			IList = new List<double> ();
			ObservableDoubles = new ObservableCollection<double> ();
		}
	}

	[TestClass]
	public partial class BindingTest : SilverlightTest
	{
		class CustomControl : UserControl
		{
			public new UIElement Content
			{
				get { return base.Content; }
				set { base.Content = value; }
			}

			public CustomControl ()
			{
				
			}
		}

		class InternalData
		{
			public Brush Brush {
				get; set;
			}
			public Data InnerData {
				get; set;
			}
			public double Opacity {
				get; set;
			}

			public InternalData ()
			{
				Brush = new SolidColorBrush(Colors.Brown);
				Opacity = 0.5;
			}
		}

		class InheritedData : Data
		{
			public double Float {
				get; set;
			}

			public InheritedData()
			{
				Float = 0.2;
			}
		}


		public class PropertyUpdater : INotifyPropertyChanged
		{
			public event PropertyChangedEventHandler PropertyChanged;

			private float opacity;
			public bool Get;
			public bool Set;

			public void Reset ()
			{
				Get = false;
				Set = false;
			}

			public float Opacity
			{
				get { Get = true; return opacity; }
				set {
					Set = true;
					opacity = value;
					if (PropertyChanged != null)
						PropertyChanged (this, new PropertyChangedEventArgs ("Opacity"));
				}
			}

			public PropertyUpdater()
			{
				opacity = 0.5f;
			}
		}

		public class ContainsPropertyUpdater
		{
			public PropertyUpdater Updater {
				get;set;
			}

			public ContainsPropertyUpdater ()
			{
				Updater = new PropertyUpdater ();
			}
		}

		public class TargetClass : Control {
			bool propertyChanged;

			public bool GetterCalled {
				get; set;
			}

			public bool SetterCalled {
				get; set;
			}

			public static readonly DependencyProperty TestProperty =
				DependencyProperty.Register ("Test", typeof (string), typeof (TargetClass),
							     new PropertyMetadata (null, new PropertyChangedCallback (TestPropertyChanged)));

			static void TestPropertyChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
			{
				(sender as TargetClass).OnTestPropertyChanged (e.OldValue as string,
									       e.NewValue as string);
			}

			void OnTestPropertyChanged (string oldValue, string newValue)
			{
				propertyChanged = true;
			}

			public string Test {
				get { GetterCalled = true; return (string)GetValue (TestProperty); }
				set { SetterCalled = true; SetValue (TestProperty, value); }
			}

			public void ClearPropertyChanged ()
			{
				propertyChanged = false;
			}

			public bool GetPropertyChanged ()
			{
				return propertyChanged;
			}
		}

		[TestMethod]
		public void AttachedProperty_Invalid ()
		{
			var data = new Rectangle ();
			Canvas.SetTop (data, 100);

			var target = new Rectangle ();
			target.SetBinding (Rectangle.WidthProperty, new Binding {
				Path = new PropertyPath ("Canvas.Top)"),
				Source = data,
			});
			Assert.IsTrue (double.IsNaN (target.Width), "#1");
		}

		[TestMethod]
		public void AttachedProperty_Invalid2 ()
		{
			// Invalid binding paths
			var data = new Rectangle ();
			Canvas.SetTop (data, 100);

			var target = new Rectangle ();
			Assert.Throws<ArgumentException> (() => {
				target.SetBinding (Rectangle.WidthProperty, new Binding {
					Path = new PropertyPath ("(Canvas.Top"),
					Source = data,
				});
			});
		}

		[TestMethod]
		public void AttachedProperty_Invalid3 ()
		{
			// Invalid binding paths
			var data = new Rectangle ();
			Canvas.SetTop (data, 100);

			var target = new Rectangle ();
			Assert.Throws<ArgumentException> (() => {
				target.SetBinding (Rectangle.WidthProperty, new Binding {
					Path = new PropertyPath ("(CanvasTop"),
					Source = data,
				});
			});
		}

		[TestMethod]
		public void AttachedProperty_Invalid4 ()
		{
			// Invalid binding paths
			var data = new Rectangle ();
			Canvas.SetTop (data, 100);

			var target = new Rectangle ();
			Assert.Throws<Exception> (() => {
				target.SetBinding (Rectangle.WidthProperty, new Binding {
					Path = new PropertyPath ("(CanvasTop)"),
					Source = data,
				});
			});
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#1 is failing bcause the objects are the same")]
		public void AttachedProperty_ClashWithCLRProperty_sl2()
		{
			// Check to ensure we do *not* use the type converter
			// declared on the CLR property as this is an attached DP.
			var dp = AttachedDPPropertyClash.AttachedProperty;
			var value = new MyControl();
			var source = new AttachedDPPropertyClash();
			var target = new Rectangle ();
			source.SetValue(dp, value);

			BindingOperations.SetBinding(target, Rectangle.DataContextProperty, new Binding("Attached") { Mode = BindingMode.TwoWay, Source = source });
			Assert.AreNotSame (source.GetValue (dp), target.DataContext, "#1");

			target.DataContext = 123;
			Assert.AreSame(value, source.GetValue(dp), "#2");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void AttachedProperty_ClashWithCLRProperty_sl3()
		{
			// Check to ensure we do *not* use the type converter
			// declared on the CLR property as this is an attached DP.
			var dp = AttachedDPPropertyClash.AttachedProperty;
			var value = new MyControl();
			var source = new AttachedDPPropertyClash();
			var target = new Rectangle ();
			source.SetValue(dp, value);

			BindingOperations.SetBinding(target, Rectangle.DataContextProperty, new Binding("Attached") { Mode = BindingMode.TwoWay, Source = source });
			Assert.AreSame (source.GetValue (dp), target.DataContext, "#1");

			target.DataContext = 123;
			Assert.AreSame(value, source.GetValue(dp), "#2");
		}

		[TestMethod]
		public void AttachedProperty_Simple ()
		{
			var data = new Rectangle ();
			Canvas.SetTop (data, 100);

			var target = new Rectangle ();
			target.SetBinding (Rectangle.WidthProperty, new Binding {
				Path = new PropertyPath ("(Canvas.Top)"),
				Source = data,
			});
			Assert.AreEqual (100, target.Width, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void AttachedProperty_CLR_FullyQualified ()
		{
			// You can't specify the fully qualified object type
			var data = new OpacityObject ();
			Canvas.SetTop (data, 100);

			Assert.Throws<Exception> (() => {
				new Binding {
					Path = new PropertyPath ("(MoonTest.System.Windows.Data.OpacityObject.Attached)"),
					Source = data,
				};
			});
		}

		[TestMethod]
		[MoonlightBug]
		public void AttachedProperty_CLR ()
		{
			// You can't specify just the class name unless it's
			// been registered before somehow. Probably has to be registered
			// through XAML.
			var data = new OpacityObject ();
			Canvas.SetTop (data, 100);

			Assert.Throws<Exception> (() => {
				new Binding {
					Path = new PropertyPath ("(OpacityObject.Attached)"),
					Source = data,
				};
			});
		}

		[TestMethod]
		[Asynchronous]
		[Ignore]
		public void BindContentPresenterContent ()
		{
			ContentPresenter presenter = new ContentPresenter ();
			presenter.SetBinding (ContentPresenter.ContentProperty, new Binding ("Opacity"));

			CustomControl c = new CustomControl { Content = presenter };
			CreateAsyncTest (c,
				() => {
					c.DataContext = new Data { Opacity = 1.0 };
				}, () => {
					Assert.AreEqual (1.0, presenter.ReadLocalValue (ContentPresenter.DataContextProperty), "#1");
					Assert.AreEqual (1.0, presenter.Content, "#2");

					c.DataContext = new Data { Opacity = 0.0 };
				}, () => {
					Assert.AreEqual (0.0, presenter.ReadLocalValue (ContentPresenter.DataContextProperty), "#3");
					Assert.AreEqual (0.0, presenter.Content, "#4");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void BindDataContext ()
		{
			// Bind the DataContext of the FE to its DataContext
			TextBlock block = new TextBlock ();
			block.SetBinding (TextBlock.DataContextProperty, new Binding ());
			CreateAsyncTest (block,
				() => Assert.IsNull (block.DataContext, "#1"),
				() => TestPanel.DataContext = "Hello",
				() => {
					Assert.AreEqual ("Hello", block.DataContext, "#2");
					Assert.IsInstanceOfType<BindingExpressionBase> (block.ReadLocalValue (TextBlock.DataContextProperty), "#3");
				}
			);
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#2 is failing because block.Text == 'Yarr'")]
		public void BindDpToDp_sl2 ()
		{
			var data = new TextProp { MyText = "Hello" };
			TextBlock block = new TextBlock ();
			block.SetBinding (TextBlock.TextProperty,
				new Binding {
					Path = new PropertyPath ("MyText"),
					Mode = BindingMode.TwoWay,
					Source = data,
				}
			);

			Assert.AreEqual ("Hello", block.Text, "#1");

			data.MyText = "Yarr";
			Assert.AreEqual ("Hello", block.Text, "#2");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void BindDpToDp_sl3 ()
		{
			var data = new TextProp { MyText = "Hello" };
			TextBlock block = new TextBlock ();
			block.SetBinding (TextBlock.TextProperty,
				new Binding {
					Path = new PropertyPath ("MyText"),
					Mode = BindingMode.TwoWay,
					Source = data,
				}
			);

			Assert.AreEqual ("Hello", block.Text, "#1");

			data.MyText = "Yarr";
			Assert.AreEqual ("Yarr", block.Text, "#2");
		}

		[TestMethod]
		public void BindDpToDp_BindingExpressionType ()
		{
			// See what happens if we have a twoway binding of property type 'BindingExpressionBase'.
			// Normally the value would be copied to the datasource, but in this case we just replace
			// the existing twoway binding.
			var data = new TextProp { };
			var target = new TextProp { };

			var binding = target.SetBinding (TextProp.WidthProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("Opacity"),
				Source = new OpacityObject { Opacity = 0.5 },
			});

			target.ClearValue (TextProp.WidthProperty);
			target.SetBinding (TextProp.MyBindingExpressionProperty, new Binding {
				Source = data,
				Path = new PropertyPath ("MyBindingExpression"),
				Mode = BindingMode.TwoWay,
			});

			target.SetValue (TextProp.MyBindingExpressionProperty, binding);
			Assert.AreSame (binding, target.ReadLocalValue (TextProp.MyBindingExpressionProperty), "#1");
		}

		// FIXME: I don't know why this happens
		[TestMethod]
		[MoonlightBug]
		public void BindToDP_WrongDPName_WithProperty ()
		{
			var data = new UnbackedDPs ();
			var binding = new Binding ();
			binding.Source = data;
			Assert.Throws<Exception> (() =>
				binding.Path = new PropertyPath (UnbackedDPs.WrongPropertyName)
			);

		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#1 is failing")]
		public void BindToDP_WrongDPName_WithName_sl2 ()
		{
			var data = new UnbackedDPs ();
			var rect = new Rectangle ();
			rect.SetBinding (Rectangle.WidthProperty, new Binding {
				Path = new PropertyPath ("SomeDP"),
				Source = data,
			});
			Assert.IsTrue (Double.IsNaN (rect.Width), "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void BindToDP_WrongDPName_WithName_sl3 ()
		{
			var data = new UnbackedDPs ();
			var rect = new Rectangle ();
			rect.SetBinding (Rectangle.WidthProperty, new Binding {
				Path = new PropertyPath ("SomeDP"),
				Source = data,
			});
			Assert.AreEqual (5, rect.Width, "#1");
		}

		[TestMethod]
		public void BindToDP_WrongDPName_WithWrongName ()
		{
			var data = new UnbackedDPs ();
			var rect = new Rectangle ();
			rect.SetBinding (Rectangle.WidthProperty, new Binding {
				Path = new PropertyPath ("WrongPropertyName"),
				Source = data,
			});

			Assert.IsTrue(Double.IsNaN (rect.Width), "#1");
		}

		[TestMethod]
		public void DPWithDefaultValueFE_Broken_DoNotUseFallback ()
		{
			var o = new DPWithDefaultValueFrameworkElement { };
			BindingOperations.SetBinding (o, DPWithDefaultValueFrameworkElement.DefaultValueProperty, new Binding { FallbackValue = "Foo" });
			Assert.AreEqual (DPWithDefaultValueFrameworkElement.Value, o.GetValue (DPWithDefaultValueFrameworkElement.DefaultValueProperty), "#1");
		}

		[TestMethod]
		public void DPWithDefaultValueFE_UseTargetNull ()
		{
			var o = new DPWithDefaultValueFrameworkElement { DataContext = new Rectangle { } };
			BindingOperations.SetBinding (o, DPWithDefaultValueFrameworkElement.DefaultValueProperty, new Binding ("DataContext") { });
			Assert.AreEqual (null, o.GetValue (DPWithDefaultValueFrameworkElement.DefaultValueProperty), "#1");
		}

		[TestMethod]
		public void DPWithDefaultValueFE_Broken_UseTargetNull ()
		{
			var o = new DPWithDefaultValueFrameworkElement { };
			BindingOperations.SetBinding (o, DPWithDefaultValueFrameworkElement.DefaultValueProperty, new Binding { TargetNullValue = "Foo" });
			Assert.AreEqual ("Foo", o.GetValue (DPWithDefaultValueFrameworkElement.DefaultValueProperty), "#1");
		}

		[TestMethod]
		public void DPWithDefaultValueDO_Broken_DoNotUseFallback ()
		{
			var o = new DPWithDefaultValueDependencyObject { };
			BindingOperations.SetBinding (o, DPWithDefaultValueDependencyObject.DefaultValueProperty, new Binding { FallbackValue = "Foo" });
			Assert.AreEqual (DPWithDefaultValueDependencyObject.Value, o.GetValue (DPWithDefaultValueDependencyObject.DefaultValueProperty), "#1");
		}

		[TestMethod]
		public void DPWithDefaultValueDO_Broken_UseTargetNull ()
		{
			var o = new DPWithDefaultValueDependencyObject { };
			BindingOperations.SetBinding (o, DPWithDefaultValueDependencyObject.DefaultValueProperty, new Binding { TargetNullValue = "Foo" });
			Assert.AreEqual ("Foo", o.GetValue (DPWithDefaultValueDependencyObject.DefaultValueProperty), "#1");
		}

		[TestMethod]
		[MinRuntimeVersion (4)]
		public void BindToDPUsingAttributeSyntax_NoCLRWrapper_SL4 ()
		{
			Assert.Throws<XamlParseException> (() =>
				XamlReader.Load  (@"	
<Canvas
	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:my=""clr-namespace:MoonTest.System.Windows.Data;assembly=moon-unit"">
	<Canvas.Resources>
		<my:UnbackedDPs x:Name=""CLRObject"" DPNoProp=""{Binding Path=Test}"" />
	</Canvas.Resources>
</Canvas>
"));
		}

		[TestMethod]
		[MaxRuntimeVersion (3)]
		public void BindToDPUsingAttributeSyntax_NoCLRWrapper_SL3 ()
		{
			var c = (Canvas) XamlReader.Load (@"	
<Canvas
	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:my=""clr-namespace:MoonTest.System.Windows.Data;assembly=moon-unit"" >
	<Canvas.Resources>
		<my:UnbackedDPs x:Name=""CLRObject"" DPNoProp=""{Binding Path=Test}"" />
	</Canvas.Resources>
</Canvas>
");
			var obj = (c.Resources["CLRObject"] as DependencyObject);
			Assert.IsInstanceOfType<BindingExpression> (obj.ReadLocalValue (UnbackedDPs.DPNoPropProperty), "#1");
		}

		[TestMethod]
		public void BindToText ()
		{
			Binding binding = new Binding ("");

			TextBox box = new TextBox ();
			box.SetBinding (TextBox.TextProperty, binding);
			box.DataContext = 0.5f;
			Assert.AreEqual ("0.5", box.Text, "#1");
			box.DataContext = 0.0;
			Assert.AreEqual ("0", box.Text, "#2");
			box.DataContext = new object ();
			Assert.AreEqual ("System.Object", box.Text, "#3");
		}
		
		[TestMethod]
		public void BindToText2 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0 };
			Binding binding = new Binding ("Opacity");
			
			TextBox box = new TextBox ();
			box.DataContext = data;
			box.SetBinding (TextBox.TextProperty, binding);

			data.Opacity = 0.5f;
			Assert.AreEqual ("0.5", box.Text, "#1");
			data.Opacity = 0.0f;
			Assert.AreEqual ("0", box.Text, "#2");
		}

		[TestMethod]
		public void BindToText3 ()
		{
			Binding binding = new Binding ("");
			binding.Source = "string";
			TextBox box = new TextBox ();
			box.SetBinding (TextBox.TextProperty, binding);

			Assert.AreEqual ("string", box.Text, "#1");
		}

		[TestMethod]
		public void BindToText5 ()
		{
			// Fails in Silverlight 3
			Binding binding = new Binding ("Prop");
			binding.Source = "string";
			TextProp prop = new TextProp ();
			prop.SetBinding (TextProp.MyTextProperty, binding);
			Assert.IsNull (prop.MyText, "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void BindXaml ()
		{
			// Fails in Silverlight 3
			Mono.Moonlight.BindingConverter c = new Mono.Moonlight.BindingConverter ();
			Grid p = (Grid) c.Root;
			Canvas canvas = (Canvas) p.Children [0];
			p.Children.Clear ();
			TestPanel.Children.Add (canvas);
			Enqueue (() => {
				Assert.AreEqual ("Thursday, February, 2009", ((TextBlock)canvas.Children[0]).Text, "#1");
				Assert.AreEqual ("converter-string", ((TextBlock)canvas.Children[1]).Text, "#2");
				Assert.AreEqual ("converter-string", ((TextBlock)canvas.Children[2]).Text, "#3");
				Assert.AreEqual ("converter-object", ((TextBlock)canvas.Children[3]).Text, "#4");
				Assert.AreEqual ("", ((TextBlock)canvas.Children[4]).Text, "#5");
				Assert.AreEqual ("null-value", ((TextBlock) canvas.Children [5]).Text, "#6");
				Assert.AreEqual ("", ((TextBlock) canvas.Children [6]).Text, "#7");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		public void ConstructorTest()
		{
			Binding binding = new Binding ();
			Assert.IsNull (binding.Converter);
			Assert.AreEqual (null, binding.ConverterCulture);
			Assert.IsNull (binding.ConverterParameter);
			Assert.AreEqual (BindingMode.OneWay, binding.Mode);
			Assert.IsFalse (binding.NotifyOnValidationError);
			Assert.IsNotNull (binding.Path);
			Assert.AreNotEqual (new PropertyPath (""), binding.Path);
			Assert.AreEqual (new PropertyPath ("").Path, binding.Path.Path);
			Assert.IsNull (binding.Source);
			Assert.IsFalse(binding.ValidatesOnDataErrors, "#ValidatesOnDataErrors");
			Assert.IsFalse(binding.ValidatesOnExceptions, "#ValidatesOnExceptions");
			Assert.IsTrue(binding.ValidatesOnNotifyDataErrors, "#ValidatesOnNotifyDataErrors");

			binding = new Binding ("Path.To.Prop");
			Assert.AreEqual ("Path.To.Prop", binding.Path.Path);
		}

		[TestMethod]
		public void DataContext_Precedence ()
		{
			Binding b = new Binding("");

			Canvas c = new Canvas();
			c.DataContext = new SolidColorBrush(Colors.Blue);

			Rectangle r = new Rectangle();
			r.DataContext = new SolidColorBrush(Colors.Green);

			c.Children.Add(r);
			r.SetBinding(Rectangle.FillProperty, b);

			// the local DataContext takes precedent.
			Assert.AreEqual(r.Fill, r.DataContext, "#1");
		}


		[TestMethod]
		public void DataContext_NullLocal ()
		{
			Binding b = new Binding("");

			Canvas c = new Canvas();
			c.DataContext = new SolidColorBrush(Colors.Blue);

			Rectangle r = new Rectangle();
			r.DataContext = new SolidColorBrush(Colors.Green);

			c.Children.Add(r);
			r.SetBinding(Rectangle.FillProperty, b);

			// a null DataContext is still a local value,
			// and takes precedence over the inherited
			// value.
			r.DataContext = null;
			Assert.AreEqual(null, r.Fill, "#1");
		}
		
		[TestMethod]
		public void DataContext_Applied ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"	
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
	 DataContext=""100"" >
	<Ellipse Width=""{Binding Source=200}"" Height=""{Binding Mode=OneTime}"" />
</Canvas>
");
			Ellipse e = (Ellipse) c.Children [0];
			Assert.IsTrue (double.IsNaN (e.Height), "#1");
			TestPanel.Children.Add (c);
			Assert.AreEqual (100, e.Height, "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void DataContext_Applied2 ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"	
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
	 DataContext=""100"" >
	<Ellipse Width=""{Binding Source=200}"" Height=""{Binding Mode=OneTime}"" />
</Canvas>
");
			Ellipse e = (Ellipse) c.Children [0];
			e.Loaded += delegate { Assert.AreEqual (100, e.Height, "#3"); };
			Assert.IsTrue (double.IsNaN (e.Height), "#2");

			CreateAsyncTest (c,
				() => Assert.AreEqual (100, e.Height, "#1")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContext_Applied3 ()
		{
			// Create an ellipse which will bind to the parent datacontext
			Ellipse e = new Ellipse ();
			e.SetBinding (Ellipse.HeightProperty, new Binding ());

			// Check to see if the binding is applied correctly after the FE is loaded
			Canvas c = new Canvas { DataContext = 100.0 };
			c.Children.Add (e);
			Assert.IsTrue (double.IsNaN (e.Height), "#1");

			CreateAsyncTest (c,
				() => Assert.AreEqual (100, e.Height, "#2")
			);
		}

		[TestMethod]
		public void DataContext_ChangeParentOneWay ()
		{
			Canvas canvas = new Canvas ();
			PropertyUpdater updater = new PropertyUpdater { Opacity = 0 };
			Binding binding = new Binding ("Opacity");
			Rectangle rectangle = new Rectangle { Name = "TED" };

			canvas.DataContext = updater;
			canvas.Children.Add (rectangle);
			rectangle.SetBinding (Rectangle.OpacityProperty, binding);

			Assert.AreSame (rectangle.DataContext, canvas.DataContext, "#1");
			updater.Opacity = 0;
			Assert.AreEqual (0, rectangle.Opacity, "#2");

			canvas.DataContext = null;
			Assert.AreEqual (1, rectangle.Opacity, "#3");
			updater.Opacity = 0.5f;
			Assert.AreEqual (1, rectangle.Opacity, "#4");
		}

		[TestMethod]
		public void DataContext_ClearLocal ()
		{
			Binding b = new Binding("");

			Canvas c = new Canvas();
			c.DataContext = new SolidColorBrush(Colors.Blue);

			Rectangle r = new Rectangle();
			r.DataContext = new SolidColorBrush(Colors.Green);

			c.Children.Add(r);
			r.SetBinding(Rectangle.FillProperty, b);

			// clearing the value allows the inherited
			// DataContext to be used again (and causes
			// the bound property to be updated)
			r.ClearValue (FrameworkElement.DataContextProperty);
			Assert.AreEqual(c.DataContext, r.DataContext, "#1");
			Assert.AreEqual(c.DataContext, r.Fill, "#2");
		}

		[TestMethod]
		public void DataContext_SetBindingSource ()
		{
			Binding b = new Binding("");

			Canvas c = new Canvas();
			c.DataContext = new SolidColorBrush(Colors.Blue);

			Rectangle r = new Rectangle();
			r.DataContext = new SolidColorBrush(Colors.Green);

			c.Children.Add(r);

			// set the source of the Binding object
			b = new Binding ("");
			b.Source = new SolidColorBrush (Colors.Yellow);
			r.SetBinding(Rectangle.FillProperty, b);
			Assert.AreEqual(b.Source, r.Fill, "#1");

			// now set the data context, and show that the
			// Binding.Source has precedence.
			r.DataContext = new LinearGradientBrush();
			Assert.AreEqual(b.Source, r.Fill, "#2");
		}
		
		[TestMethod]
		public void ConstructorTest2 ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				Binding binding = new Binding (null);
				GC.KeepAlive (binding);
			});
		}

		[TestMethod]
		public void BasicBind ()
		{
			Rectangle rectangle = new Rectangle ();
			Binding binding = new Binding ("Opacity");
			binding.Source = new Data { Opacity = 0.0 };

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			
			Assert.AreEqual (0.0, rectangle.Opacity, "#1");
			Assert.IsTrue (rectangle.ReadLocalValue (Rectangle.OpacityProperty) is BindingExpressionBase, "#2");
			rectangle.Opacity = 1.0;
			Assert.AreEqual(1.0, rectangle.Opacity, "#3");
			Assert.AreEqual (1.0, rectangle.ReadLocalValue (Rectangle.OpacityProperty), "#4");
			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.IsTrue (rectangle.ReadLocalValue (Rectangle.OpacityProperty) is BindingExpressionBase, "#5");
			Assert.AreEqual (0.0, rectangle.Opacity, "#6");
			rectangle.ClearValue (Rectangle.OpacityProperty);
			Assert.AreEqual (1.0, rectangle.Opacity, "#7");
			
			Assert.AreEqual (DependencyProperty.UnsetValue, rectangle.ReadLocalValue (Rectangle.OpacityProperty), "#8");
		}

		[TestMethod]
		public void BindFloatToDouble ()
		{
			Binding binding = new Binding ("Opacity");
			binding.Source = new OpacityTest { Opacity = 0.5f };
			Rectangle r = new Rectangle ();
			r.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.IsBetween (0.499, 0.5001, r.Opacity, "#1");
		}

		[TestMethod]
		public void BindPropertyWhichIsINotifyPropChanged ()
		{
			// This tests which object we attach INotifyPropertyChanged handlers to.
			// 'data' is our source but 'data.Updater' is the object which has the
			// property we've databound to. In this case, we attach our INPC handlers
			// to 'data.Updater'
			var data = new ContainsPropertyUpdater ();
			Rectangle rectangle = new Rectangle ();
			rectangle.SetBinding (Rectangle.OpacityProperty, new Binding ("Updater.Opacity"));
			rectangle.DataContext = data;
			data.Updater.Opacity = 0.0f;
			Assert.AreEqual (0.0, rectangle.Opacity, "#1");

			data.Updater.Opacity = 1;
			Assert.AreEqual (1, rectangle.Opacity, "#2");
		}

		[TestMethod]
		public void BindRectangle ()
		{
			Data data = new Data ();
			data.InnerData = new Data { Opacity = 1.0f };

			Rectangle rectangle = new Rectangle { Opacity = 0f };
			rectangle.DataContext = data;

			Binding binding = new Binding ("InnerData.Opacity");
			rectangle.SetBinding (Shape.OpacityProperty, binding);
			Assert.AreEqual (data.InnerData.Opacity, rectangle.Opacity, "#1");

			binding = new Binding ("Opacity");
			rectangle.SetBinding (Shape.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, rectangle.Opacity, "#2");
		}

		[TestMethod]
		public void BindInternalClass ()
		{
			InternalData data = new InternalData ();
			data.InnerData = new Data { Opacity = 1.0f };

			Rectangle rectangle = new Rectangle { Opacity = 0f };
			rectangle.DataContext = data;

			Binding binding = new Binding ("InnerData.Opacity");
			rectangle.SetBinding (Shape.OpacityProperty, binding);
		}

		[TestMethod]
		public void BindInheritedClass ()
		{
			InheritedData data = new InheritedData ();
			data.InnerData = new Data { Opacity = 1.0f };

			Rectangle rectangle = new Rectangle { Opacity = 0f };
			rectangle.DataContext = data;

			Binding binding = new Binding ("InnerData.Opacity");
			rectangle.SetBinding (Shape.OpacityProperty, binding);
			Assert.AreEqual (data.InnerData.Opacity, rectangle.Opacity, "#1");

			binding = new Binding ("Float");
			rectangle.SetBinding(Shape.OpacityProperty, binding);
		}

		[TestMethod]
		public void BindToInheritedProperty_AfterLiveTree_Inheritance()
		{
			// The 'Langauge' property is used if the binding culture isn't set
			TestPanel.Language = XmlLanguage.GetLanguage("en-GB");

			var target = new TextBlock();
			TestPanel.Children.Add(target);

			Assert.AreEqual(TestPanel.Language.IetfLanguageTag, target.Language.IetfLanguageTag, "#1");
		}

		[TestMethod]
		public void BindToInheritedProperty_AfterLiveTree()
		{
			// The 'Langauge' property is used if the binding culture isn't set
			TestPanel.Language = XmlLanguage.GetLanguage("en-GB");
			var binding = new Binding("") { StringFormat = "C", Source = 16.00 };

			var target = new TextBlock();
			TestPanel.Children.Add(target);
			target.SetBinding(TextBlock.TextProperty, binding);

			Assert.AreEqual("£16.00", target.Text, "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void BindToInheritedProperty_BeforeLiveTree()
		{
			// If we set the binding first nothing causes it to refresh so we don't pick up
			// the Language
			TestPanel.Language = XmlLanguage.GetLanguage("en-GB");
			var binding = new Binding("") { StringFormat = "C", Source = 16.00 };

			var target = new TextBlock();
			target.SetBinding(TextBlock.TextProperty, binding);
			TestPanel.Children.Add(target);

			Assert.AreEqual("$16.00", target.Text, "#1");
			Enqueue(() => Assert.AreEqual("$16.00", target.Text, "#2"));
			Enqueue(() => {
				Assert.AreEqual("$16.00", target.Text, "#3");
			});
			EnqueueTestComplete();
		}

		[TestMethod]
		public void BindToInheritedProperty_BeforeLiveTree_DataContext ()
		{
			// If we set the binding first nothing causes it to refresh so we don't pick up
			// the Language
			TestPanel.DataContext = 16.00;
			TestPanel.Language = XmlLanguage.GetLanguage("en-GB");
			var binding = new Binding("") { StringFormat = "C" };

			var target = new TextBlock();
			target.SetBinding(TextBlock.TextProperty, binding);
			TestPanel.Children.Add(target);

			Assert.AreEqual("£16.00", target.Text, "#1");
		}

		[TestMethod]
		public void ChangeSourceValue()
		{
			Data data = new Data { Opacity = 0.5 };
			Rectangle r = new Rectangle();
			r.SetBinding(Rectangle.OpacityProperty, new Binding { Path = new PropertyPath("Opacity"), Source = data });
			Assert.AreEqual(data.Opacity, r.Opacity, "#1");
			data.Opacity = 0;
			Assert.AreNotEqual(data.Opacity, r.Opacity, "#2");
		}

		[TestMethod]
		[MoonlightBug ("SL3 appears to try to look up 'Opacity' on the source object as soon as the property path is set but there is no object")]
		public void CreateWithDP ()
		{
			Binding b = new Binding ();
			// Setting a property path with a DP or invalid path throws?
			Assert.Throws<Exception> (() => 
				b.Path = new PropertyPath (Rectangle.OpacityProperty)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug ("SL3 throws an exception if the binding Path is invalid")]
		public void CreateWithInvalidPath ()
		{
			Assert.Throws<ArgumentException> (() =>
				new Binding (" Some crazy path")
			, "#1");
		}

		[TestMethod]
		public void CreateWithDottedPath ()
		{
			// This is fine
			new Binding ("Some.crazy.path");
		}

		[TestMethod]
		[Asynchronous]
		public void MentorTest_NotifyOnBindingException()
		{
			bool errored = false;
			var source = new INPC();
			var target = new CustomDependencyObject ();
			var mentor = new MentorElement { DataContext = source, MentoredCollection = new DependencyObjectCollection<object>() };
			mentor.MentoredCollection.Add(target);
			TestPanel.Children.Add(mentor);
			var binding = new Binding("SetterException") {
				Mode = BindingMode.TwoWay,
				ValidatesOnExceptions = true,
				NotifyOnValidationError = true
			};
			BindingOperations.SetBinding(target, CustomDependencyObject.WidthProperty, binding);
			Assert.AreEqual(100.0, target.Width, "#1");

			mentor.BindingValidationError += (o, e) => errored = true;
			Enqueue(() => {
				target.Width = 222;
				Assert.IsTrue(errored, "#2");
			});
			EnqueueTestComplete();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void ElementName_MentorBased_AttachMentorAfterBinding()
		{
			var source = TestPanel;
			var target = new PlaneProjection();
			var binding = new Binding("Width") { ElementName = "source" };

			source.Name = "source";
			source.Width = 100;

			BindingOperations.SetBinding(target, PlaneProjection.RotationZProperty, binding);
			source.Projection = target;
			EnqueueCallback(() => Assert.AreNotEqual(source.Width, target.RotationZ, "#1"));
			EnqueueTestComplete();
		}


		[TestMethod]
		[Asynchronous]
		public void ElementName_BeforeAddToTree ()
		{
			var source = new Rectangle {
				Name = "Source",
				Width = 100,
			};
			var target = new Rectangle {
				Name = "Target"
			};

			target.SetBinding (Rectangle.WidthProperty, new Binding {
				ElementName = "Source",
				Path = new PropertyPath ("Width"),
				Mode = BindingMode.OneWay,
			});

			TestPanel.Children.Add (target);

			CreateAsyncTest (source, () => {
				Assert.AreEqual (100, target.Width, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ElementName_BeforeAddToTree_2 ()
		{
			var source = new Rectangle {
				Name = "Source",
				Width = 100,
			};
			var target = new Rectangle {
				Name = "Target"
			};

			target.SetBinding (Rectangle.WidthProperty, new Binding {
				ElementName = "Source",
				Path = new PropertyPath ("Width"),
				Mode = BindingMode.OneWay,
			});


			CreateAsyncTest (target,
				() => {
					TestPanel.Children.Add (source);
				}, () => {
					Assert.IsTrue (double.IsNaN (target.Width), "#1");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ElementName_AfterAddToTree ()
		{
			var source = new Rectangle {
				Name = "Source",
				Width = 100,
			};
			var target = new Rectangle {
				Name = "Target"
			};

			target.SetBinding (Rectangle.WidthProperty, new Binding {
				ElementName = "Source",
				Path = new PropertyPath ("Width"),
				Mode = BindingMode.OneWay,
			});

			TestPanel.Children.Add (source);
			CreateAsyncTest (target, () => {
				Assert.AreEqual (100, target.Width, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ElementName_AfterAddToTree_2 ()
		{
			var source = new Rectangle {
				Name = "Source",
				Width = 100,
			};
			var target = new Rectangle {
				Name = "Target"
			};

			target.SetBinding (Rectangle.WidthProperty, new Binding {
				ElementName = "Source",
				Path = new PropertyPath ("Width"),
				Mode = BindingMode.OneWay,
			});

			CreateAsyncTest (source, () => {
				TestPanel.Children.Add (target);
			}, () => {
				Assert.AreEqual (100, target.Width, "#2");
			});
		}

		[TestMethod]
		public void DOBinding_Basic ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode=BindingMode.TwoWay,
			});
			Assert.AreEqual (Colors.Red.ToString (), target.Color.ToString (), "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#1 is failing - the color is still Blue")]
		public void DOBinding_OneWay_SourceToTarget_sl2 ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode = BindingMode.OneWay,
			});
			source.Color = Colors.Blue;
			Assert.AreEqual (Colors.Red.ToString (), target.Color.ToString (), "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void DOBinding_OneWay_SourceToTarget_sl3 ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode = BindingMode.OneWay,
			});
			source.Color = Colors.Blue;
			Assert.AreEqual (Colors.Blue.ToString (), target.Color.ToString (), "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#1 is failing - the color is still Blue")]
		public void DOBinding_TwoWay_SourceToTarget_sl2 ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode = BindingMode.TwoWay,
			});
			source.Color = Colors.Blue;
			Assert.AreEqual (Colors.Red.ToString (), target.Color.ToString (), "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void DOBinding_TwoWay_SourceToTarget_sl3 ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode = BindingMode.TwoWay,
			});
			source.Color = Colors.Blue;
			Assert.AreEqual (Colors.Blue.ToString (), target.Color.ToString (), "#1");
		}

		[TestMethod]
		public void DOBinding_TwoWay_TargetToSource ()
		{
			var source = new SolidColorBrush (Colors.Red);
			var target = new SolidColorBrush ();
			BindingOperations.SetBinding (target, SolidColorBrush.ColorProperty, new Binding ("Color") {
				Source = source,
				Mode = BindingMode.TwoWay,
			});
			target.Color = Colors.Blue;
			Assert.AreEqual (Colors.Blue.ToString (), source.Color.ToString (), "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		[MoonlightBug ("#2 is failing")]
		public void DPSetValuePreferred_TwoWay_sl2()
		{
			var str = "test_string";
			var source = new TargetClass ();
			source.SetValue(TargetClass.TestProperty, str);
			var target = new TargetClass();
			target.SetBinding(TargetClass.TestProperty, new Binding("Test") { Source = source, Mode = BindingMode.TwoWay });

			Assert.IsFalse(source.SetterCalled, "#1");
			Assert.IsTrue (source.GetterCalled, "#2"); /* sl2 */
			Assert.IsFalse(target.SetterCalled, "#3");
			Assert.IsFalse(target.GetterCalled, "#4");

			source.SetValue(TargetClass.TestProperty, "otherstring");
			Assert.IsFalse(source.SetterCalled, "#5");
			Assert.IsTrue(source.GetterCalled, "#6"); /* sl2 */
			Assert.IsFalse(target.SetterCalled, "#7");
			Assert.IsFalse(target.GetterCalled, "#8");

			target.SetValue (TargetClass.TestProperty, "somestring");
			Assert.IsTrue(source.SetterCalled, "#9"); /* sl2 */
			Assert.IsTrue(source.GetterCalled, "#10");
			Assert.IsFalse(target.SetterCalled, "#11");
			Assert.IsFalse(target.GetterCalled, "#12");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void DPSetValuePreferred_TwoWay_sl3()
		{
			var str = "test_string";
			var source = new TargetClass ();
			source.SetValue(TargetClass.TestProperty, str);
			var target = new TargetClass();
			target.SetBinding(TargetClass.TestProperty, new Binding("Test") { Source = source, Mode = BindingMode.TwoWay });

			Assert.IsFalse(source.SetterCalled, "#1");
			Assert.IsFalse(source.GetterCalled, "#2");
			Assert.IsFalse(target.SetterCalled, "#3");
			Assert.IsFalse(target.GetterCalled, "#4");

			source.SetValue(TargetClass.TestProperty, "otherstring");
			Assert.IsFalse(source.SetterCalled, "#5");
			Assert.IsFalse(source.GetterCalled, "#6");
			Assert.IsFalse(target.SetterCalled, "#7");
			Assert.IsFalse(target.GetterCalled, "#8");

			target.SetValue (TargetClass.TestProperty, "somestring");
			Assert.IsFalse(source.SetterCalled, "#9");
			Assert.IsFalse(source.GetterCalled, "#10");
			Assert.IsFalse(target.SetterCalled, "#11");
			Assert.IsFalse(target.GetterCalled, "#12");
		}

		[TestMethod]
		public void IncompletePath ()
		{
			Data data = new Data { Brush = null };
			Rectangle r = new Rectangle {
				DataContext = data
			};
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Brush.Color.A"));
			Assert.IsTrue (double.IsNaN (r.Width), "#1");

			data.Brush = new SolidColorBrush (Colors.Black);
			Assert.IsTrue (double.IsNaN (r.Width), "#2");

			r.SetBinding (Rectangle.WidthProperty, new Binding ("Brush.Color.A"));
			Assert.AreEqual (255, r.Width, "#2");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_OneWay ()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double [] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("DoubleList[2]"));

			Assert.AreEqual (2, rect.Width, "#1");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_IndexNegative ()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double [] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("DoubleList[-20]"));

			Assert.IsTrue(double.IsNaN(rect.Width), "#1");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_IndexNegative_ExactException ()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double [] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("DoubleList[-20]"));

			Assert.IsTrue(double.IsNaN(rect.Width), "#1");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_InvalidIndex()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double[] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("DoubleList[HAHAHAHH]"));

			Assert.IsTrue (double.IsNaN (rect.Width), "#1");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_IndexTooLarge ()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double [] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("DoubleList[20]"));

			Assert.IsTrue(double.IsNaN (rect.Width), "#1");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_IndexTooLarge_INCC ()
		{
			// Attach the binding, then add values to the collection to
			// force an update through the INotifyCollectionChanged interface
			var data = new Data { };
			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding ("ObservableDoubles[1]"));

			Assert.IsTrue (double.IsNaN (rect.Width), "#1");
			data.ObservableDoubles.Add (1.0);
			data.ObservableDoubles.Add (2.0);
			Assert.AreEqual (2.0, rect.Width, "#2");
		}

		[TestMethod]
		public void IndexerOnIndexableProperty_TwoWay ()
		{
			var data = new Data { };
			data.DoubleList.AddRange (new double [] { 0, 1, 2, 3, 4 });

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("DoubleList[2]"),
			});

			Assert.AreEqual (2, rect.Width, "#1");

			rect.Width = 6;
			Assert.AreEqual (6, data.DoubleList[2], "#2");
		}

		[TestMethod]
		public void IndexerOnObservableIndexableProperty_TwoWay ()
		{
			var data = new Data { };
			for (double i = 0; i < 5; i++)
				data.ObservableDoubles.Add (i);

			var rect = new Rectangle { DataContext = data };
			rect.SetBinding (Rectangle.WidthProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("ObservableDoubles[2]"),
			});

			Assert.AreEqual (2, rect.Width, "#1");

			data.ObservableDoubles [2] = 30.0;
			Assert.AreEqual (30, rect.Width, "#2");
		}

		[TestMethod]
		public void MentorTest()
		{
			var rect = new Rectangle { DataContext = Colors.Red };
			var brush = new SolidColorBrush();

			rect.Fill = brush;
			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			Assert.AreEqual(Colors.Red.ToString(), brush.Color.ToString (), "#1");
		}

		[TestMethod]
		public void MentorTest_SetMentorThenBinding()
		{
			var rect = new Rectangle { DataContext = Colors.Red };
			var brush = new SolidColorBrush();

			rect.Fill = brush;
			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			Assert.AreEqual(Colors.Red.ToString(), brush.Color.ToString (), "#1");
		}

		[TestMethod]
		[MoonlightBug ("We should not listen for mentor changes here")]
		public void MentorTest_SetBindingThenMentor ()
		{
			var rect = new Rectangle { DataContext = Colors.Red };
			var brush = new SolidColorBrush();

			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			rect.Fill = brush;
			Assert.AreEqual("#00000000", brush.Color.ToString(), "#1");
		}

		[TestMethod]
		public void MentorTest_SetBindingThenMentor_Collection()
		{
			var rect = new CustomCanvas { DataContext = Colors.Red, Collection = new DependencyObjectCollection<DependencyObject> () };
			var brush = new SolidColorBrush();

			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			rect.Collection.Add(brush);
			Assert.AreEqual("#FFFF0000", brush.Color.ToString(), "#1");
		}

		[TestMethod]
		public void MentorTest_DataTemplate()
		{
			var xaml = @"<Button
xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'
xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'>
	<Button.ContentTemplate>
		<DataTemplate>
			<TextBlock x:Name='textBlockFromTemplate' Text='{Binding}' DataContext='{Binding}'/>
		</DataTemplate>
	</Button.ContentTemplate>
</Button>";
			var button = (Button)XamlReader.Load(xaml);
			TestPanel.Children.Add(button);
			button.Content = "In the darkness bind them";
			TestPanel.UpdateLayout();
			var textBlock = button.FindFirstChild <TextBlock>();
			Assert.AreEqual("In the darkness bind them", textBlock.Text, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void MentorTest_TwoMentors_SetMentorsThenBinding ()
		{
			var mentor1 = new Rectangle { DataContext = Colors.Red };
			var mentor2 = new Rectangle { DataContext = Colors.Green };
			var brush = new SolidColorBrush();

			mentor1.Fill = brush;
			mentor2.Fill = brush;
			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			Assert.AreEqual("#00000000", brush.Color.ToString (), "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void MentorTest_TwoMentors_SetBindingThenMentors ()
		{
			var mentor1 = new Rectangle { DataContext = Colors.Red };
			var mentor2 = new Rectangle { DataContext = Colors.Green };
			var brush = new SolidColorBrush();

			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			mentor1.Fill = brush;
			mentor2.Fill = brush;
			Assert.AreEqual("#00000000", brush.Color.ToString(), "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void MentorTest_TwoMentors_SetBindingThenMentors_Remove()
		{
			var mentor1 = new Rectangle { DataContext = Colors.Red };
			var mentor2 = new Rectangle { DataContext = Colors.Green };
			var brush = new SolidColorBrush();

			BindingOperations.SetBinding(brush, SolidColorBrush.ColorProperty, new Binding());
			mentor1.Fill = brush;
			mentor2.Fill = brush;
			mentor1.Fill = null;
			Assert.AreEqual("#00000000", brush.Color.ToString(), "#1");
		}

		[TestMethod()]
		public void MentorTest_CustomProperty()
		{
			var child = new Button();
			var canvas = new CustomCanvas { DataContext = 15.0, Child = child };

			BindingOperations.SetBinding(child, Button.WidthProperty, new Binding());
			Assert.IsTrue (double.IsNaN (child.Width), "#1");

			GC.KeepAlive (canvas);
		}

		[TestMethod]
		public void MentorTest_FrameworkElement_CollectionInCustomProperty()
		{
			var mentor = new MentorElement();
			var collection = new DependencyObjectCollection<object>();
			var child = new SolidColorBrush ();
			BindingOperations.SetBinding(child, SolidColorBrush.ColorProperty, new Binding ());

			// Attach the collection then child
			mentor.MentoredCollection = collection;
			collection.Add(child);

			mentor.DataContext = Colors.Red;
			Assert.AreEqual(Colors.Red, child.Color, "#1");
		}

		[TestMethod]
		public void MentorTest_FrameworkElement_CollectionInCustomProperty_AttachOppositeOrder()
		{
			var mentor = new MentorElement();
			var collection = new DependencyObjectCollection<object>();
			var child = new SolidColorBrush();
			BindingOperations.SetBinding(child, SolidColorBrush.ColorProperty, new Binding());

			// Attach the child then the collection
			collection.Add(child);
			mentor.MentoredCollection = collection;

			mentor.DataContext = Colors.Red;
			Assert.AreEqual(Colors.Red, child.Color, "#1");
		}

		[TestMethod()]
		public void MentorTest_CustomCollectionProperty()
		{
			var child = new Button();
			var canvas = new CustomCanvas { DataContext = 15.0, Collection = new DependencyObjectCollection<DependencyObject>() };
			canvas.Collection.Add(child);

			BindingOperations.SetBinding(child, Button.WidthProperty, new Binding());
			Assert.IsTrue (double.IsNaN (child.Width), "#1");
		}

		[TestMethod]
		public void MultiPathINPC_Standard ()
		{
			var a = new INPC { Value = 0 };
			var b = new INPC { Value = 1 };
			var c = new INPC { Value = 2 };
			var d = new INPC { Value = 3 };

			a.Next = b;
			b.Next = c;
			c.Next = d;

			Rectangle r = new Rectangle ();
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Next.Next.Next.Value"));
			
			r.DataContext = a;
			Assert.AreEqual (3, r.Width, "#1");
			
			d.Value = 10;
			Assert.AreEqual (10, r.Width, "#2");
		}

		[TestMethod]
		public void MultiPathINPC_Standard_ChangeEntireTree ()
		{
			var a = new INPC { Value = 0 };
			var b = new INPC { Value = 1 };
			var c = new INPC { Value = 2 };
			var d = new INPC { Value = 3 };

			a.Next = b;
			b.Next = c;
			c.Next = d;

			Rectangle r = new Rectangle { DataContext = a };
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Next.Next.Next.Value"));

			r.DataContext = null;
			a = new INPC { Value = 10 };
			r.DataContext = a;
			a.Next = new INPC { Value = 20 };
			a.Next.Next = new INPC { Value = 30 };
			a.Next.Next.Next = new INPC { Value = 40 };

			Assert.AreEqual (40, r.Width, "#1");
		}

		[TestMethod]
		public void MultiPathINPC_Standard_ChangeLastLeaf ()
		{
			var a = new INPC { Value = 0 };
			var b = new INPC { Value = 1 };
			var c = new INPC { Value = 2 };
			var d = new INPC { Value = 3 };
			var newD = new INPC { Value = 4 };

			a.Next = b;
			b.Next = c;
			c.Next = d;

			Rectangle r = new Rectangle { DataContext = a };
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Next.Next.Next.Value"));
			Assert.AreEqual (3, r.Width, "#1");
			
			c.Next = newD;
			Assert.AreEqual (4, r.Width, "#2");

			d.Value = 10;
			Assert.AreEqual (4, r.Width, "#3");
		}

		[TestMethod]
		public void MultiPathINPC_Standard_ChangeMiddleLeaf ()
		{
			var a = new INPC { Value = 0 };
			var b = new INPC { Value = 1 };
			var c = new INPC { Value = 2 };
			var d = new INPC { Value = 3 };
			var newC = new INPC { Value = 4 };

			a.Next = b;
			b.Next = c;
			c.Next = d;

			Rectangle r = new Rectangle { DataContext = a };
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Next.Next.Next.Value"));

			// Replace the middle subtree. We should disconnect our property path
			// listeners and property changed listeners from 'c' and 'd'

			b.Next = newC;
			c.Next = new INPC { Value = 100 };
			d.Value = 19;
			Assert.IsTrue (double.IsNaN (r.Width), "#1");

			// We should reattach to  'd' now
			newC.Next = d;
			Assert.AreEqual (19, r.Width, "#3");
		}

		[TestMethod]
		public void MultiPathINPC_PathContainsNonINPC ()
		{
			// If 'b' is not INPC in the link a.b.c.d and we change b.Next, does Silverlight
			// realised that 'c' and 'd' are no longer in the property path? Does it ever unhook
			// from their PropertyChanged event?
			var a = new INPC { Value = 0 };
			var b = new NonINPC { Value = 1 };
			var c = new INPC { Value = 2 };
			var d = new INPC { Value = 3 };

			a.Next = b;
			b.Next = c;
			c.Next = d;

			Rectangle r = new Rectangle { DataContext = a };
			r.SetBinding (Rectangle.WidthProperty, new Binding ("Next.Next.Next.Value"));

			// This issues no propertychanged events. SL should not notice that 'b' has changed
			b.Next = new NonINPC { Value = 10 };
			Assert.AreEqual (3, r.Width, "#1");

			// This will emit a PropertyChanged event which doesn't change the property path
			d.Value = 4;
			Assert.AreEqual (4, r.Width, "#2");

			// This will emit a PC event which does change the property path. SL only verifies
			// from 'c' onwards.
			c.Next = new INPC { Value = 20 };
			Assert.AreEqual (20, r.Width, "#3");
		}

		[TestMethod]
		public void SetBinding ()
		{
			Binding binding = new Binding ("");
			binding.Source = "This is a string";

			TextBlock text = new TextBlock ();
			Assert.Throws<ArgumentException> (delegate {
				text.SetValue (TextBlock.TextProperty, binding);
			});
		}
		
		[TestMethod]
		public void SetBindingExpression()
		{
			// Fails in Silverlight 3
			Binding b = new Binding("");
			b.Source = "This is a string";

			TextBlock b1 = new TextBlock();
			TextBlock b2 = new TextBlock();

			BindingExpressionBase expression = b1.SetBinding(TextBlock.TextProperty, b);
			// Can't use the same expression twice in SL3
			Assert.Throws<ArgumentException>(() => b2.SetValue(TextBlock.TextProperty, expression), "#1");

			Assert.AreEqual (b.Source, b1.Text, "#2");
			Assert.AreEqual("", b2.Text, "#3");
		}

		[TestMethod]
		public void TestTwoWayBinding()
		{
			Data data = new Data { Opacity = 0.5 };
			Rectangle r = new Rectangle();
			r.SetBinding(Rectangle.OpacityProperty, new Binding { Path = new PropertyPath("Opacity"),
						Source = data,
						Mode = BindingMode.TwoWay });
			Assert.AreEqual(0.5, r.Opacity, "#1");
			Assert.AreEqual(0.5, data.Opacity, "#2");
			data.Opacity = 0;
			Assert.AreEqual(0.5, r.Opacity, "#3");
			r.Opacity = 1;
			Assert.IsTrue (r.ReadLocalValue (Rectangle.OpacityProperty) is BindingExpressionBase, "#4");
			Assert.AreEqual(1, r.Opacity, "#5");
			Assert.AreEqual(1, data.Opacity, "#6");

			r.ClearValue(Rectangle.OpacityProperty);
			r.Opacity = 0.5;
			Assert.AreEqual(1, data.Opacity, "#7");
		}

		[TestMethod]
		public void TestTwoWayBinding2()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			Rectangle r = new Rectangle();
			r.SetBinding(Rectangle.OpacityProperty, new Binding
			{
				Path = new PropertyPath("Opacity"),
				Source = data,
				Mode = BindingMode.TwoWay
			});
			Assert.AreEqual(0.5, r.Opacity, "#1");
			Assert.AreEqual(0.5, data.Opacity, "#2");
			data.Opacity = 0;
			Assert.AreEqual(0.0, r.Opacity, "#3");
			r.Opacity = 1;
			Assert.IsTrue(r.ReadLocalValue(Rectangle.OpacityProperty) is BindingExpressionBase, "#4");
			Assert.AreEqual(1, r.Opacity, "#5");
			Assert.AreEqual(1, data.Opacity, "#6");

			r.ClearValue(Rectangle.OpacityProperty);
			r.Opacity = 0.5;
			Assert.AreEqual(1, data.Opacity, "#7");
		}
		
		[TestMethod]
		public void TestTwoWayBinding3 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			Rectangle r = new Rectangle { Opacity = 0 };
			r.SetBinding (Rectangle.OpacityProperty, new Binding {
				Path = new PropertyPath ("OpacityASDF"),
				Source = data,
				Mode = BindingMode.TwoWay
			});
			Assert.AreEqual (1, r.Opacity, "#1");
			Assert.AreEqual (0.5, data.Opacity, "#2");
			data.Opacity = 0;
			Assert.AreEqual (1, r.Opacity, "#3");
			r.Opacity = 0.5f;
			Assert.AreEqual (0, data.Opacity);
		}
		
				
		[TestMethod]
		public void TestTwoWayBinding4 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			TextBlock block = new TextBlock { Text = "Ted" };
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Path = new PropertyPath ("Opacity"),
				Source = data,
				Mode = BindingMode.TwoWay
			});
			Assert.AreEqual ("0.5", block.Text, "#1");
			block.Text = "1";
			Assert.AreEqual (1, data.Opacity, "#2");
			block.Text = "100";
			Assert.AreEqual (100, data.Opacity, "#3");
			block.Text = "";
			Assert.AreEqual (100, data.Opacity, "#4");
		}
					
		[TestMethod]
		[MoonlightBug ("We fail this because we try to update the source even though TextBox.Text hasn't actually changed")]
		public void TestTwoWayBinding5 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			data.Reset ();
			TextBlock block = new TextBlock { Text = "Ted" };
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Path = new PropertyPath ("Opacity"),
				Source = data,
				Mode = BindingMode.TwoWay
			});
			Assert.AreEqual ("0.5", block.Text, "#1");
			Assert.IsTrue (data.Get, "#a");
			Assert.IsFalse (data.Set, "#b");
			data.Reset ();

			block.Text = "1";
			Assert.AreEqual (1, data.Opacity, "#2");
			Assert.IsTrue (data.Get, "#c");
			Assert.IsTrue (data.Set, "#d");
			data.Reset ();

			block.Text = "1";
			Assert.IsFalse (data.Get, "#e");
			Assert.IsFalse (data.Set, "#f");
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("The DP listening code in the StandardPropertyPathNode broke this test. Maybe storyboards don't raise DP changed events?")]
		public void TestTwoWayBinding6 ()
		{
			TextBlock block = new TextBlock { Text = "Ted" };
			Rectangle data = new Rectangle { Opacity = 0.5f };
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Path = new PropertyPath ("Opacity"),
				Source = data,
				Mode = BindingMode.TwoWay
			});

			Storyboard sb = new Storyboard ();
			DoubleAnimation anim = new DoubleAnimation { From = 1, To = 0, Duration = TimeSpan.FromMilliseconds (1) };
			Storyboard.SetTarget (anim, data);
			Storyboard.SetTargetProperty (anim, new PropertyPath ("Opacity"));
			sb.Children.Add (anim);

			bool complete = false;
			sb.Completed += delegate { complete = true; };
			sb.Begin ();
			EnqueueConditional (() => complete, "#1");
			Enqueue (() => {
				Assert.AreEqual (0, data.Opacity, "#2");
				Assert.AreEqual ("0.5", block.Text, "#3");
			});
			EnqueueTestComplete ();
		}
							
		[TestMethod]
        public void TestTwoWayBinding7 ()
        {
            ValidationErrorEventArgs bindingEx = null;
            Data data = new Data();

            Rectangle r = new Rectangle();
            r.BindingValidationError += delegate(object o, ValidationErrorEventArgs e) {
                bindingEx = e;
            };
            r.SetBinding(Rectangle.IsHitTestVisibleProperty, new Binding {
                Path = new PropertyPath("ThrowExceptionsOnUpdate"),
                Source = data,
                Mode = BindingMode.TwoWay
            });
            Assert.IsFalse(r.IsHitTestVisible, "#1");
            r.IsHitTestVisible = true;
            Assert.IsTrue(r.IsHitTestVisible, "#2");
            Assert.IsNull(bindingEx, "#3");
        }

        [TestMethod]
        public void TestTwoWayBinding8 ()
        {
            ValidationErrorEventArgs bindingEx = null;
            Data data = new Data();

            Rectangle r = new Rectangle();
            r.BindingValidationError += delegate(object o, ValidationErrorEventArgs e) {
                bindingEx = e;
            };
            r.SetBinding(Rectangle.IsHitTestVisibleProperty, new Binding {
                Path = new PropertyPath("ThrowExceptionsOnUpdate"),
                Source = data,
                Mode = BindingMode.TwoWay,
                ValidatesOnExceptions = true
            });
            Assert.IsFalse(r.IsHitTestVisible, "#1");
            r.IsHitTestVisible = true;
            Assert.IsTrue(r.IsHitTestVisible, "#2");
            Assert.IsNull(bindingEx, "#3");
        }

        [TestMethod]
        public void TestTwoWayBinding9 ()
        {
            ValidationErrorEventArgs bindingEx = null;
            Data data = new Data();

            Rectangle r = new Rectangle();
            r.BindingValidationError += delegate(object o, ValidationErrorEventArgs e)
            {
                bindingEx = e;
            };
            r.SetBinding(Rectangle.IsHitTestVisibleProperty, new Binding
            {
                Path = new PropertyPath("ThrowExceptionsOnUpdate"),
                Source = data,
                Mode = BindingMode.TwoWay,
                NotifyOnValidationError = true
            });
            Assert.IsFalse(r.IsHitTestVisible, "#1");
            r.IsHitTestVisible = true;
            Assert.IsTrue(r.IsHitTestVisible, "#2");
            Assert.IsNull(bindingEx, "#3");
        }

        [TestMethod]
        public void TestTwoWayBinding10 ()
        {
            ValidationErrorEventArgs bindingEx = null;
            Data data = new Data();

            Rectangle r = new Rectangle();
            r.BindingValidationError += delegate(object o, ValidationErrorEventArgs e)
            {
                bindingEx = e;
            };
            r.SetBinding(Rectangle.IsHitTestVisibleProperty, new Binding
            {
                Path = new PropertyPath("ThrowExceptionsOnUpdate"),
                Source = data,
                Mode = BindingMode.TwoWay,
                NotifyOnValidationError = true,
                ValidatesOnExceptions = true
            });
            Assert.IsFalse(r.IsHitTestVisible, "#1");
            r.IsHitTestVisible = true;
            Assert.IsTrue(r.IsHitTestVisible, "#2");
            Assert.IsNotNull(bindingEx, "#3");
        }

		[TestMethod]
		public void TestTwoWayBinding11 ()
		{
			Rectangle r = new Rectangle();
			Assert.Throws<ArgumentException> (() => r.SetBinding (Canvas.WidthProperty, new Binding { Mode = BindingMode.TwoWay }));
		}

		[TestMethod]
		public void TestOnceOffBinding ()
		{
			Data data = new Data ();
			Rectangle rectangle = new Rectangle { Opacity = 0f };
			Binding binding = new Binding {
					Path = new PropertyPath("Opacity"), 
					Mode = BindingMode.OneTime, 
					Source = data
			};

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, rectangle.Opacity, "#1");
			Assert.IsTrue(rectangle.ReadLocalValue(Rectangle.OpacityProperty) is BindingExpressionBase, "#2");
			data.Opacity = 0;
			Assert.AreNotEqual (data.Opacity, rectangle.Opacity, "#3");
		}

		[TestMethod]
		[Asynchronous]
		public void TestOnceOffBinding2 ()
		{
			Canvas c = new Canvas { DataContext = 5.0 };
			Rectangle r = new Rectangle ();
			r.SetBinding (Rectangle.HeightProperty, new Binding { Mode = BindingMode.OneTime });
			Assert.IsInstanceOfType<BindingExpressionBase> (r.ReadLocalValue (Rectangle.HeightProperty), "#1");
			c.Children.Add (r);
			Assert.IsInstanceOfType<BindingExpressionBase> (r.ReadLocalValue (Rectangle.HeightProperty), "#2");
			CreateAsyncTest (c,
				() => c.DataContext = 6.0,
				() => {
					Assert.IsInstanceOfType<BindingExpressionBase> (r.ReadLocalValue (Rectangle.HeightProperty), "#3");
					Assert.AreEqual (6.0, r.Height, "#2");
				}
			);
		}

		[TestMethod]
		public void TestOnceOffBinding3()
		{
			var source = new Rectangle { Width = 100 };
			var dest = new Rectangle();

			dest.SetBinding(Rectangle.WidthProperty, new Binding("Width") {
				Mode = BindingMode.OneTime,
				Source = source,
			});

			Assert.AreEqual(100, dest.Width, "#1");
			Assert.IsInstanceOfType<BindingExpressionBase>(dest.ReadLocalValue(Rectangle.WidthProperty), "#a");
			source.Width = 200;
			Assert.AreEqual(100, dest.Width, "#2");
			Assert.IsInstanceOfType<BindingExpressionBase>(dest.ReadLocalValue(Rectangle.WidthProperty), "#b");
		}

		[TestMethod]
		[Asynchronous]
		public void TestOnceOffBinding_SourceName ()
		{
			var source = new Rectangle { Name = "Element", Width = 100 };
			var dest = new Rectangle();

			dest.SetBinding(Rectangle.WidthProperty, new Binding("Width") {
				Mode = BindingMode.OneTime,
				ElementName = "Element"
			});
			TestPanel.Children.Add(source);
			TestPanel.Children.Add(dest);
			Assert.IsTrue (double.IsNaN (dest.Width), "#1");
			Enqueue(() => {
				Assert.AreEqual (100, dest.Width, "#2");
			});
			EnqueueTestComplete();
		}

		[TestMethod]
		[Asynchronous]
		public void TestOnceOffBinding_SourceName_2 ()
		{
			var source = new Rectangle { Name = "Element", Width = 100 };
			var dest = new Rectangle();

			TestPanel.Children.Add(source);
			TestPanel.Children.Add(dest);
			dest.SetBinding(Rectangle.WidthProperty, new Binding("Width")  {
				Mode = BindingMode.OneTime,
				ElementName = "Element"
			});

			Assert.AreEqual(100, dest.Width, "#1");
			source.Width = 200;
			Assert.AreEqual(100, dest.Width, "#2");
			Enqueue(() => {
				Assert.AreEqual(100, dest.Width, "#3");
			});
			EnqueueTestComplete();
		}

		[TestMethod]
		public void TestOneWayBinding ()
		{
			Data data = new Data ();
			Rectangle rectangle = new Rectangle { Opacity = 0f };
			Binding binding = new Binding {
				Path = new PropertyPath("Opacity"),
				Mode = BindingMode.OneWay,
				Source = data
			};

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, rectangle.Opacity);
			data.Opacity = 0.0f;
			Assert.AreNotEqual (data.Opacity, rectangle.Opacity, string.Format ("{0}-{1}", data.Opacity, rectangle.Opacity));
		}

		[TestMethod]
		public void TestOneWayBinding2 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			Rectangle rectangle = new Rectangle { Opacity = 0f };
			Binding binding = new Binding {
				Path = new PropertyPath ("Opacity"),
				Mode = BindingMode.OneWay,
				Source = data
			};

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, rectangle.Opacity);
			data.Opacity = 0.0f;
			Assert.AreEqual (data.Opacity, rectangle.Opacity);
		}

		[TestMethod]
		public void TestOneWayBinding3 ()
		{
			PropertyUpdater data = new PropertyUpdater { Opacity = 0.5f };
			Rectangle rectangle = new Rectangle { Opacity = 1f , DataContext = data };
			Binding binding = new Binding
			{
				Path = new PropertyPath ("Opacity"),
				Mode = BindingMode.OneWay,
			};

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			rectangle.DataContext = null;
			data.Opacity = 0.5f;
			Assert.AreEqual (1.0f, rectangle.Opacity, "#1");
			rectangle.DataContext = data;
			Assert.AreEqual (0.5f, rectangle.Opacity, "#2");
		}

		[TestMethod]
		public void TestOneWayBinding_GetterThrows ()
		{
			var source = new INPC ();
			var target = new Rectangle ();
			target.SetBinding (Rectangle.WidthProperty, new Binding ("GetterException") { ValidatesOnDataErrors = true, NotifyOnValidationError = true, ValidatesOnExceptions = true });
			Assert.DoesNotThrow (() => target.DataContext = source, "#1");
			Assert.AreEqual (0, Validation.GetErrors (target).Count, "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void TwoWayWithStoryboard_CustomProperty ()
		{
			bool complete = false;
			var data = new OpacityObject { OpacityDP = 0 };
			var rect = new OpacityObject { OpacityDP = 0 };
			data.SetBinding (OpacityObject.OpacityDPProperty, new Binding {
				Source = rect,
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("OpacityDP"),
			});
		
			Storyboard sb = new Storyboard ();
			sb.Children.Add (new DoubleAnimation {
				From = 0,
				To = 1,
				Duration = TimeSpan.FromMilliseconds (10)
			});
			Storyboard.SetTarget (sb.Children [0], data);
			Storyboard.SetTargetProperty (sb.Children [0], new PropertyPath ("(OpacityDP)"));
		
			sb.Completed += delegate { complete = true; };
			sb.Begin ();
			EnqueueConditional (() => complete, "#1");
			Enqueue (() => Assert.AreEqual (1, data.OpacityDP, "#2"));
			Enqueue (() => Assert.AreEqual (1, rect.OpacityDP, "#3"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void TwoWayWithStoryboard_CoreProperty ()
		{
			bool complete = false;
			var data = new OpacityObject { Opacity = 0 };
			var rect = new OpacityObject { Opacity = 0 };
			rect.SetBinding (OpacityObject.OpacityProperty, new Binding {
				Source = data,
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("Opacity"),
			});

			Storyboard sb = new Storyboard ();
			sb.Children.Add (new DoubleAnimation {
				From = 0,
				To = 1,
				Duration = TimeSpan.FromMilliseconds (10)
			});
			Storyboard.SetTarget (sb.Children [0], rect);
			Storyboard.SetTargetProperty (sb.Children [0], new PropertyPath ("(Opacity)"));

			sb.Completed += delegate { complete = true; };
			sb.Begin ();
			EnqueueConditional (() => complete, "#1");
			Enqueue (() => Assert.AreEqual (0, data.Opacity, "#2"));
			Enqueue (() => Assert.AreEqual (1, rect.Opacity, "#3"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void ModifyAfterRegisterOneWay ()
		{
			Binding binding = new Binding {
				Path = new PropertyPath ("Opacity"),
				Mode = BindingMode.OneWay
			};
			Rectangle rectangle = new Rectangle { Opacity = 0f, DataContext = new Data() };
			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.Throws<InvalidOperationException> (delegate {
				binding.Converter = null;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.ConverterCulture = CultureInfo.CurrentCulture;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.ConverterParameter = 5;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.Mode = BindingMode.TwoWay;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.NotifyOnValidationError = true;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.Path = new PropertyPath ("asd");
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.Source = null;
			});
			Assert.Throws<InvalidOperationException> (delegate {
				binding.ValidatesOnExceptions = true;

			});
		}

		[TestMethod]
		public void ModifyAfterRegisterOneTime ()
		{
			Binding binding = new Binding {
				Path = new PropertyPath ("Opacity"),
				Mode = BindingMode.OneTime,
				Source = new Data ()
			};

			Rectangle rectangle = new Rectangle { Opacity = 0f, DataContext = new Data() };
			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.AreEqual(new Data ().Opacity, rectangle.Opacity);

			Assert.Throws<InvalidOperationException> (delegate {
				binding.Converter = null;
			});
		}

		[TestMethod]
		public void NullPath ()
		{
			Assert.Throws<NullReferenceException> (() => new Binding ("Path").Path = null);
		}

		[TestMethod]
		public void ReadLocalProperty ()
		{
			PropertyUpdater data = new PropertyUpdater ();
			Rectangle rectangle = new Rectangle { Opacity = 0f };
			Binding binding = new Binding {
				Path = new PropertyPath ("Opacity"),
				Mode = BindingMode.OneWay,
				Source = data
			};
			Assert.AreEqual (0.0, (double) rectangle.ReadLocalValue (Rectangle.OpacityProperty), "#1");
			
			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			
			Assert.IsTrue(rectangle.ReadLocalValue (Rectangle.OpacityProperty) is BindingExpressionBase, "#2");
		}

		[TestMethod]
		public void PathNotValid()
		{
			Binding binding = new Binding {
				Path = new PropertyPath ("PrivOpacity"),
				Source = new Data (),
			};

			Rectangle r = new Rectangle { Opacity = 0.0 };
			r.SetBinding (Shape.OpacityProperty, binding);
			Assert.AreEqual (1.0, r.Opacity);
		}

		[TestMethod]
		public void ChangeAfterBinding ()
		{
			Data data = new Data ();
			Binding binding = new Binding {
				Path = new PropertyPath ("Opacity"),
				Source = data,
				Mode = BindingMode.OneWay
			};

			Rectangle r = new Rectangle { Opacity = 0.0 };
			r.SetBinding (Shape.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, r.Opacity);
			r.Opacity = 1.0;
			Assert.AreEqual (1.0, r.Opacity);
			data.Opacity = 0.0f;
			Assert.AreEqual (1.0, r.Opacity);
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("This causes silverlight to completely die when it expands the template")]
		public void BindingOnDO ()
		{
			// Putting a {Binding} on a non-framework element in a ControlTemplate
			// causes SL to die.
			var control = (ContentControl) XamlReader.Load (
@"	
<ContentControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ContentControl.Template>
		<ControlTemplate>
			<Grid Height=""{Binding Height}"">
				<Grid.RowDefinitions>
					<RowDefinition MinHeight=""{Binding Height}"" />
				</Grid.RowDefinitions>
				<ContentPresenter />
			</Grid>
		</ControlTemplate>
	</ContentControl.Template>
</ContentControl>
");
			CreateAsyncTest (control, () => {
				Grid grid = (Grid) VisualTreeHelper.GetChild (control, 0);
				Assert.IsInstanceOfType<TemplateBindingExpression> (grid.ReadLocalValue (Grid.HeightProperty), "#1");
				Assert.AreSame (DependencyProperty.UnsetValue, grid.RowDefinitions [0].ReadLocalValue (RowDefinition.MinHeightProperty), "#2");

				control.Height = 5;
				Assert.AreEqual (5, grid.Height, "#3");
				Assert.AreEqual (0, grid.RowDefinitions [0].MinHeight, "#4");
			});
		}

		[TestMethod]
		public void TemplateBinding_NoDP_WithClrWrapper ()
		{
			var c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
				xmlns:clr=""clr-namespace:System;assembly=mscorlib""
				xmlns:c=""clr-namespace:MoonTest.System.Windows.Data;assembly=moon-unit"">
	<ContentControl.Template>
		<ControlTemplate>
			<c:ClrPropertyNoDP MyProperty=""{TemplateBinding Width}"" />
		</ControlTemplate>
	</ContentControl.Template>
</ContentControl>");
			c.ApplyTemplate ();
			c.UpdateLayout ();
		}

		[TestMethod]
		[Asynchronous]
		public void TemplateBindingOnDO ()
		{
			// Putting a {TemplateBinding} on a non-FrameworkElement in a ControlTemplate
			// results in the binding being silently discarded.
			var control = (ContentControl)XamlReader.Load (
@"	
<ContentControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ContentControl.Template>
		<ControlTemplate>
			<Grid Height=""{TemplateBinding Height}"">
				<Grid.RowDefinitions>
					<RowDefinition MinHeight=""{TemplateBinding Height}"" />
				</Grid.RowDefinitions>
				<ContentPresenter />
			</Grid>
		</ControlTemplate>
	</ContentControl.Template>
</ContentControl>
");
			CreateAsyncTest (control, () => {
				Grid grid = (Grid) VisualTreeHelper.GetChild (control, 0);
				Assert.IsInstanceOfType<TemplateBindingExpression> (grid.ReadLocalValue (Grid.HeightProperty), "#1");
				Assert.AreSame (DependencyProperty.UnsetValue, grid.RowDefinitions [0].ReadLocalValue (RowDefinition.MinHeightProperty), "#2");
				
				control.Height = 5;
				Assert.AreEqual (5, grid.Height, "#3");
				Assert.AreEqual (0, grid.RowDefinitions [0].MinHeight, "#4");
			});
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("This blows up Silverlight")]
		public void TemplateBindingOnTooltip ()
		{
			var control = (ContentControl) XamlReader.Load (
@"	
<ContentControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ContentControl.Template>
		<ControlTemplate>
			<Border ToolTipService.ToolTip=""{TemplateBinding Content}"">
				
			</Border>
		</ControlTemplate>
	</ContentControl.Template>
</ContentControl>
");
			control.Content = "Hello!";
			CreateAsyncTest (control, () => {
				Border b = (Border) VisualTreeHelper.GetChild (control, 0);
				Assert.AreEqual ("Hello!", ToolTipService.GetToolTip (b));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void UpdateDataContext ()
		{
			string s = "Hello";
			string s2 = "Goodbye";
			TextBlock block = new TextBlock ();
			

			TestPanel.Children.Add (block);

			foreach (BindingMode mode in new BindingMode [] { BindingMode.OneTime, BindingMode.OneWay }) {
				Enqueue (() => block.SetBinding (TextBlock.TextProperty, new Binding { Mode = mode }));
				Enqueue (() => TestPanel.DataContext = s);
				Enqueue (() => Assert.AreEqual (s, block.Text, "#1"));
				Enqueue (() => TestPanel.DataContext = s2);
				Enqueue (() => Assert.AreEqual (s2, block.Text, "#2"));
				Enqueue (() => TestPanel.DataContext = s);
				Enqueue (() => Assert.AreEqual (s, block.Text, "#3"));
			}

			EnqueueTestComplete ();
		}


		[TestMethod]
		public void UpdateSourceTrigger_Default_Xaml ()
		{
			Rectangle r = (Rectangle) XamlReader.Load (@"
<Rectangle	xmlns=""http://schemas.microsoft.com/client/2007""
			Width=""{Binding Height, Mode=TwoWay, UpdateSourceTrigger=Default}"" />");
			var binding = r.GetBindingExpression (Rectangle.WidthProperty).ParentBinding;
			Assert.AreEqual (UpdateSourceTrigger.Default, binding.UpdateSourceTrigger, "#1");
		}

		[TestMethod]
		public void UpdateSourceTrigger_Explicit_Xaml ()
		{
			Rectangle r = (Rectangle) XamlReader.Load (@"
<Rectangle	xmlns=""http://schemas.microsoft.com/client/2007""
			Width=""{Binding Height, Mode=TwoWay, UpdateSourceTrigger=Explicit}"" />");
			var binding = r.GetBindingExpression (Rectangle.WidthProperty).ParentBinding;
			Assert.AreEqual (UpdateSourceTrigger.Explicit, binding.UpdateSourceTrigger, "#1");
		}

		[TestMethod]
		public void UpdateSourceTrigger_Explicit_WrongCase_Xaml ()
		{
			Rectangle r = (Rectangle) XamlReader.Load (@"
<Rectangle	xmlns=""http://schemas.microsoft.com/client/2007""
			Width=""{Binding Height, Mode=TwoWay, UpdateSourceTrigger=explicit}"" />");
			var binding = r.GetBindingExpression (Rectangle.WidthProperty).ParentBinding;
			Assert.AreEqual (UpdateSourceTrigger.Explicit, binding.UpdateSourceTrigger, "#1");
		}

		[TestMethod]
		public void UpdateSourceTrigger_Invalid_Xaml ()
		{

			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Rectangle	xmlns=""http://schemas.microsoft.com/client/2007""
			Width=""{Binding Height, Mode=TwoWay, UpdateSourceTrigger=whatever}"" />");
			});
		}

		[TestMethod]
		public void XamlCreateBinding()
		{
			object o = XamlReader.Load(
@"	
<Canvas
Width=""100""
Height=""100""
xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
>
	<TextBlock Text=""{Binding MyProperty}"" Foreground=""Green""/>
</Canvas>

");
			TextBlock block = (TextBlock) ((Canvas)o).Children[0];
			Assert.IsTrue(block.ReadLocalValue(TextBlock.TextProperty) is BindingExpressionBase);
		}

		[TestMethod]
		[Ignore ("another bogus parser test.")]
		public void XamlCreateBinding2()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load(@"	
<Canvas
Width=""100""
Height=""100""
xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
>
	<TextBlock Foreground=""Green"">
        <Foreground></Foreground>
   </TextBlock>
</Canvas>
");
            });
		}

		[TestMethod]
		public void XamlActualBinding ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.DataContext>
		<Binding Source=""string"" Mode=""OneTime"" />
	</Canvas.DataContext>
</Canvas>");

			Assert.IsInstanceOfType (c.DataContext, typeof (string), "#1");
			Assert.AreEqual ("string", c.DataContext, "#2");
		}

		[TestMethod]
		public void XamlDataContextWithBindingElement()
		{
			Canvas c = (Canvas)XamlReader.Load(@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.Resources>
		<SolidColorBrush x:Key=""Brush"" Color=""Blue"" />
	</Canvas.Resources>

	<Canvas.DataContext>
		<Binding Source=""{StaticResource Brush}"" Mode=""OneTime"" />
	</Canvas.DataContext>
</Canvas>");
			Assert.IsNotNull(c.DataContext, "#1");
			Assert.IsInstanceOfType(c.DataContext, typeof(SolidColorBrush), "#2");
		}
			
		[TestMethod]
		public void XamlDataContextWithBindingElement2 ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.Resources>
		<Rectangle x:Name=""rect"" Fill=""Red"">
		</Rectangle>
	</Canvas.Resources>

	<Canvas.DataContext>
		<Binding Source=""{StaticResource rect}"" Path=""Fill"" Mode=""OneTime"" />
	</Canvas.DataContext>
</Canvas>");
			Assert.IsNotNull (c.DataContext, "#1");
			Assert.IsInstanceOfType (c.DataContext, typeof (SolidColorBrush), "#2");
		}

		[TestMethod]
		public void XamlDataContext()
		{
			Canvas c = (Canvas)XamlReader.Load(@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.DataContext>
		<Rectangle Fill=""Blue"" />
	</Canvas.DataContext>
	<TextBlock Foreground=""{Binding Fill, Mode=OneWay}"" />
</Canvas>");
			Assert.IsInstanceOfType(c.Children[0], typeof(TextBlock), "#1");
			TextBlock block = (TextBlock)c.Children[0];
			Assert.IsInstanceOfType(block.Foreground, typeof(SolidColorBrush), "#2");

			SolidColorBrush brush = (SolidColorBrush)block.Foreground;
			Assert.AreNotEqual(brush.Color, Colors.Blue, "#3");

			TextBlock normal = new TextBlock();
			Assert.AreEqual(((SolidColorBrush)normal.Foreground).Color, brush.Color, "#4");

			Assert.IsNotNull(c.DataContext, "#5");
		}

		[TestMethod]
		[Asynchronous]
		public void XamlDataContext2 ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<TextBlock Foreground=""{Binding}"">
		<TextBlock.DataContext>
			<SolidColorBrush Color=""Blue"" />
		</TextBlock.DataContext>
	</TextBlock>
</Canvas>");

			CreateAsyncTest (c, 
				() => {
					Assert.IsInstanceOfType (c.Children [0], typeof (TextBlock), "#1");
					TextBlock block = (TextBlock) c.Children [0];
					Assert.IsInstanceOfType (block.Foreground, typeof (SolidColorBrush), "#2");
	
					SolidColorBrush brush = (SolidColorBrush) block.Foreground;
					Assert.AreEqual (brush.Color, Colors.Blue, "#3");
				}
			);
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void XamlDataContext3()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.Resources>
		<SolidColorBrush x:Name=""Brush"" Color=""Blue"" />
	</Canvas.Resources>
	<TextBlock Foreground=""{Binding Source={StaticResource Brush}}"" />
</Canvas>");
			Assert.IsInstanceOfType (c.Children [0], typeof (TextBlock), "#1");
			TextBlock block = (TextBlock) c.Children[0];
			Assert.IsInstanceOfType (block.Foreground, typeof (SolidColorBrush), "#2");

			SolidColorBrush brush = (SolidColorBrush) block.Foreground;
			Assert.AreEqual (brush.Color, Colors.Blue, "#3");

			Assert.IsNull (block.DataContext, "#4");
			block.DataContext = new SolidColorBrush (Colors.Red);

			brush = (SolidColorBrush) block.Foreground;
			Assert.AreEqual (brush.Color, Colors.Blue, "#5");
		}

		[TestMethod]
		public void XamlBindPath ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		x:Name=""LayoutRoot"">
	<Canvas.Resources>
		<SolidColorBrush x:Name=""Brush"" Color=""Blue"" />
	</Canvas.Resources>
	<TextBlock Foreground=""{Binding Source={StaticResource Brush} Path={StaticResource Brush}}"" />
</Canvas>");
			});
		}

		[TestMethod]
		public void XamlBindAfterResources ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource FAKEBRUSH}"" />
        </TextBlock.Foreground>
    </TextBlock>
</Canvas>
");
			});
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void XamlBindAfterResourcesb_sl3 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource FAKEBRUSH}}"" />
</Canvas>
");
			string color = ((TextBlock) canvas.FindName ("i")).Foreground.GetValue (SolidColorBrush.ColorProperty).ToString ();
			Assert.AreEqual (Colors.Black.ToString (), color, "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		[MoonlightBug]
		public void XamlBindAfterResourcesb_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource FAKEBRUSH}}"" />
</Canvas>
"));
		}

		[TestMethod]
		public void XamlBindAfterResources2 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource brush}"" Path=""FAKEPATH"" />
        </TextBlock.Foreground>
    </TextBlock>
</Canvas>
");
			string color = ((TextBlock) canvas.FindName ("i")).Foreground.GetValue (SolidColorBrush.ColorProperty).ToString ();
			Assert.AreEqual (Colors.Black.ToString (), color, "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void XamlBindAfterResources2b_sl3 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush} Path=FAKEPATH}"" />
</Canvas>
");
			string color = ((TextBlock) canvas.FindName ("i")).Foreground.GetValue (SolidColorBrush.ColorProperty).ToString ();
			Assert.AreEqual (Colors.Black.ToString (), color, "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		[MoonlightBug]
		public void XamlBindAfterResources2b_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush} Path=FAKEPATH}"" />
</Canvas>
"));
		}

		[TestMethod]
		public void XamlBindAfterResources3 ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource brush}"" Path="""" Converter=""{StaticResource DONTEXIST}"" />
        </TextBlock.Foreground>
    </TextBlock>
</Canvas>
");
			});
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void XamlBindAfterResources3b_sl3 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush} Converter={StaticResource DONTEXIST}}"" />
</Canvas>
");
			string color = ((TextBlock) canvas.FindName ("i")).Foreground.GetValue (SolidColorBrush.ColorProperty).ToString ();
			Assert.AreEqual (Colors.Blue.ToString (), color, "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void XamlBindAfterResources3b_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>

    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush} Converter={StaticResource DONTEXIST}}"" />
</Canvas>
"));
		}

		[TestMethod]
		public void XamlBindBeforeResources ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource brush}"" />
        </TextBlock.Foreground>
    </TextBlock>
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
</Canvas>
");
			});
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void XamlBindBeforeResourcesb_sl3 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush}}"" />
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
</Canvas>
");
			string color = ((TextBlock) canvas.FindName ("i")).Foreground.GetValue (SolidColorBrush.ColorProperty).ToString ();
			Assert.AreEqual (Colors.Black.ToString (), color, "#1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		[MoonlightBug]
		public void XamlBindBeforeResourcesb_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <TextBlock x:Name=""i"" Foreground=""{Binding Source={StaticResource brush}}"" />
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
</Canvas>
"));
		}

		[TestMethod]
		public void XamlBindBeforeResources2 ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource brush}"" Path=""FAKEPATH"" />
        </TextBlock.Foreground>
    </TextBlock>
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
</Canvas>
");
			});
		}

		[TestMethod]
		public void XamlBindBeforeResources3 ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource brush}"" Path="""" Converter=""{StaticResource DONTEXIST}"" />
        </TextBlock.Foreground>
    </TextBlock>
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
</Canvas>
");
			});
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		[MoonlightBug]
		public void XamlBindingPropertyPathPriority_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load(@"
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
Width=""100"" Height=""100"">
	<Canvas.Resources>
		<Rectangle x:Name=""rect"" Width=""20"" Height=""30"" RadiusX=""4"" RadiusY=""5""/>
	</Canvas.Resources>
	<TextBlock x:Name=""text"" Text=""{Binding Width, Path=Height, Source={StaticResource rect}, Mode=OneTime, Path=RadiusX}""/>
</Canvas>
"));
		}

		[TestMethod]
		[MoonlightBug]
		public void XamlBindToClr()
		{
 			Assert.Throws<XamlParseException>(delegate {
				XamlReader.Load(
@"	
<Canvas	
Width=""100""
Height=""100""
xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
xmlns:my=""clr-namespace:MoonTest.System.Windows.Data""

>
	<Canvas.Resources>
		<my:Data x:Name=""CLRObject"" />
	</Canvas.Resources>
	<TextBox Text=""{Binding OpacityString, Source={StaticResource CLRObject}, Mode=OneTime}""/>
</Canvas>

");
 			});
		}

		[TestMethod]
		[Ignore ("This test throws an NRE on both moonlight and silverlight.  needs to be fixed in SL-land")]
		public void XamlBoundToClr()
		{
			TestNamespace.BindingXaml a = new TestNamespace.BindingXaml();
			TextBlock block = (TextBlock)a.LayoutRoot.Children[0];
			Assert.IsNotNull(block, "#1");
			//Assert.IsNull(a.CLRObject, "#2");
			
			BindingExpressionBase expression = block.ReadLocalValue(TextBlock.TextProperty) as BindingExpressionBase;
			Assert.IsNotNull(expression, "3");
			Assert.IsNull(block.DataContext);
			Assert.IsNull(a.FindName("CLRObject"));
			Assert.IsNull(a.DataContext);
		}
							
		[TestMethod]
		public void XamlBindWithContent ()
		{
			TextProp c = (TextProp) XamlReader.Load (@"
<c:TextProp	xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
			xmlns:clr=""clr-namespace:System;assembly=mscorlib""
			xmlns:c=""clr-namespace:MoonTest.System.Windows.Data;assembly=moon-unit"">
	<c:TextProp.MyText>
		<clr:String>        This contains {Binding} and {StaticResource} and {TemplateBinding} </clr:String>
	</c:TextProp.MyText>
</c:TextProp>");
			Assert.AreEqual ("This contains {Binding} and {StaticResource} and {TemplateBinding}", c.MyText, "#1");
		}
							
		[TestMethod]
		public void XamlBindWithContent2 ()
		{
			TextProp c = (TextProp) XamlReader.Load (@"
<c:TextProp	xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
			xmlns:clr=""clr-namespace:System;assembly=mscorlib""
			xmlns:c=""clr-namespace:MoonTest.System.Windows.Data;assembly=moon-unit"">
	<c:TextProp.MyText>
		<clr:String>'This contains {Binding} and {StaticResource} and {TemplateBinding}'</clr:String>
	</c:TextProp.MyText>
</c:TextProp>");
			Assert.AreEqual ("'This contains {Binding} and {StaticResource} and {TemplateBinding}'", c.MyText, "#1");
		}
			
		[TestMethod]
		[MoonlightBug ("SL3 has different handling for invalid paths. It ignores some issues and throws exceptions on others")]
		public void XamlPropertyPathTest ()
		{
			// FIXME: When fixing this test, the commented out checks should be replaced by another test which
			// verifies that parsing invalid paths from the .xaml file throws an exception.
			Mono.Moonlight.BindingConverter c = new Mono.Moonlight.BindingConverter ();
			TextBlock a = (TextBlock) c.FindName ("a");
			//Assert.IsInstanceOfType (a.ReadLocalValue (TextBlock.TextProperty), typeof (BindingExpressionBase));
			Assert.AreEqual ("0.5", a.Text, "#1");
			//Assert.AreEqual ("", ((TextBlock) c.FindName ("b")).Text, "#2");
			Assert.AreEqual ("0.5", ((TextBlock) c.FindName ("c")).Text, "#3");
			//Assert.AreEqual ("", ((TextBlock) c.FindName ("d")).Text, "#4");
			//Assert.AreEqual ("", ((TextBlock) c.FindName ("e")).Text, "#5");
			//Assert.AreEqual ("", ((TextBlock) c.FindName ("f")).Text, "#6");
			Assert.AreEqual (typeof (OpacityTest).FullName, ((TextBlock) c.FindName ("g")).Text, "#7");
			Assert.AreEqual (typeof (OpacityTest).FullName, ((TextBlock) c.FindName ("h")).Text, "#8");
			Assert.AreEqual ("1.5", ((TextBlock) c.FindName ("i")).Text, "#9");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void XamlStaticResource_sl4 ()
		{
			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    Width=""400"" Height=""300"">
	<Rectangle x:Name=""Before"" Fill=""{Binding Source={StaticResource brush}}"" />
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
	<Rectangle x:Name=""After"" Fill=""{Binding Source={StaticResource brush}}"" />
	<Rectangle x:Name=""Invalid"" Fill=""{Binding Source={StaticResource NOTHERE}}"" />
</Canvas>
"));
		}

		[TestMethod]
		public void XamlStaticResource2 ()
		{
			Canvas canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
	<TextBlock x:Name=""block"">
		{Binding Source={StaticResource brush}}
	</TextBlock>
</Canvas>
");
			TextBlock block = (TextBlock) canvas.FindName ("block");
			Assert.AreEqual ("{Binding Source={StaticResource brush}}", block.Text, "#1");
		}

		[TestMethod]
		public void XamlStaticResource3 ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
    <TextBlock x:Name=""i"">
        <TextBlock.Foreground>
            <Binding Source=""{StaticResource NOTHERE}"" />
        </TextBlock.Text>
    </TextBlock>
</Canvas>
");
			});
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void XamlStaticResource4_sl4 ()
		{
			var c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
	<TextBlock x:Name=""block"">
		<Binding />
	</TextBlock>
</Canvas>
");
			// Where does the binding go?
			Assert.IsNull(((TextBlock)c.Children[0]).GetBindingExpression(TextBlock.TextProperty), "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		[MoonlightBug ("this test is identical to the _sl4 variant, but we're throwing a XamlParseException incorrectly")]
		public void XamlStaticResource4_sl3 ()
		{
			var c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    Width=""400"" Height=""300"">
    <Canvas.Resources>
        <SolidColorBrush  x:Name=""brush"" Color=""Blue"" />
    </Canvas.Resources>
	<TextBlock x:Name=""block"">
		<Binding />
	</TextBlock>
</Canvas>
");
			// Where does the binding go?
			Assert.IsNull(((TextBlock)c.Children[0]).GetBindingExpression(TextBlock.TextProperty), "#1");
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("This blows up silverlight")]
		public void XamlTemplateBinding ()
		{
			ContentControl c = (ContentControl)XamlReader.Load (@"
<ContentControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    xmlns:clr=""clr-namespace:Mono.Moonlight"">
    <ContentControl.Template>
        <ControlTemplate>
            <Canvas>
                <ContentControl x:Name=""Parent"" Content=""{TemplateBinding Content}"" />
            </Canvas>
        </ControlTemplate>
    </ContentControl.Template>
</ContentControl>");
			c.Content = new Rectangle ();
			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c,
					new VisualNode<Canvas> ("#1",
						new VisualNode<ContentControl>("#2",
							new VisualNode<ContentPresenter> ("#3",
								new VisualNode<Rectangle>("#4")
							)
						)
					)
				);
			});
		}
							
							
		[TestMethod]
		[Asynchronous]
		[Ignore("This blows up silverlight")]
		public void XamlTemplateBinding2 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    xmlns:clr=""clr-namespace:Mono.Moonlight"">
    <ContentControl.Template>
        <ControlTemplate>
            <Canvas>
                <TextBlock x:Name=""Parent"" Text=""{TemplateBinding Content}"" />
            </Canvas>
        </ControlTemplate>
    </ContentControl.Template>
</ContentControl>");
			c.Content = "STRING";
			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c,
					new VisualNode<Canvas> ("#1",
						new VisualNode<TextBlock> ("#2", (b) => {
							Assert.AreEqual (c.Content.ToString (), b.Text, "#a");
						})
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		[Ignore("This blows up silverlight")]
		public void XamlTemplateBinding3 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    xmlns:clr=""clr-namespace:Mono.Moonlight"">
    <ContentControl.Template>
        <ControlTemplate>
            <Canvas>
                <TextBlock x:Name=""Parent"" Text=""{TemplateBinding Content}"" />
            </Canvas>
        </ControlTemplate>
    </ContentControl.Template>
</ContentControl>");
			c.Content = new Rectangle ();
			TextBlock block = null;
			CreateAsyncTest (c,
				() => {
					Assert.VisualChildren (c,
						new VisualNode<Canvas> ("#1",
							new VisualNode<TextBlock> ("#2", (b) => block = b)
						)
					);
				},
				() => {
					Assert.AreEqual ("", block.Text, "#a");
					Assert.IsInstanceOfType<TemplateBindingExpression> (block.ReadLocalValue (TextBlock.TextProperty), "#b");
				},
				() => c.Content = "STRING",
				() => {
					Assert.AreEqual ("STRING", block.Text, "#c");
					Assert.IsInstanceOfType<TemplateBindingExpression> (block.ReadLocalValue (TextBlock.TextProperty), "#d");
				},
				() => c.Content = new Ellipse (),
				() => {
					Assert.AreEqual ("", block.Text, "#e");
					Assert.IsInstanceOfType<TemplateBindingExpression> (block.ReadLocalValue (TextBlock.TextProperty), "#f");
				}
			);
		}

		[TestMethod]
		[MoonlightBug ("This is failing in SL4 now aswell when XamlReader is parsing the string")]
		public void XamlTemplateBinding_Reuse ()
		{
			// If we re-apply a TemplateBinding, a NullReferenceException
			// is thrown in their Attach method. Probably because they cleared
			// everything out in their Detach method (or equivalent).
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    xmlns:clr=""clr-namespace:Mono.Moonlight"">
    <ContentControl.Template>
        <ControlTemplate>
            <Canvas>
                <ContentControl x:Name=""Parent"" Width=""{TemplateBinding Width}"" />
            </Canvas>
        </ControlTemplate>
    </ContentControl.Template>
</ContentControl>");

			c.ApplyTemplate ();
			var cc = c.FindFirstChild<ContentControl> ();
			var binding = (TemplateBindingExpression) cc.ReadLocalValue (ContentControl.WidthProperty);
			cc.ClearValue (ContentControl.ContentProperty);
			cc = new ContentControl ();

			Assert.Throws<NullReferenceException> (() =>
				cc.SetValue (ContentControl.ContentProperty, binding)
			, "#1");
		}

		[TestMethod]
		public void CustomObjectTest1 ()
		{
			// create the hierarchy, set the binding, and then set datacontext
			Canvas c = new Canvas ();

			TargetClass tc = new TargetClass ();

			c.Children.Add (tc);

			tc.SetBinding (TargetClass.TestProperty,
				       new Binding ());

			Assert.IsFalse (tc.GetPropertyChanged (), "#1");

			c.DataContext = "hi";

			Assert.IsTrue (tc.GetPropertyChanged (), "#2");

			Assert.AreEqual ("hi", tc.Test, "#3");
		}

		[TestMethod]
		public void CustomObjectTest2 ()
		{
			// set the binding, set the datacontext, then create the hierarchy
			Canvas c = new Canvas ();

			TargetClass tc = new TargetClass ();

			tc.SetBinding (TargetClass.TestProperty,
				       new Binding ());

			c.DataContext = "hi";

			Assert.IsFalse (tc.GetPropertyChanged (), "#1");

			c.Children.Add (tc);

			Assert.IsFalse (tc.GetPropertyChanged (), "#2");

			Assert.IsNull (tc.Test, "#3");
		}
	}
}
