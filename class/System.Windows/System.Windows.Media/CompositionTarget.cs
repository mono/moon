/*
 * CompositionTarget.cs.
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
	public static class CompositionTarget
	{
		private static EventHandlerList events = new EventHandlerList ();
		static UnmanagedEventHandler rendering_proxy = Events.CreateSafeHandler (UnmanagedRendering);

		private static void UnmanagedRendering (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			EventHandler h = (EventHandler)events[RenderingEvent];
			if (h != null) {
				h (null, new RenderingEventArgs (calldata));
			}
		}

		static object RenderingEvent = new object ();
		public static event EventHandler Rendering {
			add {
				if (events[RenderingEvent] == null) {
					IntPtr t = NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native);
					NativeMethods.event_object_add_handler (t, "Render", rendering_proxy, t, IntPtr.Zero);
				}
				events.AddHandler (RenderingEvent, value);
			}
			remove {
				events.RemoveHandler (RenderingEvent, value);
				if (events[RenderingEvent] == null) {
					IntPtr t = NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native);
					NativeMethods.event_object_remove_handler (t, "Render", rendering_proxy, t);
				}
			}
		}
	}
}
