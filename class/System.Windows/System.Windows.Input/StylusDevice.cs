/*
 * StylusDevice.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Security;

namespace System.Windows.Input
{
	public sealed partial class StylusDevice
	{
		private MouseEventArgs mouse_event_args;
		
		internal StylusDevice (MouseEventArgs args)
		{
			mouse_event_args = args;
		}
		
		public bool Inverted {
			get { throw new NotImplementedException (); }
		}
		
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public StylusPointCollection GetStylusPoints (UIElement relativeTo)
		{
			IntPtr col = NativeMethods.mouse_event_args_get_stylus_points (mouse_event_args.native, uiElement == null ? IntPtr.Zero : uiElement.native);
			if (col == IntPtr.Zero)
				return null;

			return (StylusPointCollection) DependencyObject.Lookup (Kind.STYLUSPOINT_COLLECTION, col);
		}
	}
}
