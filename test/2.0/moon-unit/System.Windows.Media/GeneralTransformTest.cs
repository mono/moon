/*
 * GeneralTransformTest.cs
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
using System;
using System.Windows.Markup;
using System.Windows.Media;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public partial class GeneralTransformTest
	{
		[TestMethod]
		public void Inverses ()
		{
			GeneralTransform inverse;
			
			RotateTransform rotate = new RotateTransform ();
			inverse = rotate.Inverse;
			Assert.IsTrue (inverse is RotateTransform);
			
			ScaleTransform scale = new ScaleTransform ();
			inverse = rotate.Inverse;
			Assert.IsTrue (inverse is ScaleTransform);
			
			SkewTransform skew = new SkewTransform ();
			inverse = rotate.Inverse;
			Assert.IsTrue (inverse is SkewTransform);
			
			TranslateTransform translate = new TranslateTransform ();
			inverse = rotate.Inverse;
			Assert.IsTrue (inverse is TranslateTransform);
		}
	}
}
