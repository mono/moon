//
// System.Windows.Media.TileBrush class
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

	public abstract class TileBrush : Brush {

		public static readonly DependencyProperty AlignmentXProperty = DependencyProperty.Lookup (Kind.TILEBRUSH, "AlignmentX", typeof (AlignmentX));
		public static readonly DependencyProperty AlignmentYProperty = DependencyProperty.Lookup (Kind.TILEBRUSH, "AlignmentY", typeof (AlignmentY));
		public static readonly DependencyProperty StretchProperty = DependencyProperty.Lookup (Kind.TILEBRUSH, "Stretch", typeof (int));


		public TileBrush () : base (NativeMethods.tile_brush_new ())
		{
		}
		
		internal TileBrush (IntPtr raw) : base (raw)
		{
		}


		public AlignmentX AlignmentX {
			get { return (AlignmentX) GetValue (AlignmentXProperty); }
			set { SetValue (AlignmentXProperty, value); }
		}

		public AlignmentY AlignmentY {
			get { return (AlignmentY) GetValue (AlignmentYProperty); }
			set { SetValue (AlignmentYProperty, value); }
		}

		public Stretch Stretch {
			get { return (Stretch) GetValue (StretchProperty); }
			set { SetValue (StretchProperty, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.TILEBRUSH;
		}
	}
}
