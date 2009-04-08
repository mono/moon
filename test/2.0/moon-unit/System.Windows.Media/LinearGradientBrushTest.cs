//
// Unit tests for LinearGradientBrush
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
	public partial class LinearGradientBrushTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();

			Assert.AreEqual (1.0d, lgb.EndPoint.X, "EndPoint.X");
			Assert.AreEqual (1.0d, lgb.EndPoint.Y, "EndPoint.Y");

			CheckDefaults (lgb, 0);
		}

		[TestMethod]
		public void AngleCtor ()
		{
			for (int i=0; i <= 360; i++) {
				LinearGradientBrush lgb = new LinearGradientBrush (null, i);
				Assert.IsTrue (Math.Abs (Math.Cos (i * Math.PI / 180) - lgb.EndPoint.X) < 0.0001, i.ToString () + "-EndPoint.X");
				Assert.IsTrue (Math.Abs (Math.Sin (i * Math.PI / 180) - lgb.EndPoint.Y) < 0.0001, i.ToString () + "-EndPoint.Y");
				CheckDefaults (lgb, 0);
			}
		}

		[TestMethod]
		public void CollectionCtorSingle ()
		{
			GradientStopCollection gsc = new GradientStopCollection ();
			gsc.Add (new GradientStop ());
			LinearGradientBrush rgb = new LinearGradientBrush (gsc, 0.0d);
			CheckDefaults (rgb, 1);
			Assert.IsTrue (Object.ReferenceEquals (gsc, rgb.GradientStops), "Same GradientStops");

			GradientStop gs1 = rgb.GradientStops [0];
			Assert.AreEqual ("#00000000", gs1.Color.ToString (), "1.Color");
			Assert.AreEqual (0.0, gs1.Offset, "1.Offset");
		}

		static public void CheckDefaults (LinearGradientBrush lgb, int count)
		{
			Assert.AreEqual (0.0d, lgb.StartPoint.X, "Start.X");
			Assert.AreEqual (0.0d, lgb.StartPoint.Y, "Start.Y");
			GradientBrushTest.CheckDefaults (lgb, count);
		}

		[TestMethod]
		[MoonlightBug ("ML has the property Matrix frozen")]
		public void EnsureNotFrozen ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (lgb.RelativeTransform as MatrixTransform), "RelativeTransform");
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (lgb.Transform as MatrixTransform), "Transform");
		}

		[TestMethod]
		[MoonlightBug ("Looks like a bad SL2 bug")]
		public void Destructive ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();
			// from this instance we can change all default values
			BrushTest.DestructiveRelativeTransform (lgb);
			BrushTest.DestructiveTransform (lgb);
			// but it's safe to execute since we revert the changes
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void RelativeTransform ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();
			BrushTest.RelativeTransform (lgb);
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void Transform ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();
			BrushTest.Transform (lgb);
		}
	}
}
