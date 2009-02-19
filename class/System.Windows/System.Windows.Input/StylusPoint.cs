// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

namespace System.Windows.Input
{
	public struct StylusPoint
	{
		IntPtr native;
		
		internal StylusPoint (IntPtr raw)
		{
			// XXX this should go away, but doing so will
			// require some changes elsewhere
			// (DependencyObject.cs, for instance).
			native = raw;
		}
		
		public StylusPoint (double x, double y)
		{
			native = NativeMethods.stylus_point_new ();
			NativeMethods.stylus_point_set_x (native, x);
			NativeMethods.stylus_point_set_y (native, y);
		}

		public float PressureFactor {
			get { return (float) NativeMethods.stylus_point_get_pressure_factor (native); }
			set { NativeMethods.stylus_point_set_pressure_factor (native, (double) value); }
		}

		public double X {
			get { return NativeMethods.stylus_point_get_x (native); }
			set { NativeMethods.stylus_point_set_x (native, value); }
		}

		public double Y {
			get { return NativeMethods.stylus_point_get_y (native); }
			set { NativeMethods.stylus_point_set_y (native, value); }
		}
	}
}
