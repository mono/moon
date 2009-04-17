using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Data;
using System.ComponentModel;

namespace Mono.Moonlight
{
	public class FloatConverter : IValueConverter
	{
		public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			Console.WriteLine ("Converting: '{0}'");
			Console.WriteLine ("Type is: {0}", value.GetType ().Name);
			return (float) value + 1;
		}

		public object ConvertBack (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			Console.WriteLine ("Converting back!?");
			return (float) (global::System.Convert.ToDouble (value) - 1);
		}
	}
	
	public class DateClass
	{
		public static readonly DependencyProperty DateProperty = DependencyProperty.Register ("Date", typeof (DateTime?), typeof (DateClass), null);
		
		public DateClass ()
		{
		}
		
		DateTime? date = DateTime.Now;
		[TypeConverter (typeof (DateTypeConverter))]
		public DateTime? Date
		{
			get { return date; }
			set { date = value; }
		}
	}

	public class DateTypeConverter : TypeConverter
	{
		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			return true;
		}
		public override bool CanConvertTo (ITypeDescriptorContext context, Type destinationType)
		{
			return true;
		}
		public override object ConvertFrom (ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
		{
			if (culture.Name != "en-US")
				throw new Exception ("Culture must be en-US");
			return DateTime.Now;
		}
		public override object ConvertTo (ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
		{
			if (culture.Name != "en-US")
				throw new Exception ("Culture must be en-US");
			return null;
		}
	}

	public class DateConverter : IValueConverter
	{
		public DateTime Now
		{
			get { return new DateTime (2009, 2, 26); }
		}
		
		public DateTime? NullNow
		{
			get { return null; }
		}

		public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if (value == null)
				return "null-value";
			
			if (parameter is DateConverter)
				return "converter-object";

			if (((string) parameter) == "dateconverter")
				return "converter-string";
			
			if (culture.Name != "en-US")
				throw new Exception ("Default culture is en-US");

			return  ((DateTime) value).ToString ((string) parameter);
		}

		public object ConvertBack (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			return DateTime.Parse ((string) value);
		}
		
		public override string ToString ()
		{
			return string.Format("[DateConverter: Now={0}]", Now);
		}
	}

	public partial class BindingConverter : UserControl
	{
        internal System.Windows.Controls.Grid Root;
		public BindingConverter ()
		{
			System.Windows.Application.LoadComponent (this, new System.Uri ("/moon-unit;component/BindingConverter.xaml", System.UriKind.Relative));
			System.Windows.Application.LoadComponent (this, new System.Uri ("/moon-unit;component/System.Windows.Data/BindingConverter.xaml", System.UriKind.Relative));
			Root = ((System.Windows.Controls.Grid) (this.FindName ("LayoutRoot")));
		}
	}
}

