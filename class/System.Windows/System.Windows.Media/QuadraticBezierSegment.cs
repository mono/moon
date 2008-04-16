//
// System.Windows.Media.QuadraticBezierSegment class
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

	public sealed class QuadraticBezierSegment : PathSegment {

		public static readonly DependencyProperty Point1Property =
			DependencyProperty.Lookup (Kind.QUADRATICBEZIERSEGMENT, "Point1", typeof (Point));
		public static readonly DependencyProperty Point2Property =
			DependencyProperty.Lookup (Kind.QUADRATICBEZIERSEGMENT, "Point2", typeof (Point));

		public QuadraticBezierSegment () : base (NativeMethods.quadratic_bezier_segment_new ())
		{
		}
		
		internal QuadraticBezierSegment (IntPtr raw) : base (raw)
		{
		}

		public Point Point1 {
			get { return (Point) GetValue (Point1Property); }
			set { SetValue (Point1Property, value); }
		}

		public Point Point2 {
			get { return (Point) GetValue (Point2Property); }
			set { SetValue (Point2Property, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.QUADRATICBEZIERSEGMENT;
		}
	}
}
