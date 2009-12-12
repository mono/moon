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

namespace MoonTest.System.Windows.Data
{

	public class TextProp : FrameworkElement
	{
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

		public Data()
		{
			Brush = new SolidColorBrush(Colors.Brown);
			Opacity = 0.5f;
		}
	}

	[TestClass]
	public class BindingTest : SilverlightTest
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
		[Asynchronous]
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
			Binding binding = new Binding (" ");
			binding.Source = "string";
			TextProp prop = new TextProp ();
			prop.SetBinding (TextProp.MyTextProperty, binding);

			Assert.AreEqual (null, prop.MyText, "#1");
		}

		[TestMethod]
		public void BindToText5b ()
		{
			// Fails in Silverlight 3
			Binding binding = new Binding (" ");
			binding.Source = "string";
			TextProp prop = new TextProp { MyText = "test" };
			prop.SetBinding (TextProp.MyTextProperty, binding);

			Assert.AreEqual (null, prop.MyText, "#1");
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
			bool loaded = false;
			Canvas c = (Canvas) XamlReader.Load (@"	
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
	 DataContext=""100"" >
	<Ellipse Width=""{Binding Source=200}"" Height=""{Binding Mode=OneTime}"" />
</Canvas>
");
			Ellipse e = (Ellipse) c.Children [0];
			e.Loaded += delegate { loaded = true; Assert.AreEqual (100, e.Height, "#3"); };
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
			}); // Fails in Silverlight 3 (no exception thrown)
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
			}); // Fails in Silverlight 3 (no exception thrown)
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
		public void CreateWithDP ()
		{
			Binding b = new Binding ();
			Assert.Throws<ArgumentException> (() => {
				b.Path = new PropertyPath (Rectangle.OpacityProperty);
			}); // Fails in Silverlight 3 (got System.Exception)
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
			Assert.AreEqual ("string", c.DataContext, "#1");
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
				Canvas canvas = (Canvas) XamlReader.Load (@"
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
		public void XamlBindAfterResourcesb ()
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
		public void XamlBindAfterResources2b ()
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
		public void XamlBindAfterResources3 ()
		{
			Assert.Throws<XamlParseException> (() => {
				Canvas canvas = (Canvas) XamlReader.Load (@"
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
		public void XamlBindAfterResources3b ()
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
		public void XamlBindBeforeResources ()
		{
			Assert.Throws<XamlParseException> (() => {
				Canvas canvas = (Canvas) XamlReader.Load (@"
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
		public void XamlBindBeforeResourcesb ()
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
		public void XamlBindBeforeResources2 ()
		{
			Assert.Throws<XamlParseException> (() => {
				Canvas canvas = (Canvas) XamlReader.Load (@"
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
				Canvas canvas = (Canvas) XamlReader.Load (@"
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
		public void XamlBindingPropertyPathPriority ()
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
		<clr:String>'        This contains {Binding} and {StaticResource} and {TemplateBinding} '</clr:String>
	</c:TextProp.MyText>
</c:TextProp>");
			Assert.AreEqual ("'        This contains {Binding} and {StaticResource} and {TemplateBinding} '", c.MyText, "#1");
		}
			
		[TestMethod]
		public void XamlPropertyPathTest ()
		{
			// Fails in Silverlight 3
			Mono.Moonlight.BindingConverter c = new Mono.Moonlight.BindingConverter ();
			TextBlock a = (TextBlock) c.FindName ("a");
			//Assert.IsInstanceOfType (a.ReadLocalValue (TextBlock.TextProperty), typeof (BindingExpressionBase));
			Assert.AreEqual ("0.5", a.Text, "#1");
			Assert.AreEqual ("", ((TextBlock) c.FindName ("b")).Text, "#2");
			Assert.AreEqual ("", ((TextBlock) c.FindName ("c")).Text, "#3");
			Assert.AreEqual ("", ((TextBlock) c.FindName ("d")).Text, "#4");
			Assert.AreEqual ("", ((TextBlock) c.FindName ("e")).Text, "#5");
			Assert.AreEqual ("", ((TextBlock) c.FindName ("f")).Text, "#6");
			Assert.AreEqual (typeof (OpacityTest).FullName, ((TextBlock) c.FindName ("g")).Text, "#7");
			Assert.AreEqual (typeof (OpacityTest).FullName, ((TextBlock) c.FindName ("h")).Text, "#8");
			Assert.AreEqual ("1.5", ((TextBlock) c.FindName ("i")).Text, "#9");
		}

		[TestMethod]
		public void XamlStaticResource ()
		{
			Canvas canvas = (Canvas)XamlReader.Load (@"
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
");
			Rectangle before = (Rectangle) canvas.FindName ("Before");
			Rectangle after = (Rectangle) canvas.FindName ("After");
			Rectangle invalid = (Rectangle) canvas.FindName ("Invalid");
			Assert.IsNull (before.Fill, "#1");
			Assert.IsNotNull (after.Fill, "#2");
			Assert.AreEqual (after.Fill.GetValue (SolidColorBrush.ColorProperty).ToString (), Colors.Blue.ToString (), "#3");
			Assert.IsNull (invalid.Fill, "#4");
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
		public void XamlStaticResource4 ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
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
			});
		}
							
		[TestMethod]
		[Asynchronous]
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
