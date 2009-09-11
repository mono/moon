/*
 * RenderingEventArgs.cs.
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
using Mono;

namespace System.Windows.Media
{	
	public sealed class RenderingEventArgs : EventArgs
	{
		internal IntPtr native;

		internal RenderingEventArgs (IntPtr raw)
		{
			native = raw;
			NativeMethods.event_object_ref (native);
		}

		~RenderingEventArgs ()
		{
			if (native != IntPtr.Zero) {
				NativeMethods.event_object_unref (native);
				native = IntPtr.Zero;
			}
		}

		public TimeSpan RenderingTime {
			get { return new TimeSpan (NativeMethods.rendering_event_args_get_rendering_time (native)); }
		}
	}
}
