using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using Mono.Moonlight.UnitTesting;
using System.Windows.Shapes;

namespace MoonTest.System.Windows.Shapes
{
	[TestClass]
	public class PathTest
	{
		[TestMethod]
		public void MeasureRectangleTest1 ()
		{
			Path p = new Path ();
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;

			p.Measure (new Size (50, 50));

			Assert.AreEqual (new Size (0, 0), p.DesiredSize);
		}

		[TestMethod]
		public void MeasureRectangleBorderTest1 ()
		{
			Border b = new Border ();
			Path p = new Path ();
			p.Fill = new SolidColorBrush (Colors.Black);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;
			b.Child = p;

			b.Measure (new Size (50, 50));

			Assert.AreEqual (new Size (0, 0), b.DesiredSize);
			Assert.AreEqual (new Size (0, 0), p.DesiredSize);
		}

		[TestMethod]
		public void MeasureRectangleBorderTest2 ()
		{
			Border b = new Border ();
			Path p = new Path ();
			p.Width = 20;
			p.Height = 20;
			p.Fill = new SolidColorBrush (Colors.Black);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;
			b.Child = p;

			b.Measure (new Size (50, 50));

			Assert.AreEqual (new Size (20, 20), b.DesiredSize);
			Assert.AreEqual (new Size (20, 20), p.DesiredSize);
		}
	}
}
