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
		bool collection = false;
		//common attributes
		string format;

		//image attributes
		ulong overlap;

		//collection attributes
		byte max_level;

		System.Runtime.InteropServices.GCHandle handle;
		void Initialize ()
		{
			Console.WriteLine ("Initialize");
			NativeMethods.DownloadedHandler func = new NativeMethods.DownloadedHandler (ParseDeepZoom);
			handle = System.Runtime.InteropServices.GCHandle.Alloc (func);
			NativeMethods.deep_zoom_image_tile_source_set_downloaded_cb (native, func);
		}

		public DeepZoomImageTileSource (Uri sourceUri) : this ()
		{
			Console.WriteLine ("new DZITS");
			UriSource = sourceUri;
		}

		~DeepZoomImageTileSource ()
		{
			handle.Free ();
		}
		
		protected override void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources)
		{
			Console.WriteLine ("GetTileLayers {0} {1} {2}", tileLevel, tilePositionX, tilePositionY);
			string uri = UriSource.ToString ();
			uri = String.Format ("{0}/{1}/{2}_{3}.{4}", uri.Substring (0, uri.LastIndexOf ('/')), tileLevel, tilePositionX, tilePositionY, format);
			tileImageLayerSources.Add (new Uri (uri, UriSource.IsAbsoluteUri ? UriKind.Absolute : UriKind.Relative));
		}

		void ParseDeepZoom (string path)
		{
			Console.WriteLine ("ParseDeepZoom {0}", path);
			XmlReader reader = XmlReader.Create ("file://" + path);
			bool done = false;
			while (reader.Read () && !done)
				switch (reader.Name) {
				case "Collection":
					collection = true;
					ParseCollection (reader);
					done = true;
					break;
				case "Image":
					collection = false;
					ParseImage (reader);
					done = true;
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseDeepZoom: Not parsing {0} element", reader.Name);
					break;
				}
			//Fire the OpenCompleted event
		}

		//Collections
		void ParseCollection (XmlReader reader)
		{
			Console.WriteLine ("Parsing <Collection>");
			//FIXME, parse attributes here
			while (reader.Read () && !(reader.Name == "Collection" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "Items":
					ParseItems (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseCollection: Not parsing Collection.{0}", reader.Name);
					break;

				}
			
		}

		void ParseItems (XmlReader reader)
		{
			Console.WriteLine ("Parsing <Items>");
			while (reader.Read () && !(reader.Name == "Items" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
					default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseCollection: Not parsing Collection.Items.{0}", reader.Name);
					break;
				}

		}

		//Images
		void ParseImage (XmlReader reader)
		{
			Console.WriteLine ("Parsing <Image>");
			TileWidth = TileHeight = (int)UInt64.Parse (reader["TileSize"]);
			format = reader["Format"];
			while (reader.Read () && !(reader.Name == "Image" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "Size":
					ParseSize (reader);
					break;
				case "DisplayRects":
					ParseDisplayRects (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseImage: Not parsing Image.{0}", reader.Name);
					break;
				}
		}

		void ParseSize (XmlReader reader)
		{
			Console.WriteLine ("Parsing <Size>");
			if (collection) {
				throw new NotImplementedException ();	
			} else {
				ImageWidth = (long)UInt64.Parse (reader["Width"]);
				ImageHeight = (long)UInt64.Parse (reader["Height"]);
			}
			
		}

		void ParseDisplayRects (XmlReader reader)
		{
			Console.WriteLine ("Parsing <DisplayRects>");
			while (reader.Read () && !(reader.Name == "DisplayRects" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseSize: Not parsing Image.DisplayRects.{0}", reader.Name);
					break;
				}
			
		}

		int read_int (XmlReader reader)
		{
			reader.Read ();
			int val = reader.ReadContentAsInt ();
			reader.Skip ();
			return val;
		}
	}
}
