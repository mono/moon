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
		[MinRuntimeVersion (3)]
		public void Inverses_sl3 ()
		{
			GeneralTransform inverse;
			
			RotateTransform rotate = new RotateTransform { Angle = 10.0 };
			inverse = rotate.Inverse;
			Assert.IsTrue (inverse is MatrixTransform, "#0");
			
			ScaleTransform scale = new ScaleTransform { ScaleX = 2.0, ScaleY = 2.0 };
			inverse = scale.Inverse;
			Assert.IsTrue (inverse is MatrixTransform, "#1");
			
			SkewTransform skew = new SkewTransform { AngleX = 1.0, AngleY = 1.0 };
			inverse = skew.Inverse;
			Assert.IsTrue (inverse is MatrixTransform, "#2");
			
			TranslateTransform translate = new TranslateTransform { X = 5.0, Y = 5.0 };
			inverse = translate.Inverse;
			Assert.IsTrue (inverse is MatrixTransform, "#3");
		}

		[TestMethod]
		[MaxRuntimeVersion (2)]
		[MoonlightBug ("we don't return null for .Inverse.  not sure why it was even present in sl2, it seems to return null unconditionally")]
		public void Inverses_sl2 ()
		{
			GeneralTransform inverse;
			
			RotateTransform rotate = new RotateTransform { Angle = 10.0 };
			inverse = rotate.Inverse;
			Assert.IsNull (inverse, "#0");
			
			ScaleTransform scale = new ScaleTransform { ScaleX = 2.0, ScaleY = 2.0 };
			inverse = scale.Inverse;
			Assert.IsNull (inverse, "#1");
			
			SkewTransform skew = new SkewTransform { AngleX = 1.0, AngleY = 1.0 };
			inverse = skew.Inverse;
			Assert.IsNull (inverse, "#2");
			
			TranslateTransform translate = new TranslateTransform { X = 5.0, Y = 5.0 };
			inverse = translate.Inverse;
			Assert.IsNull (inverse, "#3");
		}
	}
}
