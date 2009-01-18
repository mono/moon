using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Markup;
using System.Windows.Media;
using Mono.Moonlight.UnitTesting;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Shapes
{
	[TestClass]
	public class PathTest
	{
		[TestMethod]
		public void DefaultValues ()
		{
			Path p = new Path ();
			Assert.IsNull (p.Data, "Data");
			ShapeTest.CheckDefaultValues (p, Stretch.None);
		}

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
		[MoonlightBug ("This looks like a measure invalidation bug in Silverlights System.Windows.Path")]
		public void MeasureRectangleBorderTest1 ()
		{
			// See 1_1 1_2 for demonstation of this as a measure invalidation bug.
			Border b = new Border ();
			Path p = new Path ();
			p.Fill = new SolidColorBrush (Colors.Black);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;
			b.Child = p;

			b.Measure (new Size (50, 50));
			b.Arrange (new Rect (0, 0, 50, 50));

			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (50,50), b.RenderSize, "b RenderSize");
			Assert.AreEqual (new Size (0,0), p.DesiredSize, "p DesiredSize");
			Assert.AreEqual (new Size (50,50), p.RenderSize, "p RenderSize");
		}

		[TestMethod]
		public void MeasureRectangleBorderTest1_1 ()
		{
			// This is the same as the above test but has
			Border b = new Border ();
			Path p = new Path ();
			b.Child = p;
			p.Fill = new SolidColorBrush (Colors.Black);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;

			b.Measure (new Size (50, 50));
			b.Arrange (new Rect (0, 0, 50, 50));

			Assert.AreEqual (new Size (30,30), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (30,30), p.DesiredSize, "p desired");
			Assert.AreEqual (new Size (50,50), b.RenderSize, "b RenderSize");
			Assert.AreEqual (new Size (50,50), p.RenderSize, "p RenderSize");
		}

		[TestMethod]
		public void MeasureRectangleBorderTest1_2 ()
		{
			// See test 1_1 and 1 for more information
			Border b = new Border ();
			Path p = new Path ();
			p.Fill = new SolidColorBrush (Colors.Black);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 20, 20);
			p.Data = r;
			b.Child = p;

			p.InvalidateMeasure ();
			b.Measure (new Size (50, 50));
			b.Arrange (new Rect (0, 0, 50, 50));

			Assert.AreEqual (new Size (30,30), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (30,30), p.DesiredSize, "p DesiredSize");
			Assert.AreEqual (new Size (50,50), b.RenderSize, "b RenderSize");
			Assert.AreEqual (new Size (50,50), p.RenderSize, "p RenderSize");
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

			Assert.AreEqual (new Size (20, 20), b.DesiredSize, "b desiredsize");
			Assert.AreEqual (new Size (20, 20), p.DesiredSize, "p desiredsize");
		}

		[TestMethod]
		public void Measure_StretchFill_Test1 ()
		{
			Border b = new Border ();
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border Width=""50"" Height=""50"">
    <Path x:Name=""foo"" Stretch=""Fill"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Red""/>

  </Border>
</Canvas>");
			Path foo = (Path)c.FindName ("foo");
			Assert.AreEqual (new Size (0,0), foo.DesiredSize, "foo desired-2");

			b.Child = c;

			Assert.AreEqual (new Size (0,0), foo.DesiredSize, "foo desired-1");

			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (50,50), foo.DesiredSize, "foo desired0");
			Assert.AreEqual (new Size (0,0), foo.RenderSize, "foo render0");

			b.Arrange (new Rect (0, 0, 300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired1");

			Assert.AreEqual (new Size (50,50), foo.DesiredSize, "foo desired1");
			Assert.AreEqual (new Size (50,50), foo.RenderSize, "foo render1");
			Assert.AreEqual (new Size (50,50), new Size (foo.ActualWidth, foo.ActualHeight), "foo actual0");
			Border foo_parent = (Border)VisualTreeHelper.GetParent (foo);
			Assert.AreEqual (new Size (50,50), foo_parent.DesiredSize, "foo_parent desired1");
			Assert.AreEqual (new Size (50,50), foo_parent.RenderSize, "foo_parent render1");
			Assert.AreEqual (new Size (50,50), new Size (foo_parent.ActualWidth, foo_parent.ActualHeight), "foo_parent actual1");
			
		}

		[TestMethod]
		public void Measure_StretchFill_Test2 ()
		{
			Border b = new Border ();
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Path x:Name=""bar"" Width=""50"" Height=""50"" Stretch=""Fill"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Blue""/>
</Canvas>");

			b.Child = c;
			Path bar = (Path)c.FindName ("bar");

			bar.InvalidateMeasure ();
			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (0,0), bar.DesiredSize, "bar desired0");
			Assert.AreEqual (new Size (0,0), bar.RenderSize, "bar render0");

			Assert.AreEqual (new Size (50,50), new Size (bar.ActualWidth, bar.ActualHeight), "bar actual1");

			b.Arrange (new Rect (10, 10, 300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired1");
			Assert.AreEqual (new Size (0,0), bar.DesiredSize, "bar desired1");
			Assert.AreEqual (new Size (0,0), bar.RenderSize, "bar render1");

			Assert.AreEqual (new Size (50,50), new Size (bar.ActualWidth, bar.ActualHeight), "bar actual1");
		}

		[TestMethod]
		public void Measure_StretchFill_Test3 ()
		{
			Border b = new Border ();
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border Width=""50"" Height=""50"">
    <Path Width=""25"" Height=""25"" x:Name=""foo"" Stretch=""Fill"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Red""/>
  </Border>
</Canvas>");
			b.Child = c;
			Path foo = (Path)c.FindName ("foo");

			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired0");
			Assert.AreEqual (new Size (0,0), foo.RenderSize, "foo render0");

			b.Arrange (new Rect (10, 10, 300, 300));
			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired1");

			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired1");
			Assert.AreEqual (new Size (25,25), foo.RenderSize, "foo render1");
			Tester.WriteLine (String.Format ("foo.Actual ({0},{1})", foo.ActualWidth, foo.ActualHeight));
			Assert.AreEqual (new Size (25,25), new Size (foo.ActualWidth, foo.ActualHeight), "foo actual0");

			Border foo_parent = (Border)VisualTreeHelper.GetParent (foo);
			Assert.AreEqual (new Size (50,50), foo_parent.DesiredSize, "foo_parent desired1");
			Assert.AreEqual (new Size (50,50), foo_parent.RenderSize, "foo_parent render1");
			Tester.WriteLine (String.Format ("foo_parent.Actual ({0},{1})", foo_parent.ActualWidth, foo_parent.ActualHeight));
			Assert.AreEqual (new Size (50,50), new Size (foo_parent.ActualWidth, foo_parent.ActualHeight), "foo_parent actual1");
			
		}

		[TestMethod]
		public void MeasureStretchTest4 ()
		{
			Border b = (Border)XamlReader.Load (@"
  <Border xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" Width=""50"" Height=""50"">
    <Path Width=""25"" Height=""25"" x:Name=""foo"" Stretch=""Fill"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Red""/>
  </Border>");
			Path foo = (Path)b.FindName ("foo");

			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired0");
			Assert.AreEqual (new Size (0,0), foo.RenderSize, "foo render0");

			b.Arrange (new Rect (10, 10, 300, 300));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired1");

			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired1");
			Tester.WriteLine (String.Format ("foo.Actual ({0},{1})", foo.ActualWidth, foo.ActualHeight));

			Assert.AreEqual (new Size (25,25), new Size (foo.ActualWidth, foo.ActualHeight), "foo actual0");
			Assert.AreEqual (new Size (25,25), foo.RenderSize, "foo render1");

			Border foo_parent = (Border)VisualTreeHelper.GetParent (foo);
			Assert.AreEqual (new Size (50,50), foo_parent.DesiredSize, "foo_parent desired1");
			Assert.AreEqual (new Size (50,50), foo_parent.RenderSize, "foo_parent render1");
			Tester.WriteLine (String.Format ("foo_parent.Actual ({0},{1})", foo_parent.ActualWidth, foo_parent.ActualHeight));
			Assert.AreEqual (new Size (50,50), new Size (foo_parent.ActualWidth, foo_parent.ActualHeight), "foo_parent actual1");
		}

		[TestMethod]
		public void Measure_StretchNone_Test()
		{
			Border b = (Border)XamlReader.Load (@"
  <Border xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" Width=""50"" Height=""50"">
    <Path Width=""25"" Height=""25"" x:Name=""foo"" Stretch=""None"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Red""/>
  </Border>");
			Path foo = (Path)b.FindName ("foo");

			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired0");
			Assert.AreEqual (new Size (0,0), foo.RenderSize, "foo render0");

			b.Arrange (new Rect (10, 10, 300, 300));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired1");

			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired1");
			Tester.WriteLine (String.Format ("foo.Actual ({0},{1})", foo.ActualWidth, foo.ActualHeight));

			Assert.AreEqual (new Size (25,25), new Size (foo.ActualWidth, foo.ActualHeight), "foo actual0");
			Assert.AreEqual (new Size (25,25), foo.RenderSize, "foo render1");

			Border foo_parent = (Border)VisualTreeHelper.GetParent (foo);
			Assert.AreEqual (new Size (50,50), foo_parent.DesiredSize, "foo_parent desired1");
			Assert.AreEqual (new Size (50,50), foo_parent.RenderSize, "foo_parent render1");
			Tester.WriteLine (String.Format ("foo_parent.Actual ({0},{1})", foo_parent.ActualWidth, foo_parent.ActualHeight));
			Assert.AreEqual (new Size (50,50), new Size (foo_parent.ActualWidth, foo_parent.ActualHeight), "foo_parent actual1");
			
		}

		[TestMethod]
		public void Measure_StretchNone_Test2()
		{
			Border b = (Border)XamlReader.Load (@"
  <Border xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" Width=""50"" Height=""50"">
    <Path Width=""25"" Height=""25"" x:Name=""foo"" Stretch=""None"" Data=""F1 M 10,10 20,20 10,20"" StrokeLineJoin=""Round"" Stroke=""Red""/>
  </Border>");
			Path foo = (Path)b.FindName ("foo");

			b.Measure (new Size (300, 300));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired0");
			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired0");
			Assert.AreEqual (new Size (0,0), foo.RenderSize, "foo render0");

			b.Arrange (new Rect (10, 10, b.DesiredSize.Width, b.DesiredSize.Height));
			Assert.AreEqual (new Size (50,50), b.DesiredSize, "b desired1");

			Assert.AreEqual (new Size (25,25), foo.DesiredSize, "foo desired1");
			Tester.WriteLine (String.Format ("foo.Actual ({0},{1})", foo.ActualWidth, foo.ActualHeight));

			Assert.AreEqual (new Size (25,25), new Size (foo.ActualWidth, foo.ActualHeight), "foo actual0");
			Assert.AreEqual (new Size (25,25), foo.RenderSize, "foo render1");

			Border foo_parent = (Border)VisualTreeHelper.GetParent (foo);
			Assert.AreEqual (new Size (50,50), foo_parent.DesiredSize, "foo_parent desired1");
			Assert.AreEqual (new Size (50,50), foo_parent.RenderSize, "foo_parent render1");
			Tester.WriteLine (String.Format ("foo_parent.Actual ({0},{1})", foo_parent.ActualWidth, foo_parent.ActualHeight));
			Assert.AreEqual (new Size (50,50), new Size (foo_parent.ActualWidth, foo_parent.ActualHeight), "foo_parent actual1");
			
		}

		[TestMethod]
		public void Measure_StretchNone_Test3 ()
		{
			Canvas c = new Canvas ();
			Path p = new Path ();
			var r = new RectangleGeometry ();
			r.Rect = new Rect (10,11,25,47);
			p.Data = r;
			c.Children.Add (p);
			p.Stretch = Stretch.None;
			
			c.InvalidateMeasure ();
			c.Measure (new Size (300, 300));

			Assert.AreEqual (new Size (0,0), new Size (p.ActualWidth, p.ActualHeight), "p actual 3");
			Assert.AreEqual (new Size (0,0), p.DesiredSize, "p desired");
		}


		[TestMethod]
		public void Measure_StretchNone_Test4 ()
		{
			Canvas c = new Canvas ();
			Path p = new Path ();
			var r = new RectangleGeometry ();
			r.Rect = new Rect (10,11,25,47);
			p.Data = r;
			c.Children.Add (p);
			p.Stretch = Stretch.None;
			p.Width = 30;
			p.Height = 11;

			c.InvalidateMeasure ();
			c.Measure (new Size (300, 300));

			Assert.AreEqual (new Size (30,11), new Size (p.ActualWidth, p.ActualHeight), "p actual 3");
			Assert.AreEqual (new Size (0,0), p.DesiredSize, "p desired");
		}

		[TestMethod]
		public void ArrangeTest1()
		{
			Border b = new Border ();
			var path = new Path ();
			b.Child = path;
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			
			path.Stretch = Stretch.None;
			path.Data = r;
			
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (90,100), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (120,120), path.RenderSize, "render");
			Assert.AreEqual (120, path.ActualWidth);
			Assert.AreEqual (120, path.ActualHeight);
		}

		[TestMethod]
		public void ArrangeTest2()
		{
			Border b = new Border ();
			var path = new Path ();
			var canvas = new Canvas ();
			b.Child = canvas;
			canvas.Children.Add (path);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (0,0), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "render");
			Assert.AreEqual (0, path.ActualWidth);
			Assert.AreEqual (0, path.ActualHeight);
		}

		[TestMethod]
		public void ArrangeTest3()
		{
			Border b = new Border ();
			var path = new Path ();
			var canvas = new Canvas ();
			b.Child = path;
			canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			Assert.AreEqual (new Size (90,100), path.DesiredSize, "desired");

			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (90,100), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (120,120), path.RenderSize, "render");
			Assert.AreEqual (120, path.ActualWidth);
			Assert.AreEqual (120, path.ActualHeight);
		}

		[TestMethod]
		public void ArrangeTest4()
		{
			Border b = new Border ();
			var path = new Path ();
			var canvas = new Canvas ();
			b.Child = path;
			canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			b.Arrange (new Rect (0, 0, b.DesiredSize.Width, b.DesiredSize.Height));

			Assert.AreEqual (new Size (90,100), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (90,100), path.RenderSize, "render");
			Assert.AreEqual (90, path.ActualWidth);
			Assert.AreEqual (100, path.ActualHeight);
		}

		[TestMethod]
		public void ArrangeTest5()
		{
			Border b = new Border ();
			var path = new Path ();
			//var canvas = new Canvas ();
			b.Child = path;
			//canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			path.Width = 20;
			path.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (90,100), path.RenderSize, "render");
			Assert.AreEqual (90, path.ActualWidth);
			Assert.AreEqual (100, path.ActualHeight);
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTest6()
		{
			Canvas c = new Canvas ();
			Border b = new Border ();
			var path = new Path ();
			b.Child = path;
			c.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			path.Width = 20;
			path.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			c.Measure (new Size (120, 120));
			c.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (0,0), b.RenderSize, "b rendersize");
			Assert.AreEqual (new Size (0,0), new Size (b.ActualHeight,b.ActualHeight), "b actual");
			Assert.AreEqual (new Size (0,0), path.DesiredSize, "path desired");
			Assert.AreEqual (new Size (90,100), path.RenderSize, "path. render");
			Assert.AreEqual (new Size (90,100), new Size (path.ActualWidth, path.ActualHeight), "path actual");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTest7()
		{
			Canvas c = new Canvas ();
			Border b = new Border ();
			var path = new Path ();
			b.Child = path;
			c.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			b.Width = 20;
			b.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			c.Measure (new Size (120, 120));
			c.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (0,0), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (20,20), b.RenderSize, "b rendersize");
			Assert.AreEqual (new Size (20,20), new Size (b.ActualHeight,b.ActualHeight), "b actual");
			Assert.AreEqual (new Size (20,20), path.DesiredSize, "path desired");
			Assert.AreEqual (new Size (90,100), path.RenderSize, "path. render");
			Assert.AreEqual (new Size (90,100), new Size (path.ActualWidth, path.ActualHeight), "path actual");
		}

		[TestMethod]
		public void ArrangeTest8 ()
		{
			Canvas c = new Canvas ();
			Border b = new Border ();
			var path = new Path ();
			b.Child = c;
			c.Children.Add (path);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			b.Width = 20;
			b.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (20,20), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (20,20), b.RenderSize, "b rendersize");
			Assert.AreEqual (new Size (20,20), new Size (b.ActualHeight,b.ActualHeight), "b actual");
			Assert.AreEqual (new Size (0,0), path.DesiredSize, "path desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "path. render");
			Assert.AreEqual (new Size (0,0), new Size (path.ActualWidth, path.ActualHeight), "path actual");
		}
	
		[TestMethod]
		[MoonlightBug ("Missing exception")]
		public void ReuseGeometryTest ()
		{
			Path path1 = new Path ();
			Path path2 = new Path ();
			RectangleGeometry geom = new RectangleGeometry ();
			geom.Rect = new Rect (0, 0, 100, 100);
			path1.Data = geom;
			Assert.Throws<ArgumentException>(delegate {
					path2.Data = geom;
			}, "reuse");
		}

		[TestMethod]
		public void UpdateLayoutTest ()
		{
			Border b = new Border ();
			var path = new Path ();
			var canvas = new Canvas ();
			b.Child = path;
			canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80, 90);
			path.Data = r;
			
			path.Fill = new SolidColorBrush (Colors.Red);

			b.UpdateLayout ();
			path.UpdateLayout ();

			Assert.AreEqual (new Size (0,0), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "render");
			Assert.AreEqual (0, path.ActualWidth);
			Assert.AreEqual (0, path.ActualHeight);
		}
	}
}
