/*
 * RenderTargetBitmap.cs.
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
using System.Windows.Shapes;

namespace System.Windows.Media.Imaging
{
	public sealed class RenderTargetBitmap
	{
#if NET_2_1
		[SecurityCritical ()]
#endif
		public RenderTargetBitmap (int width, int height, IntPtr bitmap)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public Rect? Render (UIElement visual)
		{
			throw new NotImplementedException ();
		}

	}
}
