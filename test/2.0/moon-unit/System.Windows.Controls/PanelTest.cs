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


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class PanelTest
	{
		class PanelPoker : Panel
		{
		}

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			PanelPoker c = new PanelPoker ();
			Size s = new Size (10,10);

			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			PanelPoker c = new PanelPoker ();
			Rectangle r = new Rectangle();

			c.Children.Add (r);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest2 ()
		{
			PanelPoker c = new PanelPoker ();
			Rectangle r = new Rectangle();

			c.Children.Add (r);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}
	}

}