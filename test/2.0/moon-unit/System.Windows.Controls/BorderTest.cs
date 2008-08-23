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
	public class BorderTest
	{
		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			Border c = new Border ();
			Size s = new Size (10,10);

			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessPropertiesMeasureTest1 ()
		{
			Border c = new Border ();

			c.Padding = new Thickness (20);
			c.BorderThickness = new Thickness (5);
			c.BorderBrush = new SolidColorBrush (Colors.Black);
			c.Background = new SolidColorBrush (Colors.Black);

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessPropertiesMeasureTest2 ()
		{
			Border c = new Border ();

			c.Padding = new Thickness (20);
			c.BorderThickness = new Thickness (5);
			c.BorderBrush = new SolidColorBrush (Colors.Black);
			c.Background = new SolidColorBrush (Colors.Black);

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (50,50), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest2 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (50,50), c.DesiredSize);
		}

		[TestMethod]
		public void ChildPaddingMeasureTest1 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			c.Padding = new Thickness(10);

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize);
		}

		[TestMethod]
		public void ChildPaddingMeasureTest2 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			c.Padding = new Thickness(10);

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (70,70), c.DesiredSize);
		}

		[TestMethod]
		public void Child_Padding_ChildMargin_MeasureTest ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;
			r.Margin = new Thickness (5);

			c.Padding = new Thickness(10);

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (80,80), c.DesiredSize);
		}

		[TestMethod]
		public void Child_Padding_BorderMargin_MeasureTest ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			c.Margin = new Thickness (5);
			c.Padding = new Thickness(10);

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (80,80), c.DesiredSize);
		}
	}

}