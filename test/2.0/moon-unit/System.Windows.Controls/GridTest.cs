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
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class GridTest
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
		public void Defaults()
		{
			Grid g = new Grid();
			Assert.AreEqual(0, g.GetValue(Grid.ColumnProperty), "#1");
			Assert.AreEqual(1, g.GetValue(Grid.ColumnSpanProperty), "#2");
			Assert.AreEqual(0, g.GetValue(Grid.RowProperty), "#3");
			Assert.AreEqual(1, g.GetValue(Grid.RowSpanProperty), "#4");
			Assert.AreEqual(false, g.GetValue(Grid.ShowGridLinesProperty), "#5");

			Rectangle r1 = new Rectangle();
			Rectangle r2 = new Rectangle();
			g.Children.Add(r1);
			g.Children.Add(r2);

			Assert.AreEqual(0, Grid.GetColumn(r1), "#6");
			Assert.AreEqual(0, Grid.GetColumn(r2), "#7");
			Assert.AreEqual(0, Grid.GetRow(r1), "#8");
			Assert.AreEqual(0, Grid.GetRow(r2), "#9");
		}

		[TestMethod]
		public void InvalidValues()
		{
			Grid g = new Grid();
			Rectangle r1 = new Rectangle();
			Rectangle r2 = new Rectangle();

			g.Children.Add(r1);
			g.Children.Add(r2);

			Assert.Throws<ArgumentException>(delegate {
				r1.SetValue(Grid.ColumnProperty, -1);
			});
			Assert.Throws<ArgumentException>(delegate {
				Grid.SetColumn(r1, -1);
			});
			Assert.Throws<ArgumentException>(delegate {
				Grid.SetColumnSpan(r1, 0);
			});
			Assert.Throws<ArgumentException>(delegate {
				Grid.SetColumnSpan(r1, -1);
			});

			Assert.Throws<ArgumentException>(delegate {
				Grid.SetRow(r1, -1);
			});
			Assert.Throws<ArgumentException>(delegate {
				Grid.SetRowSpan(r1, 0);
			});
			Assert.Throws<ArgumentException>(delegate {
				Grid.SetRowSpan(r1, -1);
			});
		}

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			Grid g = new Grid ();

			g.Measure (new Size (200, 200));

			Assert.AreEqual (new Size (0,0), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessWidthHeightMeasureTest ()
		{
			Grid g = new Grid ();

			g.Width = 300;
			g.Height = 300;

			g.Measure (new Size (200, 200));

			Assert.AreEqual (new Size (200,200), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMarginTest ()
		{
			Grid g = new Grid ();

			g.Margin = new Thickness (5);

			g.Measure (new Size (200, 200));

			Assert.AreEqual (new Size (10,10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void Childless_ColumnDefinition_Width_constSize_singleColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = new GridLength (200);
			g.ColumnDefinitions.Add (def);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (200, 0), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 0), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_constSize_singleColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = new GridLength (200);
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (210, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_constSize_multiColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = new GridLength (200);
			g.ColumnDefinitions.Add (def);

			def = new ColumnDefinition ();
			def.Width = new GridLength (200);
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (410, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_autoSize_singleColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = GridLength.Auto;
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_autoSize_constSize_multiColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = GridLength.Auto;
			g.ColumnDefinitions.Add (def);

			def = new ColumnDefinition ();
			def.Width = new GridLength(200);
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (210, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_starSize_singleColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = new GridLength (2, GridUnitType.Star);
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_ColumnDefinition_Width_starSize_constSize_multiColumn ()
		{
			Grid g = new Grid ();

			ColumnDefinition def;

			def = new ColumnDefinition ();
			def.Width = new GridLength (2, GridUnitType.Star);
			g.ColumnDefinitions.Add (def);

			def = new ColumnDefinition ();
			def.Width = new GridLength(200);
			g.ColumnDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (210, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_constSize_singleRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = new GridLength (200);
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 100), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_constSize_multiRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = new GridLength (200);
			g.RowDefinitions.Add (def);

			def = new RowDefinition ();
			def.Height = new GridLength (200);
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 410), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 100), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_autoSize_singleRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = GridLength.Auto;
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_autoSize_constSize_multiRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = GridLength.Auto;
			g.RowDefinitions.Add (def);

			def = new RowDefinition ();
			def.Height = new GridLength(200);
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 100), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_starSize_singleRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = new GridLength (2, GridUnitType.Star);
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMargin_RowDefinition_Height_starSize_constSize_multiRow ()
		{
			Grid g = new Grid ();

			RowDefinition def;

			def = new RowDefinition ();
			def.Height = new GridLength (2, GridUnitType.Star);
			g.RowDefinitions.Add (def);

			def = new RowDefinition ();
			def.Height = new GridLength(200);
			g.RowDefinitions.Add (def);

			g.Margin = new Thickness (5);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (10, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (10, 100), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildMargin_constWidth_constHeight_singleCell ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (200);
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			Canvas c = new Canvas ();

			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);

			g.Children.Add (c);

			// first test with the child sized larger than the row/column definitions
			c.Width = 400;
			c.Height = 400;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (200, 200), c.DesiredSize, "DesiredSize0");
			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "DesiredSize1");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize2");

			// now test with the child sized smaller than the row/column definitions
			c.Width = 100;
			c.Height = 100;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "DesiredSize3");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize4");
		}

		[TestMethod]
		public void ChildMargin_starWidth_starHeight_singleCell ()
		{
			Console.WriteLine ("Childless_ColumnDefinition_Width_constSize_singleColumn");

			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (1, GridUnitType.Star);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (1, GridUnitType.Star);
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			Canvas c = new Canvas ();

			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);

			g.Children.Add (c);

			// first test with the child sized larger than the row/column definitions
			c.Width = 400;
			c.Height = 400;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (410, 410), g.DesiredSize, "DesiredSize1");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize2");

			// now test with the child sized smaller than the row/column definitions
			c.Width = 100;
			c.Height = 100;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (110, 110), g.DesiredSize, "DesiredSize3");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize4");
		}

		[TestMethod]
		public void ChildMargin_autoWidth_autoHeight_singleCell ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = GridLength.Auto;
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = GridLength.Auto;
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			Canvas c = new Canvas ();

			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);

			g.Children.Add (c);

			// first test with the child sized larger than the row/column definitions
			c.Width = 400;
			c.Height = 400;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (410, 410), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");

			// now test with the child sized smaller than the row/column definitions
			c.Width = 100;
			c.Height = 100;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (110, 110), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");
		}

		// two children, two columns, one row.  the children
		// are both explicitly sized, but the column
		// definitions are 1* and 2* respectively.
		[TestMethod]
		public void TwoChildrenMargin_2Columns_1Star_and_2Star_1Row_constSize ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (1, GridUnitType.Star);
			g.ColumnDefinitions.Add (cdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (2, GridUnitType.Star);
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			Canvas c;

			c = new Canvas ();
			c.Width = 400;
			c.Height = 400;
			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);
			g.Children.Add (c);

			c = new Canvas ();
			c.Width = 400;
			c.Height = 400;
			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 1);
			g.Children.Add (c);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (810, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");
		}

		// two children, two columns, one row.  the children
		// are both explicitly sized, but the column
		// definitions are 1* and 2* respectively.
		[TestMethod]
		public void Child_ColSpan2_2Columns_constSize_and_1Star_1Row_constSize ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (200);
			g.ColumnDefinitions.Add (cdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (2, GridUnitType.Star);
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			Canvas c;

			c = new Canvas ();
			c.Width = 400;
			c.Height = 400;
			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);
			Grid.SetColumnSpan (c, 2);
			g.Children.Add (c);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (400, 200), c.DesiredSize, "DesiredSize0");

			Assert.AreEqual (new Size (410, 210), g.DesiredSize, "DesiredSize1");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize2");
		}

		// 3 children, two columns, two rows.  the columns
		// are Auto sized, the rows are absolute (200 pixels
		// each).
		// 
		// +-------------------+
		// |                   |
		// |     child1        |
		// |                   |
		// +--------+----------+
		// |        |          |
		// | child2 |  child3  |
		// |        |          |
		// +--------+----------+
		//
		// child1 has colspan of 2
		// child2 and 3 are explicitly sized (width = 150 and 200, respectively)
		//
		[TestMethod]
		public void ComplexLayout1 ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			// Add rows
			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = GridLength.Auto;
			g.ColumnDefinitions.Add (cdef);

			cdef = new ColumnDefinition ();
			cdef.Width = GridLength.Auto;
			g.ColumnDefinitions.Add (cdef);

			Canvas child1, child2, child3;

			// child1
			child1 = new Canvas ();
			child1.Width = 200;
			child1.Height = 200;
			Grid.SetRow (child1, 0);
			Grid.SetColumn (child1, 0);
			Grid.SetColumnSpan (child1, 2);
			g.Children.Add (child1);

			// child2
			child2 = new Canvas ();
			child2.Width = 150;
			child2.Height = 200;
			Grid.SetRow (child2, 0);
			Grid.SetColumn (child2, 0);
			g.Children.Add (child2);

			// child3
			child3 = new Canvas ();
			child3.Width = 200;
			child3.Height = 200;
			Grid.SetRow (child3, 0);
			Grid.SetColumn (child3, 0);
			g.Children.Add (child3);

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (200, 400), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ComplexLayout2 ()
		{
			Grid g = new Grid ();

			RowDefinition rdef;
			ColumnDefinition cdef;

			rdef = new RowDefinition ();
			rdef.Height = new GridLength (200);
			g.RowDefinitions.Add (rdef);

			cdef = new ColumnDefinition ();
			cdef.Width = new GridLength (200);
			g.ColumnDefinitions.Add (cdef);

			g.Margin = new Thickness (5);

			LayoutPoker c = new LayoutPoker ();

			Grid.SetRow (c, 0);
			Grid.SetColumn (c, 0);

			g.Children.Add (c);

			c.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			// first test with the child sized larger than the row/column definitions
			c.Width = 400;
			c.Height = 400;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (400, c.Width);
			Assert.AreEqual (400, c.Height);

			Assert.AreEqual (new Size (200, 200), c.DesiredSize, "c DesiredSize0");
			Assert.AreEqual (new Size (400, 400), c.MeasureArg, "c MeasureArg0");
			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "grid DesiredSize0");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "grid DesiredSize1");
			Assert.AreEqual (new Size (400, 400), c.MeasureArg, "c MeasureArg");
			Assert.AreEqual (new Size (200, 200), c.DesiredSize, "c DesiredSize1");

			// now test with the child sized smaller than the row/column definitions
			c.Width = 100;
			c.Height = 100;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (100, 100), c.MeasureArg, "c MeasureArg2");
			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "grid DesiredSize2");
		}
	}
}
