/*
 * ImageBrushTest
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
using System;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public partial class ImageBrushTest
	{
		[TestMethod]
		[MoonlightBug ("ImageSource is null on ML")]
		public void DefaultCtor ()
		{
			ImageBrush ib = new ImageBrush ();
			Assert.IsTrue (ib.ImageSource is BitmapImage, "ImageSource");
			// ImageBrush's Transforms are null by default (true)
			TileBrushTest.CheckDefaults (ib, true);
		}

		[TestMethod]
		[MoonlightBug ("ImageSource is null on ML")]
		public void EmptyUri ()
		{
			ImageBrush ib = new ImageBrush ();
			Assert.IsNotNull (ib.ImageSource, "ImageSource");
			// I don't think we can (normally) create such an Uri (an exception is thrown)
			Assert.AreEqual (String.Empty, (ib.ImageSource as BitmapImage).UriSource.OriginalString, "UriSource.OriginalString");
		}

		[TestMethod]
		public void SetBitmapBrush ()
		{
			ImageBrush brush = new ImageBrush ();
			brush.ImageSource = new BitmapImage (new Uri ("http://www.example.com/test.jpg"));
		}

		// NOTE: Since ImageBrush.[Relative]Transform is null by default (unlike
		// other brush types) we don't run the following tests on it:
		// * BrushTest.DestructiveRelativeTransform
		// * BrushTest.DestructiveTransform
		// * BrushTest.RelativeTransform
		// * BrushTest.Transform
	}
}
