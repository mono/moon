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
		private static EventHandlerList EventList = new EventHandlerList ();

		public static event EventHandler Rendering {
			add {
				RegisterEvent (EventIds.TimeManager_RenderEvent, value, Events.CreateRenderingEventHandlerDispatcher (value));
			}
			remove {
				UnregisterEvent (EventIds.TimeManager_RenderEvent, value);
			}
		}

		private static void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			if (managedHandler == null)
				return;

			IntPtr t = NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native);
			int token = Events.AddHandler (t, eventId, nativeHandler);
			EventList.AddHandler (eventId, token, managedHandler, nativeHandler);
		}

		private static void UnregisterEvent (int eventId, Delegate managedHandler)
		{
			UnmanagedEventHandler nativeHandler = EventList.RemoveHandler (eventId, managedHandler);

			if (nativeHandler == null)
				return;

			IntPtr t = NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native);
			Events.RemoveHandler (t, eventId, nativeHandler);
		}

	}
}
