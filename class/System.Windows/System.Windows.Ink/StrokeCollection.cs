// Author:
//   Rolf Bjarne Kvinge  (RKvinge@novell.com)
//
// Copyright 2007 Novell, Inc.
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
using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Runtime.InteropServices;
using Mono;
using MS.Internal;

namespace System.Windows.Ink
{
	public sealed class StrokeCollection : Collection <Stroke>
	{
		public StrokeCollection() : base (NativeMethods.stroke_collection_new ())
		{
		}
		
		internal StrokeCollection (IntPtr raw) : base (raw)
		{
		}

		public Rect GetBounds ()
		{
			UnmanagedRect urect = new UnmanagedRect();
			NativeMethods.stroke_collection_get_bounds (native, ref urect);
			return new Rect (urect.left, urect.top, urect.width, urect.height);
		}

		public StrokeCollection HitTest (StylusPointCollection stylusPointCollection)
		{
			IntPtr col = NativeMethods.stroke_collection_hit_test (native, stylusPointCollection.native);
			if (col == IntPtr.Zero)
				return null;

			return (StrokeCollection)DependencyObject.Lookup (Kind.STROKE_COLLECTION, col);
		}
		
		internal override Kind GetKind ()
		{
			return Kind.STROKE_COLLECTION;
		}
	}
}
