//
// System.Windows.Shapes.Line
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
namespace System.Windows.Shapes {

	public sealed class Line : Shape {

		public static readonly DependencyProperty X1Property =
			DependencyProperty.Lookup (Kind.LINE, "X1", typeof (double));
		public static readonly DependencyProperty Y1Property =
			DependencyProperty.Lookup (Kind.LINE, "Y1", typeof (double));
		public static readonly DependencyProperty X2Property =
			DependencyProperty.Lookup (Kind.LINE, "X2", typeof (double));
		public static readonly DependencyProperty Y2Property =
			DependencyProperty.Lookup (Kind.LINE, "Y2", typeof (double));

		public Line () : base (Mono.NativeMethods.line_new ())
		{
		}

		internal Line (IntPtr raw) : base (raw)
		{
		}
		
		public double X1 {
			get { return (double) GetValue (X1Property); }
			set { SetValue (X1Property, value); }
		}

		public double Y1 {
			get { return (double) GetValue (Y1Property); }
			set { SetValue (Y1Property, value); }
		}

		public double X2 {
			get { return (double) GetValue (X2Property); }
			set { SetValue (X2Property, value); }
		}

		public double Y2 {
			get { return (double) GetValue (Y2Property); }
			set { SetValue (Y2Property, value); }
		}
		
		internal override Kind GetKind ()
		{
			return Kind.LINE;
		}
	}
}
