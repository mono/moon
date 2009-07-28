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
		Size _desired = new Size ();

		public static readonly DependencyProperty OrientationProperty = 
		DependencyProperty.RegisterCore ("Orientation", typeof (Orientation), typeof (StackPanel), null);
		public Orientation Orientation {
			get { return (Orientation) GetValue (OrientationProperty); }
			set { SetValue(OrientationProperty, value); }
		}

		protected override sealed Size MeasureOverride (Size availableSize) {
			Size result = new Size (0,0);

			Size childAvailable = (Orientation == Orientation.Vertical ? 
						childAvailable = new Size (double.PositiveInfinity, availableSize.Height) :
						childAvailable = new Size (availableSize.Width, double.PositiveInfinity)
			                       );

			foreach (UIElement child in this.Children) {
				if (child.Visibility == Visibility.Collapsed)
					continue;

				child.Measure (childAvailable);
				Size size = child.DesiredSize;

				if (Orientation == Orientation.Vertical) {
					result.Height += size.Height;
					//childAvailable.Height = Math.Max (childAvailable.Height - size.Height, 0);
					result.Width = Math.Max (result.Width, size.Width);
				} else {
					result.Width += size.Width;
					//childAvailable.Width = Math.Max (childAvailable.Width - size.Width, 0);
					result.Height = Math.Max (result.Height, size.Height);
				}
			}

			if (!Double.IsNaN (this.Width))
				result.Width = this.Width;

			if (!Double.IsNaN (this.Height))
				result.Height = this.Height;

			result.Width = Math.Min (result.Width, this.MaxWidth);
			result.Width = Math.Max (result.Width, this.MinWidth);
			
			result.Height = Math.Min (result.Height, this.MaxHeight);
			result.Height = Math.Max (result.Height, this.MinHeight);

			result.Width = Math.Min (result.Width, availableSize.Width);
			result.Height = Math.Min (result.Height, availableSize.Height);
			
			_desired = result;

			return result;
		}

		protected override sealed Size ArrangeOverride (Size finalSize) {
			Size result = finalSize;
			Rect requested = new Rect (0,0,finalSize.Width, finalSize.Height); //new Rect (0, 0, _desired.Width, _desired.Height);
			bool first = true;
			
			/*
			HorizontalAlignment horiz = Double.IsNaN (Width) ? this.HorizontalAlignment : HorizontalAlignment.Stretch;
			VerticalAlignment vert = Double.IsNaN (Height) ? this.VerticalAlignment : VerticalAlignment.Stretch;
			
			if (HorizontalAlignment == HorizontalAlignment.Stretch)
				requested.Width = finalSize.Width;
			
			if (VerticalAlignment == VerticalAlignment.Stretch)
				requested.Height = finalSize.Height;
			*/

			requested.Width = Math.Min (requested.Width, this.MaxWidth);
			requested.Width = Math.Max (requested.Width, this.MinWidth);
			
			requested.Height = Math.Min (requested.Height, this.MaxHeight);
			requested.Height = Math.Max (requested.Height, this.MinHeight);

			foreach (UIElement child in this.Children) {
				if (child.Visibility == Visibility.Collapsed)
					continue;

				if (first) {
					result = new Size ();
					first = false;
				}

				Size size = child.DesiredSize;
				if (Orientation == Orientation.Vertical) {
					size.Width = requested.Width;
					
					Rect childFinal = new Rect (0, result.Height, size.Width, size.Height);

					//childFinal.Intersect (requested);					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);

					result.Height += size.Height;
					result.Width = Math.Max (result.Width, size.Width);
				} else {
					size.Height = requested.Height;
					
					Rect childFinal = new Rect (result.Width, 0, size.Width, size.Height);

					//childFinal.Intersect (requested);					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);

					result.Width += size.Width;
					result.Height = Math.Max (result.Height, size.Height);
				}
			}

			return new Size (requested.Width, requested.Height);
		}
	}
}
