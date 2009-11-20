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
		public static readonly DependencyProperty OrientationProperty = 
			DependencyProperty.RegisterCore ("Orientation", typeof (Orientation), typeof (StackPanel), new PropertyMetadata (new PropertyChangedCallback (OnStackPanelOrientationChanged)));
		public Orientation Orientation {
			get { return (Orientation) GetValue (OrientationProperty); }
			set { SetValue(OrientationProperty, value); }
		}

		private static void OnStackPanelOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			StackPanel sp = d as StackPanel;

			if (sp == null)
				return;

			sp.InvalidateMeasure ();
			sp.InvalidateArrange ();
		}

		protected override sealed Size MeasureOverride (Size availableSize) {
			Size result = new Size (0,0);
			Size childAvailable = new Size (double.PositiveInfinity, double.PositiveInfinity);

			if (Orientation == Orientation.Vertical) {
				childAvailable.Width = availableSize.Width;
				if (!Double.IsNaN (this.Width))
					childAvailable.Width = this.Width;

				childAvailable.Width = Math.Min (childAvailable.Width, this.MaxWidth);
				childAvailable.Width = Math.Max (childAvailable.Width, this.MinWidth);
			}

			if (Orientation == Orientation.Horizontal) {
				childAvailable.Height = availableSize.Height;
				if (!Double.IsNaN (this.Height))
					childAvailable.Height = this.Height;

				childAvailable.Height = Math.Min (childAvailable.Height, this.MaxHeight);
				childAvailable.Height = Math.Max (childAvailable.Height, this.MinHeight);
			}

			foreach (UIElement child in this.Children) {
				child.Measure (childAvailable);
				Size size = child.DesiredSize;

				if (Orientation == Orientation.Vertical) {
					result.Height += size.Height;
					result.Width = Math.Max (result.Width, size.Width);
				} else {
					result.Width += size.Width;
					result.Height = Math.Max (result.Height, size.Height);
				}
			}

			return result;
		}

		protected override sealed Size ArrangeOverride (Size finalSize) {
			Size result = finalSize;
			Rect requested = new Rect (0,0,finalSize.Width, finalSize.Height);
			bool first = true;
			
			foreach (UIElement child in this.Children) {
				if (first) {
					if (Orientation == Orientation.Vertical)
						result.Height = 0;
					else
						result.Width = 0;
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

			if (!first)  {
				if (Orientation == Orientation.Vertical)
					result.Height = Math.Max (result.Height, finalSize.Height);
				else
					result.Width = Math.Max (result.Width, finalSize.Width);
			}
			return result;
		}
	}
}
