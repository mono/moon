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
		internal RenderingEventArgs (IntPtr raw)
		{
			RenderingTime = new TimeSpan (NativeMethods.rendering_event_args_get_rendering_time (raw));
		}

		public TimeSpan RenderingTime {
			get; private set;
		}
	}
}
