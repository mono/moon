//
// System.Windows.Media.GradientBrush class
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using Mono;

namespace System.Windows.Media {

	public abstract class GradientBrush : Brush {

		public static readonly DependencyProperty ColorInterpolationModeProperty =
			DependencyProperty.Lookup (Kind.GRADIENTBRUSH, "ColorInterpolationMode", typeof (ColorInterpolationMode));
		public static readonly DependencyProperty GradientStopsProperty =
			DependencyProperty.Lookup (Kind.GRADIENTBRUSH, "GradientStops", typeof (GradientStopCollection));
		public static readonly DependencyProperty MappingModeProperty =
			DependencyProperty.Lookup (Kind.GRADIENTBRUSH, "MappingMode", typeof (BrushMappingMode));
		public static readonly DependencyProperty SpreadMethodProperty =
			DependencyProperty.Lookup (Kind.GRADIENTBRUSH, "SpreadMethod", typeof (GradientSpreadMethod));


		public GradientBrush () : base (NativeMethods.gradient_brush_new ())
		{
		}
		
		internal GradientBrush (IntPtr raw) : base (raw)
		{
		}


		public ColorInterpolationMode ColorInterpolationMode {
			get { return (ColorInterpolationMode) GetValue (ColorInterpolationModeProperty); }
			set { SetValue (ColorInterpolationModeProperty, value); }
		}

		public GradientStopCollection GradientStops {
			get { return (GradientStopCollection) GetValue (GradientStopsProperty); }
			set { SetValue (GradientStopsProperty, value); }
		}

		public BrushMappingMode MappingMode {
			get { return (BrushMappingMode) GetValue (MappingModeProperty); }
			set { SetValue (MappingModeProperty, value); }
		}

		public GradientSpreadMethod SpreadMethod {
			get { return (GradientSpreadMethod) GetValue (SpreadMethodProperty); }
			set { SetValue (SpreadMethodProperty, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.GRADIENTBRUSH;
		}

	}
}
