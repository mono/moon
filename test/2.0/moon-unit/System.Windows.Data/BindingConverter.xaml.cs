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

namespace Mono.Moonlight
{
	public class DateConverter : IValueConverter
	{
		public DateTime Now
		{
			get { return new DateTime (2009, 2, 26); }
		}

		public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if (parameter is DateConverter)
				return "converter-object";

			if (((string) parameter) == "dateconverter")
				return "converter-string";

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

            internal System.Windows.Controls.Grid LayoutRoot;
        
            internal Mono.Moonlight.DateConverter dateconverter;
		public BindingConverter ()
		{
			System.Windows.Application.LoadComponent (this, new System.Uri ("/moon-unit;component/BindingConverter.xaml", System.UriKind.Relative));
			this.LayoutRoot = ((System.Windows.Controls.Grid) (this.FindName ("LayoutRoot")));
			this.dateconverter = ((Mono.Moonlight.DateConverter) (this.FindName ("dateconverter")));
		}
	}
}

