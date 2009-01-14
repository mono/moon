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
		internal int ImageWidth {
			get { return NativeMethods.multi_scale_tile_source_get_image_width (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_image_width (this.native, value); }
		}
		internal int ImageHeight {
			get { return NativeMethods.multi_scale_tile_source_get_image_height (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_image_height (this.native, value); }
		}
		internal int TileWidth {
			get { return NativeMethods.multi_scale_tile_source_get_tile_width (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_width (this.native, value); }
		}
		internal int TileHeight {
			get { return NativeMethods.multi_scale_tile_source_get_tile_height (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_height (this.native, value); }
		}
		internal int TileOverlap {
			get { return NativeMethods.multi_scale_tile_source_get_tile_overlap (this.native); }
			set { NativeMethods.multi_scale_tile_source_set_tile_overlap (this.native, value); }
		}

		protected TimeSpan TileBlendTime {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}
		
		void Initialize ()
		{
			NativeMethods.multi_scale_tile_source_set_image_uri_func (native, GetImageUri);
		}

		public MultiScaleTileSource (int imageWidth, int imageHeight, int tileWidth, int tileHeight, int tileOverlap)
		{
			ImageWidth = imageWidth;
			ImageHeight = imageHeight;
			TileWidth = tileWidth;
			TileHeight = tileHeight;
			TileOverlap = tileOverlap;
		}

		public MultiScaleTileSource (long imageWidth, long imageHeight, int tileWidth, int tileHeight, int tileOverlap)
		{
			throw new NotImplementedException ();
		}
		
		protected void InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
		{
			throw new NotImplementedException ();
		}
		
		protected abstract void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources);

		protected internal string GetImageUri (int tileLevel, int tilePositionX, int tilePositionY)
		{
			List<object> list = new List<object> ();
			GetTileLayers (tileLevel, tilePositionX, tilePositionY, list);
			if (list.Count == 0)
				return null;
			return (list[0] as Uri).ToString ();
		}
	}
}
