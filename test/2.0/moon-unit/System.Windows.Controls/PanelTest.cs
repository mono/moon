using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class PanelTest
	{
		class LayoutPoker : Panel
		{
			public Size MeasureResult = new Size (0,0);
			public Size MeasureArg = new Size (0,0);
			public Size ArrangeResult = new Size (0,0);
			public Size ArrangeArg = new Size (0,0);
			public Size BaseArrangeResult = new Size (0,0);
			public Size BaseMeasureResult = new Size (0,0);
			public event MeasureOverrideHandler Measured;
			public event ArrangeOverrideHandler Arranged;
			public delegate void ArrangeOverrideHandler (Size real);
			public delegate void MeasureOverrideHandler (Size real);

			protected override Size MeasureOverride (Size availableSize)
			{
				MeasureArg = availableSize;
				Tester.WriteLine (string.Format ("Panel available size is {0}", availableSize));
				BaseMeasureResult = base.MeasureOverride (availableSize);

				if (Measured != null)
					Measured (BaseMeasureResult);

				return MeasureResult;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				ArrangeArg = finalSize;
				Tester.WriteLine (string.Format ("Panel final size is {0}", finalSize));
				BaseArrangeResult = base.ArrangeOverride (finalSize);
				
				if (Arranged != null)
					Arranged (BaseArrangeResult);

				return ArrangeResult;

			}

		}

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);

			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
			Assert.AreEqual (new Size (10,10), c.MeasureArg, "measure args");
		}

		[TestMethod]
		public void ChildlessMeasureTest2 ()
		{
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			Border b = new Border ();
			
			b.Child = c;
			
			c.MeasureResult = new Size (5, 5);
			c.Margin = new Thickness (1);			
			b.Measure (s);

			Assert.AreEqual (new Size (7, 7), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMeasureTest3 ()
		{
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			Border b = new Border ();
			
			b.Child = c;
			
			c.MaxWidth = 3;
			c.MeasureResult = new Size (5, 5);
			c.Margin = new Thickness (1);			
			b.Measure (s);

			Assert.AreEqual (new Size (5, 7), c.DesiredSize, "DesiredSize");
			Assert.AreEqual (new Size (3, 8), c.MeasureArg, "MeasureArgs");
		}

		[TestMethod]
		public void ChildlessMeasureTest4 ()
		{
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			Border b = new Border ();
			
			b.Child = c;
			
			c.Width = 9;
			c.Height = 9;
			c.MeasureResult = new Size (5, 5);
			c.Margin = new Thickness (1);			
			c.Measure (s);

			Assert.AreEqual (new Size (c.Width, c.Height), c.MeasureArg, "MeasureArg");
			Assert.AreEqual (new Size (10, 10), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			LayoutPoker c = new LayoutPoker ();
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
			LayoutPoker c = new LayoutPoker ();
			Rectangle r = new Rectangle();

			c.Children.Add (r);

			r.Width = 50;
			r.Height = 50;
			
			c.Measured += (Size real) => { c.MeasureResult = real; };
			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.BaseMeasureResult);
		}

		[TestMethod]
		public void ChildMeasureTest3 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Rectangle r = new Rectangle();
			b.Child = c;
			c.Children.Add (r);
			c.Background = new SolidColorBrush (Colors.Red);

			r.Width = 50;
			r.Height = 50;
			
			bool called = false;
			c.Measured += (Size real) => { 
				c.MeasureResult = real; 
				called = true;
			};
			
			b.Measure (new Size (100, 100));
			Assert.IsTrue (called, "measure called");

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
			Assert.AreEqual (new Size (0,0), r.DesiredSize);
			Assert.AreEqual (new Size (0,0), b.DesiredSize);
		}
		
		[TestMethod]
		public void ChildlessArrangeTest1 ()
		{
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
			Assert.AreEqual (new Size (10,10), c.MeasureArg, "measure args");
			
			c.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "render size");
			Assert.AreEqual (new Size (10,10), c.ArrangeArg, "measure args");
		}

		[TestMethod]
		public void ChildlessArrangeTest2 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			b.Child = c;
			b.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (10,10), c.MeasureArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (0,b.ActualWidth);
			Assert.AreEqual (0,b.ActualHeight);
			
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (10,10), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
		}

		[TestMethod]
		public void ChildlessArrangeTest3 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			b.Child = c;
			b.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (10,10), c.MeasureArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (0,b.ActualWidth);
			Assert.AreEqual (0,b.ActualHeight);
			Assert.AreEqual (0,c.ActualWidth);
			Assert.AreEqual (0,c.ActualHeight);
			
			c.ArrangeResult = new Size (10,10);
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (10,10), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (10,10), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (10,c.ActualWidth);
			Assert.AreEqual (10,c.ActualHeight);
		}

		[TestMethod]
		public void ChildlessArrangeTest4 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			b.Child = c;
			b.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (10,10), c.MeasureArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (0,b.ActualWidth);
			Assert.AreEqual (0,b.ActualHeight);
			Assert.AreEqual (0,c.ActualWidth);
			Assert.AreEqual (0,c.ActualHeight);
			
			c.ArrangeResult = new Size (10,10);
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (10,10), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (10,10), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (10,c.ActualWidth);
			Assert.AreEqual (10,c.ActualHeight);
		}

		[TestMethod]
		public void ChildlessArrangeTest5 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			b.Padding = new Thickness (1,1,0,0);
			b.Child = c;
			b.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (9,9), c.MeasureArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (0,b.ActualWidth);
			Assert.AreEqual (0,b.ActualHeight);
			Assert.AreEqual (0,c.ActualWidth);
			Assert.AreEqual (0,c.ActualHeight);
			
			c.ArrangeResult = new Size (9,9);
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (9,9), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (9,c.ActualWidth);
			Assert.AreEqual (9,c.ActualHeight);
		}

		[TestMethod]
		public void DefaultTest()
		{
			Assert.IsNull(new LayoutPoker().Background);
		}

		[TestMethod]
		public void InvalidateMeasureTest ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			b.Child = c;

			c.MeasureResult = s;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize");

			c.MeasureResult = new Size (20,20);
			c.MeasureArg = new Size (99,99);

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			c.InvalidateMeasure ();
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (20,20), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (20,20), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (Double.PositiveInfinity,Double.PositiveInfinity), c.MeasureArg, "c measurearg");
		}

		[TestMethod]
		public void InvalidateMeasureTest2 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			b.Child = c;

			c.MeasureResult = s;

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize");

			c.MeasureResult = new Size (20,20);
			c.MeasureArg = new Size (99,99);

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			c.InvalidateMeasure ();
			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (20,20), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (Double.PositiveInfinity,Double.PositiveInfinity), c.MeasureArg, "c measurearg");
		}

		[TestMethod]
		public void InvalidateMeasureTest3 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			b.Child = c;

			c.MeasureResult = s;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize");

			c.MeasureResult = new Size (20,20);
			c.MeasureArg = new Size (99,99);

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			b.InvalidateMeasure ();
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			c.InvalidateMeasure ();
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (20,20), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (20,20), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (Double.PositiveInfinity,Double.PositiveInfinity), c.MeasureArg, "c measurearg");
		}

		[TestMethod]
		public void InvalidateMeasureTest4 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			b.Child = c;

			c.MeasureResult = s;

			b.Measure (new Size (2, 2));
			Assert.AreEqual (new Size (2,2), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (2,2), c.DesiredSize, "c desiredsize");

			c.MeasureResult = new Size (20,20);
			c.MeasureArg = new Size (99,99);

			b.Measure (new Size (3, 3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (3,3), c.MeasureArg, "c measurearg");

			c.MeasureArg = new Size (99,99);
			b.Measure (new Size (3, 3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			b.InvalidateMeasure ();
			c.MeasureArg = new Size (99,99);
			b.Measure (new Size (3, 3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg");

			c.InvalidateMeasure ();
			b.Measure (new Size (3,3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (3,3), c.MeasureArg, "c measurearg");
		}

		[TestMethod]
		public void InvalidateMeasureTest5 ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			b.Child = c;

			c.MeasureResult = s;

			b.Measure (new Size (100,100));
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desiredsize");

			c.MeasureResult = new Size (20,20);
			c.MeasureArg = new Size (99,99);

			b.Measure (new Size (110,110));
			Assert.AreEqual (new Size (20,20), b.DesiredSize, "b desiredsize1");
			Assert.AreEqual (new Size (20,20), c.DesiredSize, "c desiredsize1");
			Assert.AreEqual (new Size (110,110), c.MeasureArg, "c measurearg");
		}

		[TestMethod]
		public void InvalidateArrangeTest ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			Size s = new Size (10,10);
			
			b.Padding = new Thickness (1,1,0,0);
			b.Child = c;
			b.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (9,9), c.MeasureArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (0,b.ActualWidth);
			Assert.AreEqual (0,b.ActualHeight);
			Assert.AreEqual (0,c.ActualWidth);
			Assert.AreEqual (0,c.ActualHeight);
			
			c.ArrangeResult = new Size (20,20);
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (20,20), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (20,c.ActualWidth);
			Assert.AreEqual (20,c.ActualHeight);

			c.ArrangeResult = new Size (9,9);
			b.Arrange (new Rect (0,0,10,10));

			// Does not invalidate child
			b.InvalidateArrange ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (20,20), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (20,c.ActualWidth);
			Assert.AreEqual (20,c.ActualHeight);

			c.InvalidateArrange ();
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (9,9), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (10,b.ActualWidth);
			Assert.AreEqual (10,b.ActualHeight);
			Assert.AreEqual (9,c.ActualWidth);
			Assert.AreEqual (9,c.ActualHeight);
		}

		[TestMethod]
		public void AlignmentTest ()
		{
			Border b = new Border ();
			Border b2 = new Border ();
			LayoutPoker poker = new LayoutPoker ();
			LayoutPoker pchild = new LayoutPoker ();

			b.Child = b2;
			b2.Child = poker;
			b.Width = 50;
			
			b2.HorizontalAlignment = HorizontalAlignment.Right;
			b2.VerticalAlignment = VerticalAlignment.Bottom;

			poker.MeasureResult = new Size (20,20);
			b.Measure (new Size (100,100));

			Assert.AreEqual (new Size (50,100), poker.MeasureArg, "poker m arg");
			Assert.AreEqual (new Size (20,20), poker.DesiredSize, "poker m result");
			Assert.AreEqual (new Size (0,0), poker.BaseMeasureResult, "poker base result");
			
			Assert.AreEqual (new Size (50,20), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (20,20), b2.DesiredSize, "b2 desiredsize");
			
			poker.ArrangeResult = new Size (20,20);
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Size (20,20),poker.ArrangeArg, "poker aa");
			
			Assert.AreEqual (new Rect (0,0,20,20).ToString (), LayoutInformation.GetLayoutSlot (poker).ToString (), "poker slot");
			Assert.AreEqual (new Rect (0,0,50,20).ToString (), LayoutInformation.GetLayoutSlot (b2).ToString (), "b2 slot");
			Assert.AreEqual (new Rect (0,0,50,20).ToString (), LayoutInformation.GetLayoutSlot (b).ToString (), "b slot");
		}
	}
}
