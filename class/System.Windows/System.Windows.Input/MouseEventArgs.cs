//
// System.Windows.Input.MouseEventArgs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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

using Mono;
namespace System.Windows.Input {
	
	public class MouseEventArgs : RoutedEventArgs {
		IntPtr native;

		internal MouseEventArgs (IntPtr raw)
		{
			native = raw;
			NativeMethods.base_ref (native);
		}

		public MouseEventArgs ()
		{
		}
		
		~MouseEventArgs ()
		{
			if (this.native != IntPtr.Zero) {
				NativeMethods.base_unref (this.native);
				this.native = IntPtr.Zero;
			}
		}

		public Point GetPosition (UIElement uiElement)
		{
			double nx;
			double ny;

			NativeMethods.mouse_event_args_get_position (native, uiElement == null ? IntPtr.Zero : uiElement.native, out nx, out ny);

			return new Point (nx, ny);
		}

		public StylusInfo GetStylusInfo ()
		{
			IntPtr info = NativeMethods.mouse_event_args_get_stylus_info (native);
			if (info == IntPtr.Zero)
				return null;

			return (StylusInfo)DependencyObject.Lookup (Kind.STYLUSINFO, info);
		}
		
		public StylusPointCollection GetStylusPoints (UIElement uiElement)
		{
			IntPtr col = NativeMethods.mouse_event_args_get_stylus_points (native, uiElement == null ? IntPtr.Zero : uiElement.native);
			if (col == IntPtr.Zero)
				return null;

			return (StylusPointCollection)DependencyObject.Lookup (Kind.STYLUSPOINT_COLLECTION, col);
		}
		
		public bool Handled {
			get;
			set;
		}
	}
}
