//
// Unit tests for RectangleGeometry
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
	public partial class RectangleGeometryTest {

		[TestMethod]
		public void Defaults ()
		{
			RectangleGeometry rg = new RectangleGeometry ();
			Assert.AreEqual (0.0, rg.RadiusX, "RadiusX");
			Assert.AreEqual (0.0, rg.RadiusY, "RadiusY");
			Assert.AreEqual (new Rect (0, 0, 0, 0), rg.Rect, "Rect");
			GeometryTest.CheckDefaults (rg);
		}

		[TestMethod]
		public void CustomRect ()
		{
			RectangleGeometry rg = new RectangleGeometry ();
			rg.Rect = new Rect (1, 2, 3, 4);
			Assert.AreEqual (new Rect (1, 2, 3, 4), rg.Rect, "Rect");
			Assert.AreEqual (rg.Rect, rg.Bounds, "Bounds");
			Assert.IsNull (rg.Transform, "Transform");
		}

		[TestMethod]
		public void CustomRectWithRadixes ()
		{
			RectangleGeometry rg = new RectangleGeometry ();
			rg.Rect = new Rect (1, 2, 3, 4);
			rg.RadiusX = 5;
			rg.RadiusY = 6;
			Assert.AreEqual (new Rect (1, 2, 3, 4), rg.Rect, "Rect");
			Assert.AreEqual (rg.Rect, rg.Bounds, "Bounds");
			Assert.IsNull (rg.Transform, "Transform");
		}
	}
}
