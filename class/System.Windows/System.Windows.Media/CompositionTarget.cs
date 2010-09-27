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
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media
{
	public static class CompositionTarget
	{
		private static EventHandlerList event_list;

		private static EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList (null);
				return event_list;
			}
		}

		public static event EventHandler Rendering {
			add {
				EventList.RegisterEvent (NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native), EventIds.TimeManager_RenderEvent, value, Events.CreateRenderingEventHandlerDispatcher (value));
			}
			remove {
				EventList.UnregisterEvent (NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native), EventIds.TimeManager_RenderEvent, value);
			}
		}
	}
}
