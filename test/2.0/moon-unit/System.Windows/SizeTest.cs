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
		}
	}
}