//
// Unit tests for System.Windows.Ink.Stroke
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
using System.Windows.Ink;
using System.Windows.Input;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Ink {

	[TestClass]
	public partial class StrokeTest {

		[TestMethod]
		public void Stroke_Default ()
		{
			Stroke s = new Stroke ();

			Assert.AreEqual (0, s.StylusPoints.Count, "StylusPoints");

			Assert.Throws<ArgumentException> (delegate {
				s.HitTest (null);
			}, "HitTest-null");
			Assert.IsFalse (s.HitTest (s.StylusPoints), "HitTest-StylusPoints");
		}

		[TestMethod]
		[MoonlightBug]
		public void Stroke_Default_Bounds ()
		{
			Stroke s = new Stroke ();

			Rect bounds = s.GetBounds ();
			Assert.AreEqual (-1.5, bounds.Top, "Top");
			Assert.AreEqual (-1.5, bounds.Left, "Left");
			Assert.AreEqual (3, bounds.Height, "Height");
			Assert.AreEqual (3, bounds.Width, "Width");
		}

		[TestMethod]
		public void Stroke_StylusPointCollection ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			spc.Add (new StylusPoint (1, 2));

			Stroke s = new Stroke (spc);
			Assert.AreEqual (1, s.StylusPoints.Count, "StylusPoints-1");

			spc.Add (new StylusPoint (3, 4));
			Assert.AreEqual (2, s.StylusPoints.Count, "StylusPoints-2");

			s.StylusPoints.Add (new StylusPoint (5, 6));
			Assert.AreEqual (3, s.StylusPoints.Count, "StylusPoints-3a");
			Assert.AreEqual (3, spc.Count, "StylusPoints-3b");

			Assert.Throws<ArgumentException> (delegate {
				s.HitTest (null);
			}, "HitTest-null");
			Assert.IsTrue (s.HitTest (s.StylusPoints), "HitTest-StylusPoints");
		}

		[TestMethod]
		[MoonlightBug]
		public void Stroke_StylusPointCollection_Bounds ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			spc.Add (new StylusPoint (1, 2));

			Stroke s = new Stroke (spc);
			Rect bounds = s.GetBounds ();
			Assert.AreEqual (-0.5, bounds.Left, "Left");
			Assert.AreEqual (0.5, bounds.Top, "Top");
			Assert.AreEqual (3, bounds.Height, "Height");
			Assert.AreEqual (3, bounds.Width, "Width");
		}
	}
}

