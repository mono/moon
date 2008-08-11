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
using Mono;

namespace System.Windows.Media {

	public static class VisualTreeHelper {
		public static DependencyObject GetChild (DependencyObject reference, int childIndex)
		{
			FrameworkElement ref_fw = reference as FrameworkElement;
			if (ref_fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");

			Panel p = reference as Panel;
			if (p == null) {
				// we have no children, so everything is out of range.
				throw new ArgumentOutOfRangeException ();
			}

			return p.Children[childIndex];
		}

		public static int GetChildrenCount (DependencyObject reference)
		{
			FrameworkElement ref_fw = reference as FrameworkElement;
			if (ref_fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");

			Panel p = reference as Panel;
			if (p == null) {
				// we have no children
				return 0;
			}

			return p.Children.Count;
		}

		public static DependencyObject GetParent (DependencyObject reference)
		{
			FrameworkElement fw = reference as FrameworkElement;
			if (fw == null)
				throw new InvalidOperationException ("Reference is not a valid visual DependencyObject");
			return fw.Parent;
		}
	}
}