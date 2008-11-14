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

namespace MoonTest.System.Windows.Data
{
	public class BoundData : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;
		private Color color;
		public Color Color
		{
			get { return color; }
			set { color = value; if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("Color")); }
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
			Assert.AreEqual(CultureInfo.CurrentCulture, culture, "#a");
			Assert.AreEqual(typeof(Brush), targetType, "#b");
			if (Colors.Brown.ToString() == value.ToString())
				return new SolidColorBrush(Colors.Brown);

			return new SolidColorBrush(Colors.Blue);
		}

		public object ConvertBack(object value, Type targetType, object parameter, global::System.Globalization.CultureInfo culture)
		{
			Assert.AreEqual(CultureInfo.CurrentCulture, culture, "#a");
			Assert.AreEqual(typeof(Color), targetType, "#b");
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
		[Ignore ("Not fully implemented yet")]
		public void Test()
		{
			Setup();
			Assert.AreEqual(Colors.Brown, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#1");
			data.Color = Colors.Green;
			Assert.AreEqual(Colors.Blue.ToString (), rect.Fill.GetValue(SolidColorBrush.ColorProperty).ToString (), "#2");
			rect.SetValue(Rectangle.FillProperty, new SolidColorBrush(Colors.Brown));
			Assert.AreEqual(Colors.Brown, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#3");
			data.Color = Colors.Purple;
			Assert.AreEqual(Colors.Blue, rect.Fill.GetValue(SolidColorBrush.ColorProperty), "#4");
		}
	}
}