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

		protected override sealed Size MeasureOverride (Size availableSize)
		{
			Size childAvailable = new Size (double.PositiveInfinity, double.PositiveInfinity);
			Size measured = new Size (0, 0);
			
			if (Orientation == Orientation.Vertical) {
				// Vertical layout
				childAvailable.Width = availableSize.Width;
				if (!Double.IsNaN (this.Width))
					childAvailable.Width = this.Width;
				
				childAvailable.Width = Math.Min (childAvailable.Width, this.MaxWidth);
				childAvailable.Width = Math.Max (childAvailable.Width, this.MinWidth);
			} else {
				// Horizontal layout
				childAvailable.Height = availableSize.Height;
				if (!Double.IsNaN (this.Height))
					childAvailable.Height = this.Height;
				
				childAvailable.Height = Math.Min (childAvailable.Height, this.MaxHeight);
				childAvailable.Height = Math.Max (childAvailable.Height, this.MinHeight);
			}
			
			// Measure our children to get our extents
			foreach (UIElement child in Children) {
				child.Measure (childAvailable);
				Size size = child.DesiredSize;
				
				if (Orientation == Orientation.Vertical) {
					measured.Height += size.Height;
					measured.Width = Math.Max (measured.Width, size.Width);
				} else {
					measured.Width += size.Width;
					measured.Height = Math.Max (measured.Height, size.Height);
				}
			}
			
			return measured;
		}

		protected override sealed Size ArrangeOverride (Size finalSize)
		{
			Size arranged = finalSize;
			
			if (Orientation == Orientation.Vertical)
				arranged.Height = 0;
			else
				arranged.Width = 0;
			
			// Arrange our children
			foreach (UIElement child in Children) {
				Size size = child.DesiredSize;
				
				if (Orientation == Orientation.Vertical) {
					size.Width = finalSize.Width;
					
					Rect childFinal = new Rect (0, arranged.Height, size.Width, size.Height);
					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);
					
					arranged.Width = Math.Max (arranged.Width, size.Width);
					arranged.Height += size.Height;
				} else {
					size.Height = finalSize.Height;
					
					Rect childFinal = new Rect (arranged.Width, 0, size.Width, size.Height);
					// reorder the elements if the the flowdirection is rtl
					if (FlowDirection  ==  FlowDirection.RightToLeft)
						childFinal.X = finalSize.Width - (arranged.Width + size.Width);

					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);
					
					arranged.Width += size.Width;
					arranged.Height = Math.Max (arranged.Height, size.Height);
				}
			}
			
			if (Orientation == Orientation.Vertical)
				arranged.Height = Math.Max (arranged.Height, finalSize.Height);
			else
				arranged.Width = Math.Max (arranged.Width, finalSize.Width);
			
			return arranged;
		}
	}
}
