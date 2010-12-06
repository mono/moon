//
// InternalTransform.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Windows;
using System.Windows.Media;
using System;

namespace MS.Internal {
	internal partial class InternalTransform : GeneralTransform {
		public override Rect TransformBounds (Rect rect)
		{
			Point p1 = new Point (rect.Left, rect.Top);
			Point p2 = new Point (rect.Right, rect.Top);
			Point p3 = new Point (rect.Left, rect.Bottom);
			Point p4 = new Point (rect.Right, rect.Bottom);
			
			Rect r1 = new Rect (Transform (p1), Transform (p2));
			Rect r2 = new Rect (Transform (p3), Transform (p4));

			r1.Union (r2);

			return r1;
		}

		public override bool TryTransform (Point inPoint, out Point outPoint)
		{
			return NativeMethods.internal_transform_try_transform (native, inPoint, out outPoint);
		}
		
		public override GeneralTransform Inverse {
			get {
				GeneralTransform inverse;
				IntPtr rv;
				
				rv = NativeMethods.internal_transform_get_inverse (native);
				inverse = (GeneralTransform) NativeDependencyObjectHelper.Lookup (rv);
				NativeMethods.event_object_unref (rv);
				
				return inverse;
			}
		}
	}
}
