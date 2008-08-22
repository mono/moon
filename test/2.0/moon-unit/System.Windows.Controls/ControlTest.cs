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
	public class ControlTest
	{
		class ControlPoker : Control {
		}

		[TestMethod]
		public void DefaultRenderSizeTest ()
		{
 			ControlPoker p = new ControlPoker ();
 			Assert.AreEqual (new Size (0,0), p.RenderSize);
		}

		[TestMethod]
		public void DefaultDesiredSizeTest ()
		{
 			ControlPoker p = new ControlPoker ();
 			Assert.AreEqual (new Size (0,0), p.DesiredSize);
		}

		[TestMethod]
		public void DefaultMeasureTest ()
		{
 			ControlPoker p = new ControlPoker ();

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (new Size (0,0), p.DesiredSize);
			Assert.AreEqual (new Size (0,0), p.RenderSize);
		}

		[TestMethod]
		public void MinWidthMeasureTest1 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinWidth = 50;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Width);
		}

		[TestMethod]
		public void MinWidthMeasureTest2 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinWidth = 5;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Width);
		}

		[TestMethod]
		public void MinHeightMeasureTest1 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinHeight = 50;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Height);
		}

		[TestMethod]
		public void MinHeightMeasureTest2 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinHeight = 5;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Height);
		}

		[TestMethod]
		public void GetTemplateChildTest ()
		{
		}
	}
}
