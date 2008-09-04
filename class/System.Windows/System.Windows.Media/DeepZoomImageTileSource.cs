/*
 * DeepZoomImageTileSource.cs.
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
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Xml;

namespace System.Windows.Media
{	
	public sealed partial class DeepZoomImageTileSource : MultiScaleTileSource
	{
		delegate void DownloadedHandler (string path);

		[DllImport ("moon")]
		extern static void deep_zoom_image_tile_source_download_urisource (IntPtr instance, string uri, DownloadedHandler callback);

		public DeepZoomImageTileSource (Uri sourceUri) : base ()
		{
			throw new NotImplementedException ("Set the source after attaching the TileSource to the MultiScaleImage control");
		}
		
		protected override void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources)
		{
			throw new NotImplementedException ();
		}

//		public static readonly DependencyProperty UriSourceProperty =
//			DependencyProperty.Lookup (Kind.DEEPZOOMIMAGETILESOURCE, "UriSource", typeof (string));
		
		Uri uri_source;
		public Uri UriSource {
			get { return uri_source; }
			set {
				uri_source = value;
				deep_zoom_image_tile_source_download_urisource (this.native, uri_source.ToString (), OnDownloaded);
			}
		}

		void OnDownloaded (string path)
		{
			Console.WriteLine ("OnDownloaded {0}", path);
		}

	}
}
