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
			public event EventHandler Measuring;
			public event MeasureOverrideHandler Measured;
			public event EventHandler Arranging;
			public event ArrangeOverrideHandler Arranged;
			public delegate void ArrangeOverrideHandler (Size real);
			public delegate void MeasureOverrideHandler (Size real);

			protected override Size MeasureOverride (Size availableSize)
			{
				MeasureArg = availableSize;
				Tester.WriteLine (string.Format ("Panel available size is {0}", availableSize));

				if (Measuring != null)
					Measuring (this, EventArgs.Empty);

				BaseMeasureResult = base.MeasureOverride (availableSize);

				if (Measured != null)
					Measured (BaseMeasureResult);

				return MeasureResult;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				ArrangeArg = finalSize;
				Tester.WriteLine (string.Format ("Panel final size is {0}", finalSize));

				if (Arranging != null)
					Arranging (this, EventArgs.Empty);

				BaseArrangeResult = base.ArrangeOverride (finalSize);
				
				if (Arranged != null)
					Arranged (BaseArrangeResult);

				return ArrangeResult;

			}

		}

		[TestMethod]
		public void MaxOnElement ()
		{
			LayoutPoker c = new LayoutPoker { MaxWidth = 50, MaxHeight = 50 };

			c.Measure (new Size (1000, 1000));

			Assert.AreEqual (new Size (50, 50), c.MeasureArg, "c.MeasureArg");

			c.Arrange (new Rect (0, 0, 1000, 1000));

			Assert.AreEqual (new Size (50, 50), c.ArrangeArg, "c.ArrangeArg");

			Assert.AreEqual (new Size (0,0), c.RenderSize, "c.RenderSize");
			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c.DesiredSize");
			Assert.IsNull (LayoutInformation.GetLayoutClip (c), "c.LayoutClip == null");
		}

		[TestMethod]
		public void MinOnElement ()
		{
			LayoutPoker c = new LayoutPoker { MinWidth = 500, MinHeight = 500 };

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (500, 500), c.MeasureArg, "c.MeasureArg");

			c.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (new Size (500, 500), c.ArrangeArg, "c.ArrangeArg");

			Assert.AreEqual (new Size (0, 0), c.RenderSize, "c.RenderSize");
			Assert.AreEqual (new Size (100, 100), c.DesiredSize, "c.DesiredSize");
			Assert.IsNull (LayoutInformation.GetLayoutClip (c), "c.LayoutClip == null");
		}

		[TestMethod]
		public void MaxHeightOnParentTest ()
		{
			StackPanel sp = new StackPanel { MaxHeight = 35 };
			LayoutPoker c = new LayoutPoker { Width = 50, Height = 50 };

			sp.Children.Add (c);

			sp.Measure (new Size (1000, 1000));

			Assert.AreEqual (new Size (50,50), c.MeasureArg, "c.Measure");

			Assert.AreEqual (new Size (50,50), c.DesiredSize);

			sp.Arrange (new Rect (0, 0, 1000, 1000));

			Assert.AreEqual (new Size (50,50), c.ArrangeArg, "c.Arrange");

			// now check desired/render sizes

			// the child is oblivious to the parent's maxheight
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c.RenderedSize");
			Assert.AreEqual (new Size (50,50), c.DesiredSize, "c.DesiredSize");
			Assert.IsNull (LayoutInformation.GetLayoutClip (c), "c.LayoutClip == null");

			// the parent's maxheight clips
			Assert.AreEqual (new Size (1000,50), sp.RenderSize, "sp.RenderSize");
			Assert.AreEqual (new Size (50,35), sp.DesiredSize, "sp.DesiredSize");
			Assert.IsNotNull (LayoutInformation.GetLayoutClip (sp), "sp.LayoutClip != null");
		}

		[TestMethod]
		public void MinWidthOnParentTest ()
		{
			StackPanel sp = new StackPanel { MinWidth = 200 };
			LayoutPoker c = new LayoutPoker { Height = 25 };

			sp.Children.Add (c);

			sp.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (200,25), c.MeasureArg, "c.Measure");

			Assert.AreEqual (new Size (0,25), c.DesiredSize);

			sp.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (new Size (200,25), c.ArrangeArg, "c.Arrange");

			// now check desired/render sizes

			// the child is oblivious to the parent's maxheight
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c.RenderedSize");
			Assert.AreEqual (new Size (0,25), c.DesiredSize, "c.DesiredSize");
			Assert.IsNull (LayoutInformation.GetLayoutClip (c), "c.LayoutClip == null");

			// the parent's maxheight clips
			Assert.AreEqual (new Size (200,100), sp.RenderSize, "sp.RenderSize");
			Assert.AreEqual (new Size (100,25), sp.DesiredSize, "sp.DesiredSize");
			Assert.IsNotNull (LayoutInformation.GetLayoutClip (sp), "sp.LayoutClip != null");
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
		public void ComputeWidthCallsLayout ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			b.Child = c;

			int called = 0;
			c.Measured += (Size real) => { 
				c.MeasureResult = real; 
				called++;
			};

			c.Width = 50;
			c.Height = 50;
			
			Assert.AreEqual (0, called);

			b.Measure (new Size (100, 100));
			
			Assert.AreEqual (1, called);
		}

		[TestMethod]
		public void ComputeWidthCallsLayoutParent ()
		{
			Border b = new Border ();
			LayoutPoker c = new LayoutPoker ();
			b.Child = c;

			int called = 0;
			c.Measured += (Size real) => { 
				c.MeasureResult = real; 
				called++;
			};

			b.Width = 50;
			b.Height = 50;
			
			Assert.AreEqual (0, called);

			b.Measure (new Size (100, 100));
			
			Assert.AreEqual (1, called);
		}

		[TestMethod]
		[MoonlightBug]
		public void CanvasCallsLayoutTest ()
		{
			var parent = new Canvas ();
			LayoutPoker c = new LayoutPoker ();
			parent.Children.Add (c);

			c.Width = 50;
			c.Height = 50;
			
			int measure_called = 0;
			int arrange_called = 0;
			c.Measured += (Size real) => { 
				c.MeasureResult = real; 
				measure_called++;
			};
			c.Arranged += (Size real) => {
				c.ArrangeResult = real;
				arrange_called++;
			};

			parent.Measure (new Size (100, 100));
			Assert.AreEqual (0, arrange_called, "arrange called 0");
			Assert.AreEqual (1, measure_called, "measure called 0");

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
			Assert.AreEqual (new Size (0,0), parent.DesiredSize);

			parent.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (1, arrange_called, "arrange called 1");
			Assert.AreEqual (1, measure_called, "measure called 1");

			c.InvalidateMeasure ();
			c.InvalidateArrange ();
			parent.InvalidateMeasure ();
			parent.InvalidateArrange ();
			parent.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (2, arrange_called, "arrange called 2");
			Assert.AreEqual (2, measure_called, "measure called 2");
		}

		[TestMethod]
		[MoonlightBug]
		public void BorderCallsLayoutTest ()
		{
			var parent = new Border ();
			LayoutPoker c = new LayoutPoker ();
			parent.Child = c;

			c.Width = 50;
			c.Height = 50;
			
			int measure_called = 0;
			int arrange_called = 0;
			c.Measured += (Size real) => { 
				c.MeasureResult = real; 
				measure_called++;
			};
			c.Arranged += (Size real) => {
				c.ArrangeResult = real;
				arrange_called++;
			};

			parent.Measure (new Size (100, 100));
			Assert.AreEqual (0, arrange_called, "arrange called 0");
			Assert.AreEqual (1, measure_called, "measure called 0");

			Assert.AreEqual (new Size (50,50), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (50,50), parent.DesiredSize, "parent desired");

			parent.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (1, arrange_called, "arrange called 1");
			Assert.AreEqual (1, measure_called, "measure called 1");

			c.InvalidateMeasure ();
			c.InvalidateArrange ();
			parent.InvalidateArrange ();
			parent.InvalidateMeasure ();
			parent.Arrange (new Rect (0, 0, 100, 100));

			Assert.AreEqual (2, arrange_called, "arrange called 2");
			Assert.AreEqual (2, measure_called, "measure called 2");
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
		[MoonlightBug ("Right now we are clearing the invalidation when we shouldn't")]
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
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg2");

			c.InvalidateMeasure ();
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (20,20), b.DesiredSize, "b desiredsize2");
			Assert.AreEqual (new Size (20,20), c.DesiredSize, "c desiredsize2");
			Assert.AreEqual (new Size (Double.PositiveInfinity,Double.PositiveInfinity), c.MeasureArg, "c measurearg2");
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
		[MoonlightBug ("Right now we are clearing the invalidation when we shouldn't")]
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
		[MoonlightBug ("Right now we are clearing the invalidation when we shouldn't")]
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
			Assert.AreEqual (new Size (3,3), c.MeasureArg, "c measurearg1");

			c.MeasureArg = new Size (99,99);
			b.Measure (new Size (3, 3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize2");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize2");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg2");

			b.InvalidateMeasure ();
			c.MeasureArg = new Size (99,99);
			b.Measure (new Size (3, 3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize3");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize3");
			Assert.AreEqual (new Size (99,99), c.MeasureArg, "c measurearg3");

			c.InvalidateMeasure ();
			b.Measure (new Size (3,3));
			Assert.AreEqual (new Size (3,3), b.DesiredSize, "b desiredsize4");
			Assert.AreEqual (new Size (3,3), c.DesiredSize, "c desiredsize4");
			Assert.AreEqual (new Size (3,3), c.MeasureArg, "c measurearg4");
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
			Assert.AreEqual (new Size (10,10), new Size (b.ActualWidth, b.ActualHeight),"b actual");
			Assert.AreEqual (new Size (20,20), new Size (c.ActualWidth, c.ActualHeight), "c actual");

			c.ArrangeResult = new Size (9,9);
			b.Arrange (new Rect (0,0,10,10));

			// Does not invalidate child
			b.InvalidateArrange ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (20,20), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (new Size (10,10), new Size (b.ActualWidth, b.ActualHeight),"b actual1");
			Assert.AreEqual (new Size (20,20), new Size (c.ActualWidth, c.ActualHeight), "c actual1");

			c.InvalidateArrange ();
			b.Arrange (new Rect (0,0,10,10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c DesiredSize");
			Assert.AreEqual (new Size (9,9), c.RenderSize, "c render size");
			Assert.AreEqual (new Size (9,9), c.ArrangeArg, "c measure args");
			Assert.AreEqual (new Size (1,1), b.DesiredSize, "b DesiredSize");
			Assert.AreEqual (new Size (10,10), b.RenderSize, "b render size");
			Assert.AreEqual (new Size (10,10), new Size (b.ActualWidth, b.ActualHeight),"b actual2");
			Assert.AreEqual (new Size (9,9), new Size (c.ActualWidth, c.ActualHeight), "c actual2");
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
