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
		void Initialize ()
		{
			NativeMethods.deep_zoom_image_tile_source_set_downloaded_cb (native, ParseDeepZoom);
		}

		public DeepZoomImageTileSource (Uri sourceUri) : this ()
		{
			UriSource = sourceUri;
		}
		
		protected override void GetTileLayers (int tileLevel, int tilePositionX, int tilePositionY, IList<object> tileImageLayerSources)
		{
			Console.WriteLine ("GetTileLayers {0} {1} {2}", tileLevel, tilePositionX, tilePositionY);
			Level l;
			if (!levels.TryGetValue (tileLevel, out l))
				return;

			//FIXME, check tile-presence
			if (l.tile_width * tilePositionX <= l.width && l.tile_height * tilePositionY <= l.height) {
				string uri = UriSource.ToString ();
				uri = String.Format ("{0}/{1}/{2}_{3}.jpg", uri.Substring (0, uri.LastIndexOf ('/')), tileLevel, tilePositionX, tilePositionY);
				tileImageLayerSources.Add (new Uri (uri, UriSource.IsAbsoluteUri ? UriKind.Absolute : UriKind.Relative));
				return;
			}

			

		}

		void ParseDeepZoom (string path)
		{
			Console.WriteLine ("Downloaded {0}", path);
			XmlReader reader = XmlReader.Create ("file://" + path);
			while (reader.Read ())
				switch (reader.Name) {
				case "image":
					ParseImage (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseDeepZoom: Not parsing {0} element", reader.Name);
					break;
				}
		}

		class TileRect {
			public int left;	
			public int top;	
			public int right;	
			public int bottom;	
		}

		class Level {
			public int width;
			public int height;
			public int tile_width;
			public int tile_height;
			public int tile_overlap_top;
			public int tile_overlap_left;
			public int tile_overlap_right;
			public int tile_overlap_bottom;
			public int offset_width;
			public int offset_height;
			public List<TileRect> tile_presence;
		}

		int num_levels = -1;

		Dictionary<int, Level> levels;

		void ParseImage (XmlReader reader)
		{
//			Console.WriteLine ("Parsing <image>");
			while (reader.Read () && !(reader.Name == "image" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "width":
					ImageWidth = read_int (reader);
					break;
				case "height":
					ImageHeight = read_int (reader);
					break;
				case "num-levels":
					num_levels = read_int (reader);
					break;
				case "levels":
					ParseLevels (reader);
					break;
				case "tile-presence":
					ParseTilePresence (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource::ParseImage: Not parsing image.{0}", reader.Name);
					break;
				}
		}

		void ParseLevels (XmlReader reader)
		{
//			Console.WriteLine ("Parsing <levels>");
			levels = new Dictionary<int, Level> (num_levels);
			while (reader.Read () && !(reader.Name == "levels" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "level":
					ParseLevel (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.levels.{0}", reader.Name);
					break;
				}
		}

		void ParseLevel (XmlReader reader)
		{
			int l = Convert.ToInt32 (reader["index"]);
//			Console.WriteLine ("Parsing <level> {0}", l);
			Level level = null;
			if (!levels.TryGetValue (l, out level)) {
				level = new Level ();
				levels [l] = level;
			}

			while (reader.Read () && !(reader.Name == "level" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "width":
					level.width = read_int (reader);
					break;
				case "height":
					level.height = read_int (reader);
					break;
				case "tile-size":
					while (reader.Read () && !(reader.Name == "tile-size" && reader.NodeType == XmlNodeType.EndElement))
						switch (reader.Name) {
						case "width":
							TileWidth = level.tile_width = read_int (reader);
							break;
						case "height":
							TileHeight = level.tile_height = read_int (reader);
							break;
						default:
							if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
								Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.levels.level.tile-size.{0}", reader.Name);
							break;
						}
					break;
				case "tile-overlap":
					while (reader.Read () && !(reader.Name == "tile-overlap" && reader.NodeType == XmlNodeType.EndElement))
						switch (reader.Name) {
						case "left":
							level.tile_overlap_left = read_int (reader);
							break;
						case "top":
							level.tile_overlap_top = read_int (reader);
							break;
						case "right":
							level.tile_overlap_right = read_int (reader);
							break;
						case "bottom":
							level.tile_overlap_bottom = read_int (reader);
							break;
						default:
							if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
								Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.levels.level.tile-overlap.{0}", reader.Name);
							break;
						}
					break;
				case "offset":
					while (reader.Read () && !(reader.Name == "offset" && reader.NodeType == XmlNodeType.EndElement))
						switch (reader.Name) {
						case "width":
							level.offset_width = read_int (reader);
							break;
						case "height":
							level.offset_height = read_int (reader);
							break;
						default:
							if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
								Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.levels.level.tile-size.{0}", reader.Name);
							break;
						}
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.levels.level.{0}", reader.Name);
					break;
				}
		}

		void ParseTilePresence (XmlReader reader)
		{
//			Console.WriteLine ("Parsing <tile-presence>");
			while (reader.Read () && !(reader.Name == "tile-presence" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "rect":	
					ParseRect (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.tile-presence.{0}", reader.Name);
					break;
				}
		}

		void ParseRect (XmlReader reader)
		{
//			Console.WriteLine ("Parsing <rect>");
			TileRect rect = new TileRect ();

			while (reader.Read () && !(reader.Name == "rect" && reader.NodeType == XmlNodeType.EndElement))
				switch (reader.Name) {
				case "level":
					int l = read_int (reader);
					Level level = null;
					if (!levels.TryGetValue (l, out level)) {
						level = new Level ();
						levels [l] = level;
					}
					if (level.tile_presence == null)
						level.tile_presence = new List<TileRect> ();
					level.tile_presence.Add (rect);
					break;
				case "left":
					rect.left = read_int (reader);
					break;
				case "top":
					rect.top = read_int (reader);
					break;
				case "right":
					rect.right = read_int (reader);
					break;
				case "bottom":
					rect.bottom = read_int (reader);
					break;
				default:
					if (!String.IsNullOrEmpty (reader.Name) && reader.NodeType != XmlNodeType.EndElement)
						Console.WriteLine ("DeepZoomImageTileSource: Not parsing image.tile-presence.rect.{0}", reader.Name);
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
