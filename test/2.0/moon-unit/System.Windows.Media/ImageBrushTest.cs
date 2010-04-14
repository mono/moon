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
