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
		public DeepZoomImageTileSource (Uri sourceUri) : base ()
		{
			throw new NotImplementedException ("Set the source after attaching the TileSource to the MultiScaleImage control");
		}
		
		protected override void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources)
		{
			throw new NotImplementedException ();
		}
            
	    

		public static DependencyProperty UriSourceProperty = 
			DependencyProperty.Register ("UriSource", typeof (Uri), typeof (DeepZoomImageTileSource), null);
		
		public Uri UriSource {
			get { return (Uri)GetValue(UriSourceProperty); }
			set { 
			//	SetValue(UriSourceProperty, value);
				NativeMethods.deep_zoom_image_tile_source_download_urisource (this.native, value.ToString (), ParseDeepZoom);
			}
		}

		void ParseDeepZoom (string path)
		{
			Console.WriteLine ("Downloaded {0}", path);
			XmlReader reader = XmlReader.Create ("file://" + path);
			while (reader.Read ())
				Console.WriteLine ("{1} {0}", reader.Depth, reader.Name);
		}

	}
}
