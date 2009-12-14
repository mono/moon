using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Markup;
using System.Windows.Media;
using Mono.Moonlight.UnitTesting;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Shapes
{
	[TestClass]
	public partial class PathTest : SilverlightTest
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
		public void ComputeActualWidth ()
		{
			var c = new Path ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");

			c.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render3");
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void ComputeActualSizeIntrisicCanvas_InTree ()
		{
			var panel = new Canvas ();
			var c = new Path ();

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			panel.Children.Add (c);

			CreateAsyncTest (panel, () => {
					Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
					Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
					Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");
				});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void ComputeActualSizeIntrisicGrid_InTree ()
		{
			var panel = new Grid ();
			var c = new Path ();

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			panel.Children.Add (c);

			CreateAsyncTest (panel, () => {
					Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired");
					Assert.AreEqual (new Size (649,580), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
					Assert.AreEqual (new Size (649,580), c.RenderSize, "c render");
				});
		}

		[TestMethod]
		[Asynchronous]
		public void ComputeActualSizeIntrisicGridCanvas_InTree ()
		{
			var parentPanel = new Canvas ();
			var panel = new Grid ();
			var c = new Path ();

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			panel.Children.Add (c);
			parentPanel.Children.Add (panel);

			CreateAsyncTest (parentPanel, () => {
					Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired");
					Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
					Assert.AreEqual (new Size (25,33), c.RenderSize, "c render");
				});
		}

		[TestMethod]
		public void ComputeActualSizeIntrinsic ()
		{
			var c = new Path ();

			//Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			//Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");

			c.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");
		}

		[TestMethod]
		public void ComputeActualSizeIntrinsicCanvas ()
		{
			Canvas canvas = new Canvas ();
			var c = new Path ();

			//Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			//Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			canvas.Children.Add (c);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			canvas.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");

			canvas.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");
		}

		[TestMethod]
		public void ComputeActualSizeIntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");

			c.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (25,33), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,33), c.RenderSize, "c render");
		}

		[TestMethod]
		public void ComputeActualSizeIntrinsicBorder_Margin ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			c.Margin = new Thickness (2,4,5,6);

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (32,43), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render");

			c.Arrange (new Rect (0,0,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Rect (0,0,32,43), LayoutInformation.GetLayoutSlot (c), "c slot3");
			Assert.AreEqual (new Size (32,43), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,33), c.RenderSize, "c render");
		}

		[TestMethod]
		public void ComputeRestrainedSizeIntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,33), c.RenderSize, "c render");
			Assert.AreEqual (new Rect (10,10,10,10), LayoutInformation.GetLayoutSlot (c), "c slot2");

		}

		[TestMethod]
		public void ComputeReducedSizeIntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (10,10), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,33), c.RenderSize, "c render3");
			Assert.AreEqual (new Rect (10,10,10,10), LayoutInformation.GetLayoutSlot (c), "c slot3");

		}

		[TestMethod]
		public void ComputeLargerSize_StretchFill_IntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.Fill;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100,100), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (8,6,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (100,100), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (100,100), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (100,100), c.RenderSize, "c render3");
			Assert.AreEqual (new Rect (8,6,100,100), LayoutInformation.GetLayoutSlot (c), "c slot3");

		}

		[TestMethod]
		public void ComputeLargerSize_StretchUniform_IntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.Uniform;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100,92), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (8,6,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (100,92), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (100,92), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (100,92), c.RenderSize, "c render3");
			Assert.AreEqual (new Rect (8,6,100,92), LayoutInformation.GetLayoutSlot (c), "c slot3");

		}

		[TestMethod]
		public void BorderComputeLargerSize_StretchUniform_IntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.Uniform;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			b.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100,92), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			b.Arrange (new Rect (8,6,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Size (100,92), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (100,92), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (100,92), c.RenderSize, "c render3");
			Assert.AreEqual (new Rect (0,0,100,92), LayoutInformation.GetLayoutSlot (c), "c slot3");

		}

		[TestMethod]
		[MoonlightBug ("Layout rounding regression")]
		public void BorderComputeLargerSize_StretchUniform_SizedIntrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.Uniform;
			b.Width = 75;
			b.Height = 50;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");
			
			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			b.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (54,50), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			b.Arrange (new Rect (8,6,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Size (54,50), c.DesiredSize, "c desired3");
			Assert.IsTrue (c.ActualWidth < 54.4 && c.ActualWidth > 54.3, "c.ActualWidth == " + c.ActualWidth.ToString ());
			Assert.AreEqual (50, c.ActualHeight, "c actual.height");
			Assert.IsTrue (c.RenderSize.Width < 54.4 && c.RenderSize.Width > 54.3, "c.RenderSize.Width = " + c.RenderSize.Width.ToString ());
			Assert.AreEqual (50, c.RenderSize.Height, "c render.height");
			Assert.AreEqual (new Rect (0,0,75,50), LayoutInformation.GetLayoutSlot (c), "c slot2");
		}
		
		[TestMethod]
		public void BorderComputeLargerSize_StretchUniformToFill_IntrinsicBorderSized ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.UniformToFill;
			b.Width = 75;
			b.Height = 66;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			b.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (75,66), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			b.Arrange (new Rect (8,6,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Size (75,66), c.DesiredSize, "c desired3");
			Assert.AreEqual (new Size (75,69), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (75,69), c.RenderSize, "c render3");
			Assert.AreEqual (new Rect (0,0,75,66), LayoutInformation.GetLayoutSlot (c), "c slot3");
		}

		[TestMethod]
		[MoonlightBug]
		public void BorderComputeLargerSize_StretchUniformToFill_AlignCenter_IntrinsicBorderSized ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;
			c.Stretch = Stretch.UniformToFill;
			b.Width = 75;
			c.VerticalAlignment = VerticalAlignment.Center;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			b.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (75,100), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			b.Arrange (new Rect (8,6,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Size (75,100), c.DesiredSize, "c desired3");
			Assert.AreEqual (109, c.ActualWidth, "c.ActualWidth");
			Assert.IsTrue (c.RenderSize.Height < 100.3 && c.RenderSize.Height > 100.2, "c.renderSize.Height = "+ c.RenderSize.Height.ToString ());
			Assert.IsTrue (c.ActualHeight < 100.3 && c.ActualHeight > 100.2, "c.ActualHeight = "+ c.ActualHeight.ToString ());

			Assert.AreEqual (109, c.RenderSize.Width, "c.RenderSize.Width");
			Assert.AreEqual (new Rect (0,0,75,100), LayoutInformation.GetLayoutSlot (c), "c slot3");
		}

		[TestMethod]
		[MoonlightBug]
		public void ComputeReducedSize_StretchUniform_InstrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			b.Width = 10;
			b.Height = 10;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");
			Assert.IsTrue (c.UseLayoutRounding, "rounding?");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			c.Stretch = Stretch.Uniform;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,9), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (10,9), c.DesiredSize, "c desired");
			Assert.AreEqual (c.RenderSize, new Size (c.ActualWidth, c.ActualHeight), " render == actual");
			Assert.IsTrue (c.ActualWidth < 9.8 && c.ActualWidth > 9.7, "c.ActualWidth == " + c.ActualWidth.ToString ());
			Assert.AreEqual (9, c.ActualHeight, "c actual.height");
			Assert.AreEqual (new Rect (10,10,10,9), LayoutInformation.GetLayoutSlot (c), "c slot2");

		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("Layout rounding regression")]
		public void ComputeReducedSize_StretchUniform_InstrinsicBorder_InTree ()
		{
			Border b = new Border ();
			b.UseLayoutRounding = true;
			var c = new Path ();
			b.Child = c;

			b.Width = 10;
			b.Height = 10;

			Assert.IsTrue (c.UseLayoutRounding, "use rounding");
			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			c.Stretch = Stretch.Uniform;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,9), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			CreateAsyncTest (b, () => {
					Assert.AreEqual (new Rect (0,0,10,10), LayoutInformation.GetLayoutSlot (c), "c slot2");
					Assert.AreEqual (new Size (10,9), c.DesiredSize, "c desired");
					Assert.AreEqual (c.RenderSize, new Size (c.ActualWidth, c.ActualHeight), "render == actual");
					Assert.AreEqual (10, c.ActualWidth, "c.ActualWidth == " + c.ActualWidth.ToString ());
					Assert.IsTrue (c.ActualHeight > 9 && c.ActualHeight < 9.2, "c actual.height == " + c.ActualHeight.ToString ());
				});
		}

		[TestMethod]
		public void ComputeSizeInfinite_StretchFill_InstrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			c.Stretch = Stretch.Fill;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (25,23), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (25,23), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,23), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,23), c.RenderSize, "c render");
			Assert.AreEqual (new Rect (10,10,25,23), LayoutInformation.GetLayoutSlot (c), "c slot2");

		}

		[TestMethod]
		public void ComputeSizeInfinite_StretchUniform_InstrinsicBorder ()
		{
			Border b = new Border ();
			var c = new Path ();
			b.Child = c;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual0");

			var data = new RectangleGeometry ();
			data.Rect = new Rect (0,10,25,23);
			c.Data = data;
			c.Stretch = Stretch.Uniform;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot");

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (25,23), c.DesiredSize, "c desired2");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
			Assert.AreEqual (new Size (0,0), c.RenderSize, "c render2");
			Assert.AreEqual (new Rect (0,0,0,0), LayoutInformation.GetLayoutSlot (c), "c slot2");

			c.Arrange (new Rect (10,10,c.DesiredSize.Width,c.DesiredSize.Height));

			Assert.AreEqual (new Size (25,23), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,23), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,23), new Size (c.ActualWidth,c.ActualHeight), "c actual3");
			Assert.AreEqual (new Size (25,23), c.RenderSize, "c render");
			Assert.AreEqual (new Rect (10,10,25,23), LayoutInformation.GetLayoutSlot (c), "c slot2");
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
			Assert.AreEqual (new Size (50,50), foo.RenderSize, "foo render1 " + (foo.RenderSize.Width - 50.0) + " " + (foo.RenderSize.Height - 50.0));
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

			b.InvalidateMeasure ();
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
			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "render");
			Assert.AreEqual (0, path.ActualWidth);
			Assert.AreEqual (0, path.ActualHeight);

			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (90,100), path.RenderSize, "render");
			Assert.AreEqual (90, path.ActualWidth);
			Assert.AreEqual (100, path.ActualHeight);
		}

		[TestMethod]
		public void ArrangeTest5_Rounding()
		{
			Border b = new Border ();
			var path = new Path ();
			//var canvas = new Canvas ();
			b.Child = path;
			//canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80.7, 90.1);
			path.Data = r;
			path.Width = 20;
			path.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			Assert.IsTrue (path.UseLayoutRounding, "path rounds?");
			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "render");
			Assert.AreEqual (0, path.ActualWidth);
			Assert.AreEqual (0, path.ActualHeight);

			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (91,100), path.RenderSize, "render");
			Assert.AreEqual (91, path.ActualWidth);
			Assert.AreEqual (100, path.ActualHeight);
		}


		[TestMethod]
		public void Arrange_Rounding()
		{
			Border b = new Border ();
			var path = new Path ();
			//var canvas = new Canvas ();
			b.Child = path;
			//canvas.Children.Add (b);
			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10, 10, 80.1, .2);
			path.Data = r;
			
			path.Width = 20;
			path.Height = 20;
			path.Fill = new SolidColorBrush (Colors.Red);

			b.Measure (new Size (120, 120));
			Assert.IsTrue (path.UseLayoutRounding, "path rounds?");
			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (0,0), path.RenderSize, "render");
			Assert.AreEqual (0, path.ActualWidth);
			Assert.AreEqual (0, path.ActualHeight);

			b.Arrange (new Rect (0, 0, 120, 120));

			Assert.AreEqual (new Size (20,20), path.DesiredSize, "desired");
			Assert.AreEqual (new Size (90,20), path.RenderSize, "render");
			Assert.AreEqual (90, path.ActualWidth);
			Assert.AreEqual (20, path.ActualHeight);
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
		[MoonlightBug]
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
