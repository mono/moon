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
		public static event EventHandler Rendering {
			add {
				Deployment.Current.Surface.TimeManager.Render += value;
			}
			remove {
				Deployment.Current.Surface.TimeManager.Render -= value;
			}
		}
	}
}