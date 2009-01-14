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
		[MoonlightBug]
		public void TestIFormatProvider ()
		{
			Rect rect = new Rect (10, 20, 30, 40);
			Assert.AreEqual ("10,20,30,40", String.Format ("{0}",rect));
		}

		public void EmptyToString ()
		{
			Assert.AreEqual ("0,0,0,0", String.Empty.ToString ());
		}
	}
}
