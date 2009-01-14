//
// GridLength Unit Tests
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
	public class GridLengthTest {

		static void CheckAuto (GridLength gl)
		{
			Assert.AreEqual (1.0d, gl.Value, "Value");
			Assert.AreEqual (GridUnitType.Auto, gl.GridUnitType, "GridUnitType");
			Assert.IsFalse (gl.IsAbsolute, "IsAbsolute");
			Assert.IsTrue (gl.IsAuto, "IsAuto");
			Assert.IsFalse (gl.IsStar, "IsStar");
			Assert.AreEqual ("Auto", gl.ToString (), "ToString");

			Assert.IsFalse (gl.Equals (null), "Equals(null)");
			Assert.IsTrue (gl.Equals (gl), "Equals(self)");
		}

		[TestMethod]
		public void Auto ()
		{
			CheckAuto (GridLength.Auto);
		}

		[TestMethod]
		public void CtorDefault ()
		{
			GridLength gl = new GridLength ();
			CheckAuto (gl);
			Assert.IsTrue (gl.Equals (GridLength.Auto), "Equals(Auto)");
			Assert.IsTrue (gl.Equals (GridLength.Auto), "Auto.Equals");
			Assert.IsTrue (gl == GridLength.Auto, "==");
			Assert.IsFalse (gl != GridLength.Auto, "!=");
		}

		[TestMethod]
		public void CtorDouble_Validations ()
		{
			Assert.Throws<ArgumentException> (delegate {
				new GridLength  (-0.1);
			}, "Negative");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.MinValue);
			}, "MinValue");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.NegativeInfinity);
			}, "NegativeInfinity");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.NaN);
			}, "NaN");

			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.PositiveInfinity);
			}, "PositiveInfinity");
		}

		[TestMethod]
		public void CtorDouble_Zero ()
		{
			GridLength gl = new GridLength (0);
			Assert.AreEqual (0.0d, gl.Value, "Value");
			Assert.AreEqual (GridUnitType.Pixel, gl.GridUnitType, "GridUnitType");
			Assert.IsTrue (gl.IsAbsolute, "IsAbsolute");
			Assert.IsFalse (gl.IsAuto, "IsAuto");
			Assert.IsFalse (gl.IsStar, "IsStar");
			Assert.AreEqual ("0", gl.ToString (), "ToString");
		}

		[TestMethod]
		[SilverlightBug(PlatformID.MacOSX)]
		public void CtorDouble_Max ()
		{
			GridLength gl = new GridLength (Double.MaxValue);
			Assert.AreEqual (Double.MaxValue, gl.Value, "Value");
			Assert.AreEqual (GridUnitType.Pixel, gl.GridUnitType, "GridUnitType");
			Assert.IsTrue (gl.IsAbsolute, "IsAbsolute");
			Assert.IsFalse (gl.IsAuto, "IsAuto");
			Assert.IsFalse (gl.IsStar, "IsStar");
			// This fails on macos
			Assert.AreEqual (Double.MaxValue.ToString (), gl.ToString (), "ToString");
		}

		[TestMethod]
		public void CtorDoubleGridUnitType_Validations ()
		{
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (-0.1, GridUnitType.Pixel);
			}, "Negative");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.MinValue, GridUnitType.Pixel);
			}, "MinValue");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.NegativeInfinity, GridUnitType.Pixel);
			}, "NegativeInfinity");
			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.NaN, GridUnitType.Pixel);
			}, "NaN");

			Assert.Throws<ArgumentException> (delegate {
				new GridLength (Double.PositiveInfinity);
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				new GridLength (1.0, (GridUnitType)Int32.MinValue);
			}, "Bad GridUnitType");
		}

		[TestMethod]
		public void CtorDoubleGridUnitType ()
		{
			GridLength gl = new GridLength (42d, GridUnitType.Star);
			Assert.AreEqual (42d, gl.Value, "Value");
			Assert.AreEqual (GridUnitType.Star, gl.GridUnitType, "GridUnitType");
			Assert.IsFalse (gl.IsAbsolute, "IsAbsolute");
			Assert.IsFalse (gl.IsAuto, "IsAuto");
			Assert.IsTrue (gl.IsStar, "IsStar");
			Assert.AreEqual ("42*", gl.ToString (), "ToString");
		}

		[TestMethod]
		public void NonOneAuto ()
		{
			GridLength gl = new GridLength (0, GridUnitType.Auto);
			CheckAuto (gl);
			// double value is ignored for Auto (it's always 1.0)
		}
	}
}
