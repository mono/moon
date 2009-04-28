using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
	public partial class CanvasTest
	{
		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			Canvas c = new Canvas ();
			Size s = new Size (10,10);

			c.Measure (s);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			Canvas c = new Canvas ();
			Rectangle r = new Rectangle();

			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest2 ()
		{
			Canvas c = new Canvas ();
			Rectangle r = new Rectangle();

			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		[MoonlightBug]
		public void ChildMeasureTest3 ()
		{
			Canvas c = new Canvas ();
			Border r = new Border ();

			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c deisred");
			Assert.AreEqual (new Size (0,0), r.DesiredSize, "r desired");
			Assert.AreEqual (new Size (0,0), new Size (r.ActualWidth, r.ActualHeight));
		}

		[TestMethod]
		public void ChildMeasureTest4 ()
		{
			Border root = new Border ();
			Canvas c = new Canvas ();
			Border r = new Border ();

			root.Child = c;
			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
			Assert.AreEqual (new Size (50,50), r.DesiredSize);
		}

		[TestMethod]
		public void ChildBackgroundMeasureTest1 ()
		{
			Canvas c = new Canvas ();
			Rectangle r = new Rectangle();

			c.Background = new SolidColorBrush (Colors.Black);
			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void ChildBackgroundMeasureTest2 ()
		{
			Canvas c = new Canvas ();
			Rectangle r = new Rectangle();

			c.Background = new SolidColorBrush (Colors.Black);
			c.Children.Add (r);

			Canvas.SetLeft (r, 10);
			Canvas.SetTop (r, 10);

			r.Width = 50;
			r.Height = 50;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void DefaultsTest()
		{
			Rectangle r = new Rectangle();
			Assert.AreEqual(0, Canvas.GetLeft(r), "#1");
			Assert.AreEqual(0, Canvas.GetTop(r), "#2");
			Assert.AreEqual(0, Canvas.GetZIndex(r), "#3");

			Canvas c = new Canvas ();
		}

		[TestMethod]
		public void InvalidTest()
		{
			Assert.Throws<NullReferenceException>(delegate {
				Canvas.GetLeft(null);
			}, "#1");
			Assert.Throws<NullReferenceException>(delegate {
				Canvas.GetTop(null);
			}, "#2");
			Assert.Throws<NullReferenceException>(delegate {
				Canvas.GetZIndex(null);
			}, "#3");

			Assert.Throws<NullReferenceException>(delegate {
				Canvas.SetLeft(null, -100);
			}, "#4");
			Assert.Throws<NullReferenceException>(delegate {
				Canvas.SetTop(null, -100);
			}, "#5");
			Assert.Throws<NullReferenceException>(delegate {
				Canvas.SetZIndex(null, -100);
			}, "#6");
		}

		[TestMethod]
		public void WidthHeightMeasureTest1 ()
		{
			Canvas c = new Canvas ();

			c.Width = 50;
			c.Height = 60;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void WidthHeightMeasureTest2 ()
		{
			Canvas c = new Canvas ();

			c.Width = 50;
			c.Height = 60;

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}
	
		[TestMethod]
		public void WidthHeightBackgroundMeasureTest1 ()
		{
			Canvas c = new Canvas ();

			c.Background = new SolidColorBrush (Colors.Black);

			c.Width = 50;
			c.Height = 60;

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (0,0), c.DesiredSize);
		}

		[TestMethod]
		public void ComputeActualWidth ()
		{
			Canvas c = new Canvas ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");

			c.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");
		}

		[TestMethod]
		public void ComputeActualWidthBorder ()
		{
			Border b = new Border ();
			Canvas c = new Canvas ();

			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");

			Assert.IsTrue (c.UseLayoutRounding, "use rounding");
		}

		[TestMethod]
		public void ComputeActualWidthBorderMeasure ()
		{
			Border b = new Border ();
			Canvas c = new Canvas ();

			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			b.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");

			Assert.IsTrue (c.UseLayoutRounding, "use rounding");
		}

		[TestMethod]
		public void ComputeActualWidthGrid ()
		{
			Grid b = new Grid ();
			Canvas c = new Canvas ();

			b.Children.Add (c);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
		}

		[TestMethod]
		public void ComputeActualWidthStackPanel ()
		{
			StackPanel b = new StackPanel ();
			Canvas c = new Canvas ();

			b.Children.Add (c);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
		}

		[TestMethod]
		public void ComputeActualWidthCanvas ()
		{
			Canvas b = new Canvas ();
			Canvas c = new Canvas ();

			b.Children.Add (c);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
		}

		[TestMethod]
		public void WidthHeightBackgroundMeasureTest2 ()
		{
			Canvas c = new Canvas ();

			c.Background = new SolidColorBrush (Colors.Black);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.Width = 50;
			c.Height = 60;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (50,60), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (50,60), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
		}

		[TestMethod]
		public void ChildNameScope ()
		{
			Canvas b = new Canvas ();
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border>
    <Path x:Name=""foo"" Data=""F1 M 10,10 20,20 10,20"" Stroke=""Red""/>
  </Border>
</Canvas>");
			Assert.IsNotNull (c.FindName ("foo"));
			
			b.Children.Add (c);
			
			Assert.IsNull (b.FindName ("foo"));
			Assert.IsNotNull (c.FindName ("foo"));
		}

		[TestMethod]
		public void BackgroundTest ()
		{
			Border b = new Border ();
			Canvas c = new Canvas ();
			c.Background = new SolidColorBrush (Colors.Orange);
			b.Child = c;
			b.Width = 50;
			b.Height = 50;
			
			b.Measure (new Size (100,100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired");

			Assert.AreEqual (HorizontalAlignment.Stretch, c.HorizontalAlignment);
			Assert.AreEqual (VerticalAlignment.Stretch, c.VerticalAlignment);
		}

		[TestMethod]
		public void BackgroundTest2 ()
		{
			Border b = new Border ();
			Canvas c = new Canvas ();
			c.Background = new SolidColorBrush (Colors.Orange);
			b.Child = c;
			c.Width = 50;
			c.Height = 50;
			
			b.Measure (new Size (100,100));

			Assert.AreEqual (new Size (50,50), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired");

			Assert.AreEqual (HorizontalAlignment.Stretch, c.HorizontalAlignment);
			Assert.AreEqual (VerticalAlignment.Stretch, c.VerticalAlignment);
		}

		[TestMethod]
		[MoonlightBug]
		public void AttachedTest ()
		{
			Canvas c = new Canvas ();
			Border b = new Border ();
			b.Width = 10;
			b.Height = 33;
			b.Background = new SolidColorBrush (Colors.Orange);
			Canvas.SetTop (b, 88);
			Canvas.SetLeft (b, 150);
			c.Children.Add (b);

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired");

			c.Arrange (new Rect (0,0,400,400));
			
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (b));
		}

		[TestMethod]
		public void DoublesForZIndex ()
		{
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Rectangle Canvas.ZIndex=""5.0"" />
</Canvas>");

			Rectangle r = (Rectangle)VisualTreeHelper.GetChild (c, 0);
			Assert.AreEqual (5, Canvas.GetZIndex (r), "1");


		        c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Rectangle Canvas.ZIndex=""5.0E03"" />
</Canvas>");

			r = (Rectangle)VisualTreeHelper.GetChild (c, 0);
			Assert.AreEqual (5, Canvas.GetZIndex (r), "2");

		        c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Rectangle Canvas.ZIndex=""5V"" />
</Canvas>");

			r = (Rectangle)VisualTreeHelper.GetChild (c, 0);
			Assert.AreEqual (5, Canvas.GetZIndex (r), "3");

		        Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Rectangle Canvas.ZIndex=""V"" />
</Canvas>"); });
		}
	}

}
