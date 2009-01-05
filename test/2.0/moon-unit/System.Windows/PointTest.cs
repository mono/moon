//
// Unit tests for Point
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class PointTest {

		private void Compare (Point p)
		{
			Point p2 = p;
			Assert.IsFalse (p.Equals (null), "Equals(null)");
			Assert.IsTrue (p.Equals ((object) p), "Equals(object)");
			Assert.IsTrue (p.Equals (p), "Equals(Point)");
			Assert.IsTrue (p == p2, "==");
			Assert.IsFalse (p != p2, "!=");
		}

		[TestMethod]
		public void Defaults ()
		{
			Point p = new Point ();
			Assert.AreEqual (0.0, p.X, "X");
			Assert.AreEqual (0.0, p.Y, "Y");
			Assert.AreEqual ("0,0", p.ToString (), "ToString");
			Compare (p);
		}

		[TestMethod]
		public void NaN ()
		{
			Point p = new Point (Double.NaN, Double.NaN);
			Assert.IsTrue (Double.IsNaN (p.X), "X");
			Assert.IsTrue (Double.IsNaN (p.Y), "Y");
			Assert.AreEqual ("NaN,NaN", p.ToString (), "ToString");

			// special reserved case
			Point p2 = p;
			Assert.IsFalse (p.Equals ((object) p2), "Equals(object)");
			Assert.IsFalse (p.Equals (p2), "Equals(Point)");
			Assert.IsFalse (p == p2, "==");
			Assert.IsTrue (p != p2, "!=");
		}

		[TestMethod]
		public void NegativeInfinity ()
		{
			Point p = new Point (Double.NegativeInfinity, Double.NegativeInfinity);
			Assert.IsTrue (Double.IsNegativeInfinity (p.X), "X");
			Assert.IsTrue (Double.IsNegativeInfinity (p.Y), "Y");
			Assert.AreEqual ("-Infinity,-Infinity", p.ToString (), "ToString");
			Compare (p);
		}

		[TestMethod]
		public void PositiveInfinity ()
		{
			Point p = new Point (Double.PositiveInfinity, Double.PositiveInfinity);
			Assert.IsTrue (Double.IsPositiveInfinity (p.X), "X");
			Assert.IsTrue (Double.IsPositiveInfinity (p.Y), "Y");
			Assert.AreEqual ("Infinity,Infinity", p.ToString (), "ToString");
			Compare (p);
		}
	}
}
