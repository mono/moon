using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows
{
	[TestClass]
	public class SizeTest
	{
		[TestMethod]
		public void EmptyTest ()
		{
			Assert.AreEqual (Double.NegativeInfinity, Size.Empty.Width);
			Assert.AreEqual (Double.NegativeInfinity, Size.Empty.Height);

			Assert.AreEqual (false, (new Size(0,0)).IsEmpty);

			Assert.Throws (delegate { new Size (-1, 5); }, typeof (ArgumentException));
			Assert.Throws (delegate { new Size (Double.MinValue, Double.MinValue); }, typeof (ArgumentException));
			Assert.Throws (delegate { new Size (Double.NegativeInfinity, Double.NegativeInfinity); }, typeof (ArgumentException));

			Size s = new Size (0,0);
			Assert.Throws (delegate { s.Width = Double.NegativeInfinity; }, typeof (ArgumentException));
			
			s = Size.Empty;
			s.Width = 10;
			Assert.AreEqual (false, s.IsEmpty);
		}

		[TestMethod]
		public void NaNTest ()
		{
			Size test = new Size (Double.NaN, Double.NaN);
			Assert.AreEqual (true, Double.IsNaN (test.Width));
			Assert.AreEqual (true, Double.IsNaN (test.Height));
		}

		[TestMethod]
		[MoonlightBug]
		public void NaNValuesTest ()
		{
			Size nan = new Size (Double.NaN, Double.NaN);
			Assert.AreEqual (nan, new Size (Double.NaN, Double.NaN));
		}

		[TestMethod]
		public void ToStringTest ()
		{
			Assert.AreEqual (Concat (5, 5), new Size (5, 5).ToString ());
			Assert.AreEqual (Concat (5.234124, 5234235), new Size (5.234124, 5234235).ToString ());
		}

		static string Concat (double a, double b)
		{
			return a.ToString () + "," + b.ToString ();
		}

		[TestMethod]
		public void SpecialToString ()
		{
			Assert.AreEqual (Concat (Double.NaN, Double.NaN), new Size (Double.NaN, Double.NaN).ToString ());
			Assert.AreEqual ("Empty", Size.Empty.ToString ());

			Size s = Size.Empty;
			s.Width = Double.NaN;
			Assert.AreEqual (Concat (Double.NaN, Double.NegativeInfinity), s.ToString ());
		}

		[TestMethod]
		public void QuasiEquality ()
		{
			Size expected = new Size (25, 25);
			Size actual = new Size (25.000000000000001, 25.000000000000001);
			Assert.IsTrue (expected.Equals (actual), "Equals(Size)");
			Assert.IsTrue (actual.Equals ((object)expected), "Equals(object)");

			actual = new Size (25.00000000000001, 25.00000000000001);
			Assert.IsFalse (expected.Equals (actual), "not-Equals(Size)");
			Assert.IsFalse (actual.Equals ((object) expected), "not-Equals(object)");
		}
	}
}
