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

		[TestMethod]
		[KnownFailure]
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
	}
}
