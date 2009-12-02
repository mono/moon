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

namespace System.Windows.Media.Media3D {

	// the unmanaged Matrix3D is a DependencyObject so we can set it's value from JScript
	internal partial class UnmanagedMatrix3D : DependencyObject {

		// FIXME: introduce a private DP that allows us to set all values in a single SetValue call
		public static readonly DependencyProperty M11Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M11", typeof (double));
		public static readonly DependencyProperty M12Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M12", typeof (double));
		public static readonly DependencyProperty M13Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M13", typeof (double));
		public static readonly DependencyProperty M14Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M14", typeof (double));
		public static readonly DependencyProperty M21Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M21", typeof (double));
		public static readonly DependencyProperty M22Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M22", typeof (double));
		public static readonly DependencyProperty M23Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M23", typeof (double));
		public static readonly DependencyProperty M24Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M24", typeof (double));
		public static readonly DependencyProperty M31Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M31", typeof (double));
		public static readonly DependencyProperty M32Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M32", typeof (double));
		public static readonly DependencyProperty M33Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M33", typeof (double));
		public static readonly DependencyProperty M34Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M34", typeof (double));
		public static readonly DependencyProperty OffsetXProperty = DependencyProperty.Lookup (Kind.MATRIX3D, "OffsetX", typeof (double));
		public static readonly DependencyProperty OffsetYProperty = DependencyProperty.Lookup (Kind.MATRIX3D, "OffsetY", typeof (double));
		public static readonly DependencyProperty OffsetZProperty = DependencyProperty.Lookup (Kind.MATRIX3D, "OffsetZ", typeof (double));
		public static readonly DependencyProperty M44Property = DependencyProperty.Lookup (Kind.MATRIX3D, "M44", typeof (double));

		public UnmanagedMatrix3D (Matrix3D m) :
			this ()
		{
			SetValue (UnmanagedMatrix3D.M11Property, m.M11);
			SetValue (UnmanagedMatrix3D.M12Property, m.M12);
			SetValue (UnmanagedMatrix3D.M13Property, m.M13);
			SetValue (UnmanagedMatrix3D.M14Property, m.M14);
			SetValue (UnmanagedMatrix3D.M21Property, m.M21);
			SetValue (UnmanagedMatrix3D.M22Property, m.M22);
			SetValue (UnmanagedMatrix3D.M23Property, m.M23);
			SetValue (UnmanagedMatrix3D.M24Property, m.M24);
			SetValue (UnmanagedMatrix3D.M31Property, m.M31);
			SetValue (UnmanagedMatrix3D.M32Property, m.M32);
			SetValue (UnmanagedMatrix3D.M33Property, m.M33);
			SetValue (UnmanagedMatrix3D.M34Property, m.M34);
			SetValue (UnmanagedMatrix3D.OffsetXProperty, m.OffsetX);
			SetValue (UnmanagedMatrix3D.OffsetYProperty, m.OffsetY);
			SetValue (UnmanagedMatrix3D.OffsetZProperty, m.OffsetZ);
			SetValue (UnmanagedMatrix3D.M44Property, m.M44);
		}
	}
}
