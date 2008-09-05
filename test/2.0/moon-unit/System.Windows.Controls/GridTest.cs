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
	public class GridTest
	{

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			Grid g = new Grid ();

			g.Measure (new Size (200, 200));

			Assert.AreEqual (new Size (0,0), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		[KnownFailure]
		public void ChildlessMarginTest ()
		{
			Grid g = new Grid ();

			g.Margin = new Thickness (5);

			g.Measure (new Size (200, 200));

			Assert.AreEqual (new Size (10,10), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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

			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");

			// now test with the child sized smaller than the row/column definitions
			c.Width = 100;
			c.Height = 100;

			g.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (210, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		[KnownFailure]
		public void ChildMargin_starWidth_starHeight_singleCell ()
		{
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

		[TestMethod]
		[KnownFailure]
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
		[KnownFailure]
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
		[KnownFailure]
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

			Assert.AreEqual (new Size (410, 210), g.DesiredSize, "DesiredSize");

			g.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (100, 100), g.DesiredSize, "DesiredSize");
		}
	}
}
