
using System;
using System.Windows;
using System.Windows.Media;

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
	}
}

