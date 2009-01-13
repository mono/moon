//
// StackPanel.cs
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

using System.Windows.Media;
using System.Windows;
using Mono;

namespace System.Windows.Controls {
	public partial class StackPanel : Panel {

		public Orientation Orientation {
			get { return (Orientation) GetValue (OrientationProperty); }
			set { SetValue(OrientationProperty, value); }
		}

		public static readonly DependencyProperty OrientationProperty = 
		DependencyProperty.Register ("Orientation", typeof (Orientation), typeof (StackPanel), null);

		protected override sealed Size MeasureOverride (Size availableSize) {
			Size result = new Size (0, 0);
			foreach (UIElement child in this.Children) {
				child.Measure (availableSize);
				Size size = child.DesiredSize;
				result.Height += size.Height;
				result.Width = Math.Max (result.Width, size.Width);
			}
			return result;
		}

		protected override sealed Size ArrangeOverride (Size finalSize) {
			Size result = new Size (0, 0);
			foreach (UIElement child in this.Children) {
				child.Measure (finalSize);
				Size size = child.DesiredSize;
				size.Width = Math.Max (size.Width, finalSize.Width);
				child.Arrange (new Rect (0, result.Height, size.Width, size.Height));
				result.Height += size.Height;
				result.Width = Math.Max (result.Width, size.Width);
			}
			return finalSize;
		}
	}
}
