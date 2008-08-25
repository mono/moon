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
		public void ChildlessPropertiesMeasureTest3 ()
		{
			Border c = new Border ();

			c.Padding = new Thickness (20);
			c.BorderThickness = new Thickness (5);
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

		[TestMethod]
		public void ArrangeTest_RenderSize_ChildSmallerThanFinalRect ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			Assert.AreEqual (new Size (0, 0), r.RenderSize);

			c.Child = r;

			Assert.AreEqual (new Size (0, 0), r.RenderSize);

			r.Width = 50;
			r.Height = 50;

			Assert.AreEqual (new Size (0, 0), r.RenderSize);

			// we intentionally give border less room to
			// work with than the configured width/height
			// of the child
			c.Measure (new Size (25, 25));

			Assert.AreEqual (new Size (0, 0), r.RenderSize);

			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (new Size (25, 25), r.DesiredSize);
			Assert.AreEqual (new Size (50, 50), r.RenderSize);
		}

		[TestMethod]
		public void ArrangeTest_ChildSmallerThanFinalRect ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			r.Width = 50;
			r.Height = 50;

			// we intentionally give border less room to
			// work with than the configured width/height
			// of the child
			c.Measure (new Size (25, 25));
			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (new Size (25, 25), r.DesiredSize);
		}

		[TestMethod]
		public void ArrangeTest_ChildSmallerThanFinalRect_WithBorderMargin ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			c.Margin = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			// we intentionally give border less room to
			// work with than the configured width/height
			// of the child
			c.Measure (new Size (25, 25));
			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (new Size (15, 15), r.DesiredSize);
		}

		[TestMethod]
		public void ArrangeTest_ChildSmallerThanFinalRect_WithBorderBrushAndThickness ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			c.BorderBrush = new SolidColorBrush (Colors.Black);
			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			// we intentionally give border less room to
			// work with than the configured width/height
			// of the child
			c.Measure (new Size (25, 25));
			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (new Size (15, 15), r.DesiredSize);
		}

		[TestMethod]
		public void ArrangeTest_ChildSmallerThanFinalRect_WithBorderThickness ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			// we intentionally give border less room to
			// work with than the configured width/height
			// of the child
			c.Measure (new Size (25, 25));
			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (new Size (15, 15), r.DesiredSize);
		}
	}
}