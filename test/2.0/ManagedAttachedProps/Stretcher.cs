using System;
using System.Net;
using System.Windows;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace ManagedAttachedProps
{

	public class StretchAmountConverter : TypeConverter {

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type source_type)
		{
			if (source_type == typeof (string))
				return true;
			return base.CanConvertFrom (context, source_type);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			string sv = value as string;
			if (sv != null) {
				Stretch s = new Stretch ();
				int amount;
				if (Int32.TryParse (sv, out amount)) {
					s.Amount = amount;
					return s;
				}
			}
			return base.ConvertFrom (context, culture, value);
		}


	}

	[TypeConverter (typeof (StretchAmountConverter))]
	public class Stretch
	{
		private int amount;

		public int Amount {
			get { return amount; }
			set { amount = value; }
		}
	}

        public partial class Stretcher : Panel
        {
		public static readonly DependencyProperty StretchAmountProperty = DependencyProperty.RegisterAttached("StretchAmount", typeof(Stretch), typeof(Stretcher), null);

		public Stretcher()
		{
		}

		public static void SetStretchAmount(DependencyObject obj, Stretch amount)
		{
			FrameworkElement elem = obj as FrameworkElement;
			if (elem != null)
			{
				elem.Width *= amount.Amount;
			}

			obj.SetValue(StretchAmountProperty, amount);

			Console.WriteLine("SET STRETCH AMOUNT");
		}

		public static string GetStretchAmount(DependencyObject obj)
		{
			return (string) obj.GetValue(StretchAmountProperty);
		}
        }
}
