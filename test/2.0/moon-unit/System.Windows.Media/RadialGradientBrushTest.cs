//
// Unit tests for RadialGradientBrush
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Windows;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public partial class RadialGradientBrushTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush ();
			CheckDefaults (rgb, 0);
		}

		[TestMethod]
		public void ColorsCtor ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush (Colors.Black, Colors.White);
			CheckDefaults (rgb, 2);

			GradientStop gs1 = rgb.GradientStops [0];
			Assert.AreEqual ("#FF000000", gs1.Color.ToString (), "1.Color");
			Assert.AreEqual (0.0, gs1.Offset, "1.Offset");

			GradientStop gs2 = rgb.GradientStops [1];
			Assert.AreEqual ("#FFFFFFFF", gs2.Color.ToString (), "2.Color");
			Assert.AreEqual (1.0, gs2.Offset, "2.Offset");
		}

		[TestMethod]
		public void CollectionCtorNull ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush (null);
			CheckDefaults (rgb, 0);
		}

		[TestMethod]
		public void CollectionCtorEmpty ()
		{
			GradientStopCollection gsc = new GradientStopCollection ();
			RadialGradientBrush rgb = new RadialGradientBrush (gsc);
			CheckDefaults (rgb, 0);
			Assert.IsTrue (Object.ReferenceEquals (gsc, rgb.GradientStops), "Same GradientStops");
		}

		[TestMethod]
		public void CollectionCtorSingle ()
		{
			GradientStopCollection gsc = new GradientStopCollection ();
			gsc.Add (new GradientStop ());
			RadialGradientBrush rgb = new RadialGradientBrush (gsc);
			CheckDefaults (rgb, 1);
			Assert.IsTrue (Object.ReferenceEquals (gsc, rgb.GradientStops), "Same GradientStops");

			GradientStop gs1 = rgb.GradientStops [0];
			Assert.AreEqual ("#00000000", gs1.Color.ToString (), "1.Color");
			Assert.AreEqual (0.0, gs1.Offset, "1.Offset");
		}

		static public void CheckDefaults (RadialGradientBrush rgb, int count)
		{
			Assert.AreEqual (0.5d, rgb.Center.X, "Center.X");
			Assert.AreEqual (0.5d, rgb.Center.Y, "Center.Y");
			Assert.AreEqual (0.5d, rgb.GradientOrigin.X, "GradientOrigin.X");
			Assert.AreEqual (0.5d, rgb.GradientOrigin.Y, "GradientOrigin.Y");
			Assert.AreEqual (0.5d, rgb.RadiusX, "RadiusX");
			Assert.AreEqual (0.5d, rgb.RadiusY, "RadiusY");
			GradientBrushTest.CheckDefaults (rgb, count);
		}

		[TestMethod]
		[MoonlightBug ("ML has the property Matrix frozen")]
		public void EnsureNotFrozen ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush ();
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (rgb.RelativeTransform as MatrixTransform), "RelativeTransform");
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (rgb.Transform as MatrixTransform), "Transform");
		}

		[TestMethod]
		[MoonlightBug ("Looks like a bad SL2 bug")]
		public void Destructive ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush ();
			// from this instance we can change all default values
			BrushTest.DestructiveRelativeTransform (rgb);
			BrushTest.DestructiveTransform (rgb);
			// but it's safe to execute since we revert the changes
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void RelativeTransform ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush ();
			BrushTest.RelativeTransform (rgb);
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void Transform ()
		{
			RadialGradientBrush rgb = new RadialGradientBrush ();
			BrushTest.Transform (rgb);
		}
	}
}
