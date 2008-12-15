//
// Thickness Unit Tests
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
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class ThicknessTest
	{
		[TestMethod]
		public void ToStringTest ()
		{
			Thickness t = new Thickness (10);
			Assert.AreEqual ("10,10,10,10", t.ToString());

			t = new Thickness (1, 2, 3, 4);
			Assert.AreEqual ("1,2,3,4", t.ToString());

			t = new Thickness (-1);
			Assert.AreEqual ("-1,-1,-1,-1", t.ToString());
		}

		[TestMethod]
		public void SpecialCases ()
		{
			Thickness t = new Thickness (Double.MinValue);
			Assert.AreEqual ("-1.79769313486232E+308,-1.79769313486232E+308,-1.79769313486232E+308,-1.79769313486232E+308", t.ToString (), "MinValue");

			t = new Thickness (Double.MaxValue);
			Assert.AreEqual ("1.79769313486232E+308,1.79769313486232E+308,1.79769313486232E+308,1.79769313486232E+308", t.ToString (), "MaxValue");

			t = new Thickness(Double.NegativeInfinity);
			Assert.AreEqual ("-Infinity,-Infinity,-Infinity,-Infinity", t.ToString (), "-Infinity");

			t = new Thickness(Double.PositiveInfinity);
			Assert.AreEqual ("Infinity,Infinity,Infinity,Infinity", t.ToString (), "Infinity");

			t = new Thickness (Double.NaN);
			Assert.AreEqual ("Auto,Auto,Auto,Auto", t.ToString ());

			t = new Thickness (Double.MinValue, Double.MaxValue, Double.NegativeInfinity, Double.PositiveInfinity);
			Assert.AreEqual ("-1.79769313486232E+308,1.79769313486232E+308,-Infinity,Infinity", t.ToString (), "Mix");
			Assert.IsTrue (t.Equals (t), "Mix.Equals");

			t.Bottom = Double.NaN;
			Assert.IsFalse (t.Equals (t), "MixNaN.Equals");
		}

		[TestMethod]
		public void NaN ()
		{
			Thickness t1 = new Thickness (Double.NaN);
			Assert.IsFalse (t1.Equals (t1), "NaN-1");

			Assert.IsTrue (Double.IsNaN (t1.Left), "Left");
			Assert.IsTrue (Double.IsNaN (t1.Top), "Top");
			Assert.IsTrue (Double.IsNaN (t1.Right), "Right");
			Assert.IsTrue (Double.IsNaN (t1.Bottom), "Bottom");

			Thickness t2 = new Thickness (Double.NaN);
			Assert.IsFalse (t1.Equals (t2), "NaN-2");
			Assert.IsFalse (t1 == t2, "NaN-3");
			Assert.IsTrue (t1 != t2, "NaN-4");
		}
	}
}
