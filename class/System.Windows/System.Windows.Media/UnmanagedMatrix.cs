//
// System.Windows.Media.UnmanagedMatrix DO
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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

	// the unmanaged Matrix is a DependencyObject so we can set it's value from JScript
	internal partial class UnmanagedMatrix : DependencyObject {

		// FIXME: introduce a private DP that allows us to set all values in a single SetValue call
		public static readonly DependencyProperty M11Property = DependencyProperty.Lookup (Kind.MATRIX, "M11", typeof (double));
		public static readonly DependencyProperty M12Property = DependencyProperty.Lookup (Kind.MATRIX, "M12", typeof (double));
		public static readonly DependencyProperty M21Property = DependencyProperty.Lookup (Kind.MATRIX, "M21", typeof (double));
		public static readonly DependencyProperty M22Property = DependencyProperty.Lookup (Kind.MATRIX, "M22", typeof (double));
		public static readonly DependencyProperty OffsetXProperty = DependencyProperty.Lookup (Kind.MATRIX, "OffsetX", typeof (double));
		public static readonly DependencyProperty OffsetYProperty = DependencyProperty.Lookup (Kind.MATRIX, "OffsetY", typeof (double));

		public UnmanagedMatrix (Matrix m) :
			this ()
		{
			SetValue (UnmanagedMatrix.M11Property, m.M11);
			SetValue (UnmanagedMatrix.M12Property, m.M12);
			SetValue (UnmanagedMatrix.M21Property, m.M21);
			SetValue (UnmanagedMatrix.M22Property, m.M22);
			SetValue (UnmanagedMatrix.OffsetXProperty, m.OffsetX);
			SetValue (UnmanagedMatrix.OffsetYProperty, m.OffsetY);
		}
	}
}
