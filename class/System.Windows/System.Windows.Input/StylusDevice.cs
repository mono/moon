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

using Mono;
using System;

namespace System.Windows.Input
{
	public sealed partial class StylusDevice
	{
		private MouseEventArgs mouse_event_args;
		private StylusInfo stylus;
		
		internal StylusDevice (MouseEventArgs args)
		{
			mouse_event_args = args;
		}
		
		public bool Inverted {
			get {
				if (stylus == null) {
					IntPtr retval = NativeMethods.mouse_event_args_get_stylus_info (mouse_event_args.NativeHandle);
					stylus = (StylusInfo) NativeDependencyObjectHelper.Lookup (Kind.STYLUSINFO, retval);
				}
				
				return stylus.IsInverted;
			}
		}
		
		public StylusPointCollection GetStylusPoints (UIElement relativeTo)
		{
			IntPtr col = NativeMethods.mouse_event_args_get_stylus_points (mouse_event_args.NativeHandle, relativeTo == null ? IntPtr.Zero : relativeTo.native);
			if (col == IntPtr.Zero)
				return null;

			return (StylusPointCollection) NativeDependencyObjectHelper.Lookup (Kind.STYLUSPOINT_COLLECTION, col);
		}
	}
}
