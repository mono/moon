using System;
using System.Windows;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class RectTest
	{
		[TestMethod]
		public void TestToString ()
		{
			Rect rect = new Rect (10, 20, 30, 40);
			Assert.AreEqual ("10,20,30,40", rect.ToString ());
		}

		[TestMethod]
		// in need of a better test case
		public void TestIFormatProvider ()
		{
			Rect rect = new Rect (10, 20, 30, 40);
			Assert.AreEqual ("10,20,30,40", String.Format ("{0}",rect));
		}

		[TestMethod]
		public void EmptyToString ()
		{
			Assert.AreEqual ("Empty", Rect.Empty.ToString ());
		}
		
		[TestMethod]
		public void InvalidValuesTest ()
		{
			Rect e = new Rect (0,0,0,0);

			Assert.Throws<ArgumentException>(delegate{ 
					e.Width = e.Height = -20;
				}, "negative values");

			Assert.Throws<ArgumentException>(delegate{ 
					e.Width = e.Height = Double.NegativeInfinity;
				}, "negative infinity");

			Assert.Throws<ArgumentException>(delegate{
					Rect a = new Rect (Double.PositiveInfinity,Double.PositiveInfinity,
							   Double.NegativeInfinity, Double.NegativeInfinity);
				}, "negative ctor");
		}

		[TestMethod]
		public void EmptyTest ()
		{
			Assert.AreEqual (Double.PositiveInfinity, Rect.Empty.X, "x");
			Assert.AreEqual (Double.PositiveInfinity, Rect.Empty.Y, "y");
			Assert.AreEqual (Double.NegativeInfinity, Rect.Empty.Width, "width");
			Assert.AreEqual (Double.NegativeInfinity, Rect.Empty.Height, "height");
			Assert.IsFalse (new Rect (0,0,0,0).IsEmpty, "zeros all the way down");
			Assert.IsTrue (Rect.Empty.IsEmpty, "Empty better be Empty");
			Rect e = Rect.Empty;
			e.X = 0;
			Assert.IsTrue (e.IsEmpty, "0 x");
			e.Y = 0;
			Assert.IsTrue (e.IsEmpty, "0 y");
			e.Width = 0;
			Assert.IsFalse (e.IsEmpty, "0 width");
			Assert.AreEqual (Double.NegativeInfinity, e.Height);
			Assert.AreEqual (0.0, e.Width);

			e.Height = 0;
			Assert.IsFalse (e.IsEmpty, "all zero again");

			e.Height = e.Width = Double.NaN;
			Assert.IsFalse (e.IsEmpty, "w/h NaN");
			
			e.Width = e.Height = Double.PositiveInfinity;
			Assert.IsFalse (e.IsEmpty, "w/h PositiveInfinity");

			e = Rect.Empty;
			e.Width = 0;
			Assert.IsFalse (e.IsEmpty, "(\u221e,\u221e,0,-\u221e).IsEmpty");

			e.X = e.Y = e.Width = e.Height = Double.NaN;
			Assert.IsFalse (e.IsEmpty, "(NaN,NaN,NaN,NaN).IsEmpty");
		}

		[TestMethod]
		public void EmptyIntersectTest ()
		{
			var r1 = new Rect (0,0,10,10);
			var r2 = new Rect (20,20,10,10);
			
			r1.Intersect (r2);
			
			Assert.IsTrue (r1.IsEmpty);

		}

		[TestMethod]
		public void TouchingIntersectTest ()
		{
			var r1 = new Rect (0,0,10,10);
			var r2 = new Rect (10,10,10,10);
			
			r1.Intersect (r2);
			
			Assert.AreEqual (new Rect (10,10,0,0), r1, "zero w.h");
		}

		[TestMethod]
		public void EdgeIntersectTest ()
		{
			var r1 = new Rect (0,0,10,10);
			var r2 = new Rect (0,10,10,10);
			
			r1.Intersect (r2);
			
			Assert.AreEqual (new Rect (0,10,10,0), r1);
		}

		[TestMethod]
		public void EmptyIntersectEmptyTest ()
		{
			var r1 = new Rect (0,0,10,10);
			
			r1.Intersect (Rect.Empty);
			
			Assert.IsTrue (r1.IsEmpty);
		}

		[TestMethod]
		public void EqualsNaN ()
		{
			Rect r = new Rect (Double.NaN, Double.NaN, Double.NaN, Double.NaN);
			Assert.IsFalse (r.Equals (r), "Equals(Rect)");
			Assert.IsFalse (r.Equals ((object)r), "Equals(object)");
		}
	}
}
