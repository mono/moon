//
// System.Windows.Media.VisualTreeHelper.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Windows;
using System.Windows.Controls;
using System.Collections.Generic;
using Mono;

namespace System.Windows.Media {
	public static class VisualTreeHelper {
		public static DependencyObject GetChild (DependencyObject reference, int childIndex)
		{
			FrameworkElement ref_fw = reference as FrameworkElement;
			if (ref_fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");

			DependencyObject subtree = ref_fw.SubtreeObject;
			UIElementCollection collection = subtree as UIElementCollection;
			if (collection != null)
				return collection[childIndex];

			UIElement item = subtree as UIElement;
			if (item != null && childIndex == 0)
				return item;

			throw new ArgumentOutOfRangeException ();
		}

		public static int GetChildrenCount (DependencyObject reference)
		{
			FrameworkElement ref_fw = reference as FrameworkElement;
			if (ref_fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");

			DependencyObject subtree = ref_fw.SubtreeObject;

			UIElementCollection collection = subtree as UIElementCollection;
			if (collection != null)
				return collection.Count;
			
			UIElement item = subtree as UIElement;
			if (item != null)
				return 1;

			return 0;
		}

		public static DependencyObject GetParent (DependencyObject reference)
		{
			FrameworkElement fw = reference as FrameworkElement;
			if (fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");
			return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.uielement_get_visual_parent (fw.native)) as DependencyObject;
		}

		public static IEnumerable<UIElement> FindElementsInHostCoordinates (Point intersectingPoint, UIElement subtree)
		{
			HitTestCollection collection = new HitTestCollection ();
			// note: 'subtree' is not validated (null-check) by Silverlight 2 - leading to a NRE
			Mono.NativeMethods.uielement_find_elements_in_host_coordinates_p (subtree.native, intersectingPoint, collection.native);
			return new List<UIElement> (collection);
		}

		public static IEnumerable<UIElement> FindElementsInHostCoordinates (Rect intersectingRect, UIElement subtree)
		{
			HitTestCollection collection = new HitTestCollection ();
			// note: 'subtree' is not validated (null-check) by Silverlight 2 - leading to a NRE
			Mono.NativeMethods.uielement_find_elements_in_host_coordinates_r (subtree.native, intersectingRect, collection.native);
			return new List<UIElement> (collection);
		}
	}
}
