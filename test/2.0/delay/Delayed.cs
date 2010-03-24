using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace delay
{
	public class DelayedAttached {
		static Button button;

		public static readonly DependencyProperty PropProperty = DependencyProperty.RegisterAttached ("Prop", typeof (string), typeof (DelayedAttached),
														new PropertyMetadata (OnPropChanged));

		static void OnPropChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			if (sender is Button) {
				Button el = (Button)sender;

				if (el.Parent != null) {
					el.Content = "el.parent != null in OnPropChanged";
				}
				else if (VisualTreeHelper.GetParent (el) != null) {
					el.Content = "VisualTreeHelper.GetParet (el) != null in OnPropChanged";
				}
				else {
					el.Content = "parent == null in OnPropChanged";
				}

				button = el;
			}
			else if (sender is SolidColorBrush) {
				SolidColorBrush scb = (SolidColorBrush)sender;

				if (button.Foreground == sender) {
					scb.Color = Colors.Red;
				}
				else {
					scb.Color = Colors.Green;
				}
			}
		}

		public static void SetProp (DependencyObject el, string value)
		{
			el.SetValue (PropProperty, value);
		}

		public static string GetProp (DependencyObject el)
		{
			return (string)el.GetValue (PropProperty);
		}
	}
}
