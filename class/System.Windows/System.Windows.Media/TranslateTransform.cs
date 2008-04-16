//
// System.Windows.Media.TranslateTransform class
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

	public sealed class TranslateTransform : Transform {

		public static readonly DependencyProperty XProperty =
			DependencyProperty.Lookup (Kind.TRANSLATETRANSFORM, "X", typeof (double));
		public static readonly DependencyProperty YProperty =
			DependencyProperty.Lookup (Kind.TRANSLATETRANSFORM, "Y", typeof (double));

		public TranslateTransform () : base (NativeMethods.translate_transform_new ())
		{
		}

		internal TranslateTransform (IntPtr raw) : base (raw)
		{
		}
		
		public double X {
			get { return (double) GetValue (XProperty); }
			set { SetValue (XProperty, value); }
		}

		public double Y {
			get { return (double) GetValue (YProperty); }
			set { SetValue (YProperty, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.TRANSLATETRANSFORM;
		}
	}
}
