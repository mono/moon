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

namespace MoonTest.System.Windows
{
	[TestClass]
	public class ThicknessTest
	{
		[TestMethod]
		public void ToString ()
		{
			Thickness t = new Thickness (10);
			Assert.AreEqual ("10,10,10,10", t.ToString());

			t = new Thickness (1, 2, 3, 4);
			Assert.AreEqual ("1,2,3,4", t.ToString());

			t = new Thickness (-1);
			Assert.AreEqual ("-1,-1,-1,-1", t.ToString());
		}
	}
}