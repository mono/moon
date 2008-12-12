//
// CornerRadius Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class CornerRadiusTest {

		static void CheckProperties (CornerRadius cr, double value)
		{
			Assert.AreEqual (value, cr.TopLeft, "TopLeft");
			Assert.AreEqual (value, cr.TopRight, "TopRight");
			Assert.AreEqual (value, cr.BottomRight, "BottomRight");
			Assert.AreEqual (value, cr.BottomLeft, "BottomLeft");
		}

		[TestMethod]
		public void CtorDefault ()
		{
			CornerRadius cr = new CornerRadius ();
			CheckProperties (cr, 0);
			Assert.AreEqual ("0,0,0,0", cr.ToString (), "ToString");
		}

		[TestMethod]
		public void CtorDouble ()
		{
			CornerRadius cr = new CornerRadius (0);
			CheckProperties (cr, 0);
			cr.TopLeft = 1;
			cr.TopRight = 2;
			cr.BottomRight = 3;
			cr.BottomLeft = 4;
			Assert.AreEqual ("1,2,3,4", cr.ToString (), "ToString");

			CornerRadius cr2 = new CornerRadius (0);

			Assert.IsFalse (cr.Equals (cr2), "!Equals");
			Assert.IsFalse (cr2.Equals (cr), "!Equals2");
			Assert.IsFalse (cr.Equals (null), "Equals(null)");
			Assert.IsTrue (cr.Equals (cr), "Equals(self)");

			Assert.Throws<ArgumentException> (delegate {
				new CornerRadius (-0.1);
			}, "Negative");
		}

		[TestMethod]
		public void CtorFourDoubles ()
		{
			CornerRadius cr = new CornerRadius (4, 3, 2, 1);
			Assert.AreEqual ("4,3,2,1", cr.ToString (), "ToString");

			CornerRadius cr2 = new CornerRadius (4, 3, 2, 1);
			Assert.IsTrue (cr == cr2, "== true");
			Assert.IsFalse (cr != cr2, "== false");

			cr.TopLeft = 0;
			cr.TopRight = 0;
			cr.BottomRight = 0;
			cr.BottomLeft = 0;
			CheckProperties (cr, 0);

			Assert.IsFalse (cr == cr2, "== false");
			Assert.IsTrue (cr != cr2, "== true");
		}

		[TestMethod]
		public void CtorSpecialValues ()
		{
			CornerRadius cr = new CornerRadius (Double.MaxValue);
			CheckProperties (cr, Double.MaxValue);

			Assert.Throws<ArgumentException> (delegate {
				new CornerRadius (Double.MinValue);
			}, "Double.MinValue");

			Assert.Throws<ArgumentException> (delegate {
				new CornerRadius (Double.NaN);
			}, "Double.NaN");

			cr = new CornerRadius (Double.PositiveInfinity);
			CheckProperties (cr, Double.PositiveInfinity);

			Assert.Throws<ArgumentException> (delegate {
				new CornerRadius (Double.NegativeInfinity);
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void PropertiesSpecialValues ()
		{
			CornerRadius cr = new CornerRadius (Double.MaxValue);
			Assert.Throws<ArgumentException> (delegate {
				cr.TopLeft = Double.MinValue;
			}, "Double.MinValue");
			Assert.Throws<ArgumentException> (delegate {
				cr.TopRight = Double.NaN;
			}, "Double.NaN");
			Assert.Throws<ArgumentException> (delegate {
				cr.BottomRight = Double.NegativeInfinity;
			}, "Double.NegativeInfinity");
			Assert.Throws<ArgumentException> (delegate {
				cr.BottomLeft = -0.0001d;
			}, "Negative");
		}
	}
}
