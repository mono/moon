
using System;
using System.Windows;
using System.Windows.Media;
using System.Collections.Generic;

namespace MoonTest
{
	public static class UIElementExtensions
	{
		public static T GetChild<T> (this UIElement self, int index) where T : UIElement
		{
			return (T) VisualTreeHelper.GetChild (self, index);
		}

		public static T FindFirstChild <T> (this UIElement self) where T : UIElement
		{
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount (self); i++) {
				var child = (UIElement) VisualTreeHelper.GetChild (self, i);
				if (child is T) {
					return (T) child;
				} else {
					var t = FindFirstChild <T> (child);
					if (t != null)
						return t;
				}
			}

			return null;
		}

		public static List<FrameworkElement> GetVisualChildren (this UIElement self, bool recurse)
		{
			List<FrameworkElement> children = new List<FrameworkElement> ();
			GetVisualChildren (self, children, recurse);
			return children;
		}

		static void GetVisualChildren (this UIElement self, List<FrameworkElement> children, bool recurse)
		{
			for (int i=0; i < VisualTreeHelper.GetChildrenCount (self); i++) {
				var child = (FrameworkElement) VisualTreeHelper.GetChild (self, i);
				children.Add (child);
				if (recurse)
					GetVisualChildren (child, children, recurse);
			}
		}
	}
}

