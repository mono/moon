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
		class LayoutPoker : Panel
		{
			public Size MeasureResult = new Size (0,0);
			public Size MeasureArg = new Size (0,0);
			public Size ArrangeResult = new Size (0,0);
			public Size ArrangeArg = new Size (0,0);

			protected override Size MeasureOverride (Size availableSize)
			{
				MeasureArg = availableSize;
				Tester.WriteLine (string.Format ("Panel available size is {0}", availableSize));
				return MeasureResult;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				ArrangeArg = finalSize;
				Tester.WriteLine (string.Format ("Panel final size is {0}", finalSize));
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
			
			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
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
		[KnownFailure]
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
	}
}
