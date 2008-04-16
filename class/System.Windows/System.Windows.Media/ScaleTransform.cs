//
// System.Windows.Media.ScaleTransform class
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

	public sealed class ScaleTransform : Transform {

		public static readonly DependencyProperty CenterXProperty =
			DependencyProperty.Lookup (Kind.SCALETRANSFORM, "CenterX", typeof (double));
		public static readonly DependencyProperty CenterYProperty =
			DependencyProperty.Lookup (Kind.SCALETRANSFORM, "CenterY", typeof (double));
		public static readonly DependencyProperty ScaleXProperty =
			DependencyProperty.Lookup (Kind.SCALETRANSFORM, "ScaleX", typeof (double));
		public static readonly DependencyProperty ScaleYProperty =
			DependencyProperty.Lookup (Kind.SCALETRANSFORM, "ScaleY", typeof (double));

		public ScaleTransform () : base (NativeMethods.scale_transform_new ())
		{
		}

		internal ScaleTransform (IntPtr raw) : base (raw)
		{
		}
		
		public double CenterX {
			get { return (double) GetValue (CenterXProperty); }
			set { SetValue (CenterXProperty, value); }
		}

		public double CenterY {
			get { return (double) GetValue (CenterYProperty); }
			set { SetValue (CenterYProperty, value); }
		}

		public double ScaleX {
			get { return (double) GetValue (ScaleXProperty); }
			set { SetValue (ScaleXProperty, value); }
		}

		public double ScaleY {
			get { return (double) GetValue (ScaleYProperty); }
			set { SetValue (ScaleYProperty, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.SCALETRANSFORM;
		}
	}
}
