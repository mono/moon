/*
 * HostingRenderTargetBitmap.cs.
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

namespace System.Windows.Interop
{
	public sealed class HostingRenderTargetBitmap
	{
		public HostingRenderTargetBitmap (int width, int height, IntPtr bitmap)
		{
			throw new NotImplementedException ();
		}

		public Rect? Render (UIElement visual)
		{
			throw new NotImplementedException ();
		}
	}
}
