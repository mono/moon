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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public partial class DoubleCollectionTest
	{
		[TestMethod]
		[MoonlightBug]
		public void IndexOfAndContainsTest ()
		{
			DoubleCollection c = new Rectangle ().StrokeDashArray;
			c.Add (1);
			Assert.AreEqual (1, c.Count, "#1");
			Assert.Throws<ArgumentException> (() => c.IndexOf (1), "#2");
			Assert.Throws<ArgumentException> (() => c.Contains (1), "#3");
			
			c.Insert (0, 2);
			Assert.AreEqual (2.0, (double) c [0], "#4");
			Assert.IsFalse (c.Remove (2), "#5");
			Assert.AreEqual (2, c.Count, "#7");
			Assert.AreEqual (2.0, (double) c [0], "#6");
		}
	}
}
