/*
 * DeepZoomImageTileSource.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using Mono;
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Xml;

namespace System.Windows.Media
{	
	public sealed partial class DeepZoomImageTileSource : MultiScaleTileSource
	{
		public DeepZoomImageTileSource (Uri sourceUri) : this ()
		{
			if (sourceUri == null)
				return;

			IntPtr uuri = UnmanagedUri.FromUri (sourceUri);
	
			NativeMethods.deep_zoom_image_tile_source_strip_and_set_uri (native, uuri);

			Marshal.FreeHGlobal (uuri);
		}

		protected override void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources)
		{
			Console.WriteLine ("//FIXME");
			Console.WriteLine ("//P/Invoke the native GetTileLayer");
		}
	}
}
