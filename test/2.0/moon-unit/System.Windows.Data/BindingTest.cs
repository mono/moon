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

namespace MoonTest.System.Windows.Data
{

	public class OpacityTest
	{
		public float Opacity {
			get; set;
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

		public Data()
		{
			Brush = new SolidColorBrush(Colors.Brown);
			Opacity = 0.5f;
		}
	}

	[TestClass]
	public class BindingTest
	{
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
			
			public float Opacity
			{
				get { return opacity; }
				set {
					opacity = value;
					if (PropertyChanged != null)
						PropertyChanged(this, new PropertyChangedEventArgs("Opacity"));
				}
			}

			public PropertyUpdater()
			{
				opacity = 0.5f;
			}
		}

		public class TargetClass : Control {
			bool propertyChanged;

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
				get { return (string)GetValue (TestProperty); }
				set { SetValue (TestProperty, value); }
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
			Assert.IsFalse (binding.ValidatesOnExceptions);

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
		[MoonlightBug]
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
			double d = r.Opacity;
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

			Assert.Throws<MethodAccessException> (delegate {
				rectangle.SetBinding (Shape.OpacityProperty, binding);
			});
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
			Assert.Throws<MethodAccessException> (delegate {
				rectangle.SetBinding(Shape.OpacityProperty, binding);
			});
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
		public void SetBindingExpression()
		{
			Binding b = new Binding("");
			b.Source = "This is a string";

			TextBlock b1 = new TextBlock();
			TextBlock b2 = new TextBlock();

			BindingExpressionBase expression = b1.SetBinding(TextBlock.TextProperty, b);
			b2.SetValue(TextBlock.TextProperty, expression);

			Assert.AreEqual(b1.Text, b.Source, "#1");
			Assert.AreEqual(b2.Text, b.Source, "#1");
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
		[MoonlightBug]
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
		[MoonlightBug]
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
			Assert.IsInstanceOfType (c.Children [0], typeof (TextBlock), "#1");
			TextBlock block = (TextBlock) c.Children[0];
			Assert.IsInstanceOfType (block.Foreground, typeof (SolidColorBrush), "#2");

			SolidColorBrush brush = (SolidColorBrush) block.Foreground;
			Assert.AreNotEqual (brush.Color, Colors.Blue, "#3");

			TextBlock normal = new TextBlock ();
			Assert.AreEqual (((SolidColorBrush) normal.Foreground).Color, brush.Color, "#4");

			Assert.IsNotNull (block.DataContext, "#5");
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

			TextBlock normal = new TextBlock ();
			Assert.AreNotEqual (((SolidColorBrush) normal.Foreground).Color, brush.Color, "#4");

			Assert.IsNull (block.DataContext, "#5");
			block.DataContext = new SolidColorBrush (Colors.Red);

			brush = (SolidColorBrush) block.Foreground;
			Assert.AreEqual (brush.Color, Colors.Blue, "#6");
		}

		[TestMethod]
		public void XamlBindingPropertyPathPriority()
		{
			Canvas canvas = (Canvas) XamlReader.Load(@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
Width=""100"" Height=""100"">
	<Canvas.Resources>
		<Rectangle x:Name=""rect"" Width=""20"" Height=""30"" RadiusX=""4"" RadiusY=""5""/>
	</Canvas.Resources>
	<TextBlock x:Name=""text"" Text=""{Binding Width, Path=Height, Source={StaticResource rect}, Mode=OneTime, Path=RadiusX}""/>
</Canvas>
");
			
			TextBlock block = (TextBlock) canvas.Children[0];
			object text = block.ReadLocalValue (TextBlock.TextProperty);
			Assert.IsTrue (text is BindingExpressionBase);
			Assert.AreEqual ("4", block.Text);
		}

		[TestMethod]
		[Ignore ("this is supposed to be a binding test - testing for a parser error is hardly appropriate")]
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
