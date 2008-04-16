//
// System.Windows.Media.PathFigure class
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

	public sealed class PathFigure : DependencyObject {

		public static readonly DependencyProperty IsClosedProperty =
			DependencyProperty.Lookup (Kind.PATHFIGURE, "IsClosed", typeof (bool));
		public static readonly DependencyProperty SegmentsProperty = 
			DependencyProperty.Lookup (Kind.PATHFIGURE, "Segments", typeof (PathSegmentCollection));
		public static readonly DependencyProperty StartPointProperty = 
			DependencyProperty.Lookup (Kind.PATHFIGURE, "StartPoint", typeof (Point));

		public PathFigure () : base (NativeMethods.path_figure_new ())
		{
		}
		
		internal PathFigure (IntPtr raw) : base (raw)
		{
		}

		public bool IsClosed {
			get { return (bool) GetValue (IsClosedProperty); }
			set { SetValue (IsClosedProperty, value); }
		}
			
		public PathSegmentCollection Segments {
			get { return (PathSegmentCollection) GetValue (SegmentsProperty); }
			set { SetValue (SegmentsProperty, value); }
		}

		public Point StartPoint {
			get { return (Point) GetValue (StartPointProperty); }
			set { SetValue (StartPointProperty, value); }
		}
		
		internal override Kind GetKind ()
		{
			return Kind.PATHFIGURE;
		}
	}
}
