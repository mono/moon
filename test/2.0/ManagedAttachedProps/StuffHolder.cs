

using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace ManagedAttachedProps {

	public class Stretcher : Panel {

		public static readonly DependencyProperty StretchAmountProperty = DependencyProperty.RegisterAttached ("StretchAmount", typeof (double), typeof (Stretcher), null);

		public Stretcher ()
		{
		}

		public static void SetStretchAmount (DependencyObject obj, double amount)
		{
			FrameworkElement elem = obj as FrameworkElement;
			if (elem != null) {
				elem.Width *= amount;
			}
			obj.SetValue (StretchAmountProperty, amount);
		}

		public static double GetStretchAmount (DependencyObject obj)
		{
			return (double) obj.GetValue (StretchAmountProperty);
		}
		
	}
}


