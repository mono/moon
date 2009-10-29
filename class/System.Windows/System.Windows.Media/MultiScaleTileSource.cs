/*
 * MultiScaleTileSource.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008,2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using Mono;

using System;
using System.Collections.Generic;

namespace System.Windows.Media
{	
	public abstract partial class MultiScaleTileSource : DependencyObject
	{		
		System.Runtime.InteropServices.GCHandle handle;
		Action clear_image_uri_func;
		
		void Initialize ()
		{
			if (!(this is DeepZoomImageTileSource)) {
				ImageUriFunc func = new Mono.ImageUriFunc (GetImageUriSafe);
				handle = System.Runtime.InteropServices.GCHandle.Alloc (func);
				NativeMethods.multi_scale_tile_source_set_image_uri_func (native, func);
				
				clear_image_uri_func = ClearImageUri;
				
				if (!Deployment.QueueForShutdown (clear_image_uri_func)) {
					/* we're already shutting down */
					clear_image_uri_func ();
				}
			}
		}

		public MultiScaleTileSource (int imageWidth, int imageHeight, int tileWidth, int tileHeight, int tileOverlap) : this ((long)imageWidth, (long)imageHeight, tileWidth, tileHeight, tileOverlap)
		{
		}

		public MultiScaleTileSource (long imageWidth, long imageHeight, int tileWidth, int tileHeight, int tileOverlap) : this ()
		{
			if (imageWidth < 0)
				throw new ArgumentException ("imageWidth is negative");
			if (imageHeight < 0)
				throw new ArgumentException ("imageHeight is negative");
			if (tileWidth < 0)
				throw new ArgumentException ("tileWidth is negative");
			if (tileHeight < 0)
				throw new ArgumentException ("tileHeight is negative");
			if (tileOverlap < 0)
				throw new ArgumentException ("tileOverlap is negative");
				
			ImageWidth = imageWidth;
			ImageHeight = imageHeight;
			TileWidth = tileWidth;
			TileHeight = tileHeight;
			TileOverlap = tileOverlap;
		}

		void ClearImageUri ()
		{
			NativeMethods.multi_scale_tile_source_set_image_uri_func (native, null);
			handle.Free ();	
		}

		~MultiScaleTileSource () /* thread-safe: no p/invokes */
		{
			/*
			 * If we're destructed during normal execution, the only ref
			 * left on this object is the toggleref, which means that 
			 * the function pointers won't be called anymore.
			 * 
			 * If we're destructed during shutdown, the clear image uri
			 * func has already been executed, and the function pointers
			 * cleared out.
			 * 
			 * In neither case it's necessary to clear out the function
			 * pointers here: this prevents the need for locking in the
			 * native MultiScaleTileSource object.
			 */
			if (clear_image_uri_func != null)
				Deployment.UnqueueForShutdown (clear_image_uri_func);
			
			if (handle.IsAllocated)
				handle.Free ();
		}
		
		protected void InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
		{
			NativeMethods.multi_scale_tile_source_invalidate_tile_layer (native, level, tilePositionX, tilePositionY, tileLayer);
		}
		
		protected abstract void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources);

		private bool GetImageUriSafe (int tileLevel, int tilePositionX, int tilePositionY, IntPtr uuri, IntPtr ignore)
		{
			try {
				List<object> list = new List<object> ();
				GetTileLayers (tileLevel, tilePositionX, tilePositionY, list);
				if (list.Count == 0)
					return false;
				Uri uri = list[0] as Uri;
				if (uri == null)
					return false;
				return NativeMethods.uri_parse (uuri, uri.OriginalString, false);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in MultiScaleTileSource.GetImageUri: {0}", ex);
				} catch {
					// Ignore
				}
			}
			return false;
		}
	}
}
