using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class BorderTest
	{
		[TestMethod]
		public void Defaults ()
		{
			Border c = new Border ();

			Assert.AreEqual(null, c.Background);
			Assert.AreEqual(null, c.BorderBrush);
			Assert.AreEqual(new Thickness(0), c.BorderThickness);
			Assert.AreEqual(null, c.Child);
			Assert.AreEqual(new CornerRadius(0, 0, 0, 0), c.CornerRadius);
			Assert.AreEqual (new Thickness(0), c.Padding);

			FrameworkElementTest.CheckDefaultProperties (c);
		}

		[TestMethod]
		public void InvalidValues()
		{
			Border c = new Border();

			Assert.Throws<Exception>(delegate {
				c.BorderThickness = new Thickness(double.MinValue);
			}, "#1");
			Assert.Throws<ArgumentException>(delegate {
				c.BorderThickness = new Thickness(-1);
			}, "#2");
			Assert.Throws<Exception>(delegate {
				c.BorderThickness = new Thickness(double.MaxValue);
			}, "#3");

			Assert.Throws<ArgumentException>(delegate {
				// it's actually the CornerRadius ctor that is throwing this exception
				c.CornerRadius = new CornerRadius(double.MinValue);
			}, "#4");
			Assert.Throws<ArgumentException>(delegate {
				// it's actually the CornerRadius ctor that is throwing this exception
				c.CornerRadius = new CornerRadius(-1);
			}, "#5");
			Assert.Throws<Exception>(delegate {
				c.CornerRadius = new CornerRadius(double.MaxValue);
			}, "#6");

			// FIXME: The maximum value for padding seems to be ~1.7976931348623149E+308
			Assert.Throws<Exception>(delegate {
				c.Padding = new Thickness(double.MinValue);
			}, "#7");
			Assert.Throws<ArgumentException>(delegate {
				c.Padding = new Thickness(-1);
			}, "#8");
			Assert.Throws<Exception>(delegate {
				c.Padding = new Thickness(double.MaxValue);
			}, "#9");
		}

		[TestMethod]
		public void ChildlessMeasureTest1 ()
		{
			Border c = new Border ();
			Size s = new Size (10,10);

			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMeasureTest2 ()
		{
			Border c = new Border ();
			Size s = new Size (10,10);
			
			c.Width = 20;
			c.Height = 20;

			c.Measure (s);

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "DesiredSize");
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

			Assert.AreEqual (new Size (10,10), r.DesiredSize);
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
		public void ChildMeasureTest3 ()
		{
			Border b = new Border ();
			Canvas c = new Canvas ();
			
			c.Width = 30;
			c.Height = 30;
			
			c.Measure (new Size (40, 40));
			Assert.AreEqual (new Size (0,0), c.DesiredSize);

			b.Child = c;

			c.Measure (new Size (40, 40));
			Assert.AreEqual (new Size (c.Width,c.Height), c.DesiredSize);

			b.Measure (new Size (40, 40));
			
			Assert.AreEqual (new Size (30,30), b.DesiredSize);
			Assert.AreEqual (new Size (30,30), c.DesiredSize);

			c.Measure (new Size (40, 40));
			Assert.AreEqual (new Size (30,30), c.DesiredSize);
			
			c.Width = 20;
			c.Measure (new Size (40, 40));

			Assert.AreEqual (new Size (20,30), c.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest4 ()
		{
			Border b1 = new Border ();
			Border b2 = new Border ();
			Border b3 = new Border ();
			
			b1.Child = b2;
			b2.Child = b3;
			b1.Measure (new Size (20, 20));
			
			Assert.AreEqual (new Size (0,0), b1.DesiredSize, "b1 before");
			Assert.AreEqual (new Size (0,0), b2.DesiredSize, "b2 before");
			Assert.AreEqual (new Size (0,0), b3.DesiredSize, "b3 before");

			b2.Width = 10;
			b2.Height = 10;
			
			b1.Measure (new Size (20, 20));
			
			Assert.AreEqual (new Size (10,10), b1.DesiredSize, "b1 after");
			Assert.AreEqual (new Size (10,10), b2.DesiredSize, "b2 after");
			Assert.AreEqual (new Size (0,0), b3.DesiredSize, "b3 after");
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
			Assert.AreEqual (0, c.ActualWidth);
			Assert.AreEqual (0, c.ActualHeight);
		}

		[TestMethod]
		public void ChildPaddingMeasureTest1_2()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;

			c.Padding = new Thickness(10);
			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize);
			Assert.AreEqual (0, c.ActualWidth);
			Assert.AreEqual (0, c.ActualHeight);
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
		public void ChildBorderBrushThicknessMeasureTest1 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;
			c.BorderBrush = new SolidColorBrush (Colors.Black);
			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize);
		}

		[TestMethod]
		public void ChildBorderBrushThicknessMeasureTest2 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;
			c.BorderBrush = new SolidColorBrush (Colors.Black);
			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (60,60), c.DesiredSize);
		}

		[TestMethod]
		public void ChildBorderThicknessMeasureTest1 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;
			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize);
		}

		[TestMethod]
		public void ChildBorderThicknessMeasureTest2 ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle();

			c.Child = r;
			c.BorderThickness = new Thickness (5);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (60,60), c.DesiredSize);
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
		public void ArrangeTest_RenderSize_ChildLargerThanFinalRect ()
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
		public void ArrangeTest_ChildLargerThanFinalRect ()
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
		public void ArrangeTest_ChildLargerThanFinalRect_WithBorderMargin ()
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
		public void ArrangeTest_ChildLargerThanFinalRect_WithBorderBrushAndThickness ()
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
		public void ArrangeTest_ChildLargerThanFinalRect_WithBorderThickness ()
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
			Assert.AreEqual (new Size (50, 50), r.RenderSize);
		}

		[TestMethod]
		public void ArrangeTest_ChildUnsetSize ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			/* don't specify a width/height */

			c.Measure (new Size (25, 25));
			Assert.AreEqual (0, r.ActualWidth);
			Assert.AreEqual (0, r.ActualHeight);
			Assert.AreEqual (0, c.ActualWidth);
			Assert.AreEqual (0, c.ActualHeight);
			Assert.AreEqual (new Size (0, 0), r.DesiredSize, "r desired 0");
			Assert.AreEqual (new Size (0, 0), c.DesiredSize, "c desired 0");

			c.Arrange (new Rect (0, 0, 25, 25));

			Assert.AreEqual (25, r.ActualWidth);
			Assert.AreEqual (25, r.ActualHeight);
			Assert.AreEqual (new Size (0, 0), r.DesiredSize, "r desired 1");
			Assert.AreEqual (new Size (25, 25), r.RenderSize, "r render 1");
			Assert.AreEqual (new Size (0, 0), c.DesiredSize, "c desired 1");
			Assert.AreEqual (new Size (25, 25), c.RenderSize, "c render 1");
		}

		[TestMethod]
		public void MeasureTest_TransformedChild ()
		{
			Border c = new Border ();
			Rectangle r = new Rectangle ();

			c.Child = r;

			/* give the child a width and height */
			r.Width = 20;
			r.Height = 20;

			/* and add a render transform to see if it
			   affects either call.  it shouldn't. */
			ScaleTransform s = new ScaleTransform ();
			s.ScaleX = s.ScaleY = 2.0;

			r.RenderTransform = s;
			
			c.Measure (new Size (250, 250));
			c.Arrange (new Rect (0, 0, 250, 250));

			Assert.AreEqual (new Size (20, 20), r.DesiredSize);
			Assert.AreEqual (new Size (20, 20), r.RenderSize);
		}

		[TestMethod]
		public void ChildNameScope ()
		{
			Border b = new Border ();
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border>
    <Path x:Name=""foo"" Data=""F1 M 10,10 20,20 10,20"" Stroke=""Red""/>
  </Border>
</Canvas>");
			Assert.IsNotNull (c.FindName ("foo"),"c before");
			
			b.Child = c;
			
			Assert.IsNull (b.FindName ("foo"),"b after");
			Assert.IsNotNull (c.FindName ("foo"),"c after");
		}
	}
}
