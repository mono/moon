/*
 * MatrixTrasformTest.cs.
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
	public class ImageBrushTest
	{
		[TestMethod]
		[MoonlightBug ("Shouldn't treat ImageSource as a string in native code")]
		public void SetBitmapBrush ()
		{
			ImageBrush brush = new ImageBrush ();
			brush.ImageSource = new BitmapImage (new Uri ("test.jpg"));
		}
	}
}
