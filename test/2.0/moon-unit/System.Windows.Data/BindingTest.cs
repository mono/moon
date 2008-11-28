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

		public class Data
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

			public Data()
			{
				Brush = new SolidColorBrush(Colors.Brown);
				Opacity = 0.5f;
			}
		}

		public class PropertyUpdater : INotifyPropertyChanged
		{
			public event PropertyChangedEventHandler PropertyChanged;

			// Under MS.NET this works when it's classified as a float
			private double opacity;
			
			public double Opacity
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
		public void TestOnceOffBinding ()
		{
			Data data = new Data ();
			Rectangle rectangle = new Rectangle { Opacity = 0f };
			Binding binding = new Binding { Path = new PropertyPath("Opacity"), 
											Mode = BindingMode.OneTime, 
											Source = data
			};

			rectangle.SetBinding (Rectangle.OpacityProperty, binding);
			Assert.AreEqual (data.Opacity, rectangle.Opacity);
			data.Opacity = 0;
			Assert.AreNotEqual (data.Opacity, rectangle.Opacity);
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
			PropertyUpdater data = new PropertyUpdater ();
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
		[Ignore ("I don't know how to find the default value that a DP has")]
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
		[Ignore ("The parser should throw an exception for this")]
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
		[Ignore ("Test fails on mono")]
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
	}
}
