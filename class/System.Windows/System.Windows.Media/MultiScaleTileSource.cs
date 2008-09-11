/*
 * MultiScaleTileSource.cs.
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
using System.Collections.Generic;
using System.Security;

namespace System.Windows.Media
{	
	public abstract partial class MultiScaleTileSource : DependencyObject
	{
		protected internal int ImageWidth {
			get { return NativeMethods.multi_scale_tile_source_get_image_width (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_image_width (this.native, value); }
		}
		protected internal int ImageHeight {
			get { return NativeMethods.multi_scale_tile_source_get_image_height (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_image_height (this.native, value); }
		}
		protected internal int TileWidth {
			get { return NativeMethods.multi_scale_tile_source_get_tile_width (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_width (this.native, value); }
		}
		protected internal int TileHeight {
			get { return NativeMethods.multi_scale_tile_source_get_tile_height (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_height (this.native, value); }
		}
		protected internal int TileOverlap {
			get { return NativeMethods.multi_scale_tile_source_get_tile_overlap (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_overlap (this.native, value); }
		}

		public MultiScaleTileSource (int imageWidth, int imageHeight, int tileWidth, int tileHeight, int tileOverlap)
		{
			ImageWidth = imageWidth;
			ImageHeight = imageHeight;
			TileWidth = tileWidth;
			TileHeight = tileHeight;
			TileOverlap = tileOverlap;
		}
		
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		protected void InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
		{
			throw new NotImplementedException ();
		}
		
		protected abstract void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources);
	}
}
