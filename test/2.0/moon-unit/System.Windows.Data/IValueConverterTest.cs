using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using System.Windows.Data;
using System.ComponentModel;
using System.Globalization;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Data
{
	public class InvalidConverter : IValueConverter
	{
		public object Value { get; set; }
		public object Convert (object value, Type targetType, object parameter, CultureInfo culture)
		{
			return Value;
		}

		public object ConvertBack (object value, Type targetType, object parameter, CultureInfo culture)
		{
			return Value;
		}
	}
	
	public class BoundData : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;
		float converting;
		Color color;
		GridLength gridLength;
		Visibility visibility;
		public Color Color
		{
			get { return color; }
			set { color = value; if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("Color")); }
		}
		
		[TypeConverter (typeof (InvalidConverter))]
		public float Converting
		{
			get { return converting; }
			set {
				converting = value;
				if (PropertyChanged != null)
					PropertyChanged (this, new PropertyChangedEventArgs ("Converting"));
			}
		}

		public GridLength GridLength
		{
			get { return gridLength; }
			set
			{
				gridLength = value;
				if (PropertyChanged != null)
					PropertyChanged (this, new PropertyChangedEventArgs ("GridLength"));
			}
		}

		[TypeConverter (typeof (InvalidConverter))]
		public Visibility Visibility
		{
			get { return visibility; }
			set
			{
				visibility = value;
				if (PropertyChanged != null)
					PropertyChanged (this, new PropertyChangedEventArgs ("Visibility"));
			}
		}

		public BoundData()
		{
			Color = Colors.Brown;
		}
	}

	public class SolidBrushConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter,CultureInfo culture)
		{
			Assert.AreEqual(new CultureInfo("en-US"), culture, "#a1");
			Assert.AreEqual(typeof(Brush), targetType, "#b1");
			if (Colors.Brown.ToString() == value.ToString())
				return new SolidColorBrush(Colors.Brown);

			return new SolidColorBrush(Colors.Blue);
		}

		public object ConvertBack(object value, Type targetType, object parameter, global::System.Globalization.CultureInfo culture)
		{
			Assert.AreEqual(new CultureInfo("en-US"), culture, "#a2");
			Assert.AreEqual(typeof(Color), targetType, "#b2");
			return ((SolidColorBrush)value).Color;
		}
	}
	
	[TestClass]
	public class IValueConverterTest
	{
		BoundData data;
		Rectangle rect;

		public void Setup()
		{
			//FIXME: Use a test where you bind a Brush property to a Rectangle - it fails!
			data = new BoundData();
			rect = new Rectangle { Opacity = 0.4 };
			Binding binding = new Binding { Path = new PropertyPath("Color"), Source = data, Mode = BindingMode.TwoWay};
			binding.Converter = new SolidBrushConverter();
			rect.SetBinding(Rectangle.FillProperty, binding);
		}

		[TestMethod]
//		[MoonlightBug ("Not fully implemented yet")]
		public void Test()
		{
			global::System.Threading.Thread.CurrentThread.CurrentCulture = new CultureInfo ("en-IE");
			Setup();
			Assert.AreEqual(Colors.Brown, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#1");
			data.Color = Colors.Green;
			Assert.AreEqual(Colors.Blue.ToString (), rect.Fill.GetValue(SolidColorBrush.ColorProperty).ToString (), "#2");
			rect.SetValue(Rectangle.FillProperty, new SolidColorBrush(Colors.Brown));
			Assert.AreEqual(Colors.Brown, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#3");
			data.Color = Colors.Purple;
			Assert.AreEqual(Colors.Blue, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#4");
		}
		
		[TestMethod]
		public void InvalidValueConverter ()
		{
			BoundData data = new BoundData { };
			InvalidConverter converter = new InvalidConverter { Value = 123 };
			TextBlock block = new TextBlock ();
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("Converting"),
				Source = data,
				Converter = converter
			});
			
			// Make TextBlock.Text update
			data.Converting = 5;
			Assert.AreEqual ("123", block.Text, "#1");

			// Make data.Converting update
			block.Text = "Update";
			Assert.AreEqual (123, data.Converting, "#2");

			// What happens if we return something crazy
			converter.Value = "I'm a string";

			// Make TextBlock.Text update
			data.Converting = 5;
			Assert.AreEqual ("I'm a string", block.Text, "#1");

			// Make data.Converting update
			block.Text = "Hello";
			Assert.AreEqual (5, data.Converting, "#2");
		}
			
		[TestMethod]
		public void InvalidValueConverter2 ()
		{
			BoundData data = new BoundData { };
			InvalidConverter converter = new InvalidConverter { Value = 50 };
			TextBlock block = new TextBlock { Visibility = Visibility.Collapsed };

			block.SetBinding (TextBlock.VisibilityProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("Converting"),
				Source = data,
				Converter = converter
			});

			Assert.AreEqual (Visibility.Visible, block.Visibility, "#0");

			// Make TextBlock.Visibility update
			data.Converting = 5;
			Assert.AreEqual (Visibility.Visible, block.Visibility, "#1");

			converter.Value = 1;
			data.Converting = 6;
			Assert.AreEqual (Visibility.Visible, block.Visibility, "#2");

			converter.Value = Visibility.Collapsed;
			data.Converting = 7;
			Assert.AreEqual (Visibility.Collapsed, block.Visibility, "#3");
		}
				
		[TestMethod]
		[MoonlightBug ("Fails because the Visibility enum basetype is not byte")]
		public void InvalidValueConverter3 ()
		{
			BoundData data = new BoundData { Visibility = Visibility.Collapsed };
			InvalidConverter converter = new InvalidConverter { Value = 0 };
			TextBlock block = new TextBlock { };
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("Visibility"),
				Source = data,
				Converter = converter
			});

						
			Assert.AreEqual (Visibility.Collapsed, data.Visibility, "#1");

			block.Text = "a";
			Assert.AreEqual (Visibility.Collapsed, data.Visibility, "#2");

			converter.Value = Visibility.Visible;
			block.Text = "b";
			Assert.AreEqual (Visibility.Visible, data.Visibility, "#3");

			converter.Value = "Collapsed";
			block.Text = "c";

			Assert.AreEqual (Visibility.Collapsed, data.Visibility, "#4");
		}
					
		[TestMethod]
		public void InvalidValueConverter4 ()
		{
			BoundData data = new BoundData { GridLength = new GridLength (0, GridUnitType.Auto) };
			InvalidConverter converter = new InvalidConverter { Value = "*" };
			TextBlock block = new TextBlock { };
			block.SetBinding (TextBlock.TextProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("GridLength"),
				Source = data,
				Converter = converter
			});

			// Check if we use the same type conversions as for Style and Storyboards
			Assert.IsTrue(data.GridLength.IsAuto, "#1");

			block.Text = "a";
			Assert.IsTrue (data.GridLength.IsStar, "#2");
		}
						
		[TestMethod]
		public void InvalidValueConverter5 ()
		{
			BoundData data = new BoundData { GridLength = new GridLength (0, GridUnitType.Auto) };
			TextBlock block = new TextBlock { };

			block.SetBinding (TextBlock.TextProperty, new Binding {
				Mode = BindingMode.TwoWay,
				Path = new PropertyPath ("GridLength"),
				Source = data
			});

			// Check if we use the same type conversions as for Style and Storyboards
			Assert.IsTrue (data.GridLength.IsAuto, "#1");

			block.Text = "a";
			Assert.IsTrue (data.GridLength.IsAuto, "#2");
								
		}
	}
}