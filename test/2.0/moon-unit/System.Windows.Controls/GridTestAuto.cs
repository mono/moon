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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls
{
	static class GridExtensions
	{
		public static void AddChild (this Grid grid, FrameworkElement element, int row, int column, int rowspan, int columnspan)
		{
			if (row != -1)
				Grid.SetRow (element, row);
			if (rowspan != 0)
				Grid.SetRowSpan (element, rowspan);
			if (column != -1)
				Grid.SetColumn (element, column);
			if (columnspan != 0)
				Grid.SetColumnSpan (element, columnspan);
			grid.Children.Add (element);
		}

		public static void AddColumns (this Grid grid, params GridLength [] columns)
		{
			foreach (GridLength c in columns)
				grid.ColumnDefinitions.Add (new ColumnDefinition { Width = c });
		}

		public static void AddRows (this Grid grid, params GridLength [] rows)
		{
			foreach (GridLength c in rows)
				grid.RowDefinitions.Add (new RowDefinition { Height = c });
		}

		public static void ChangeRow (this Grid grid, int childIndex, int newRow)
		{
			Grid.SetRow ((FrameworkElement) grid.Children [childIndex], newRow);
		}

		public static void ChangeRowSpan (this Grid grid, int childIndex, int newRowSpan)
		{
			Grid.SetRowSpan ((FrameworkElement) grid.Children [childIndex], newRowSpan);
		}

		public static void ChangeCol (this Grid grid, int childIndex, int newCol)
		{
			Grid.SetColumn ((FrameworkElement) grid.Children [childIndex], newCol);
		}

		public static void ChangeColSpan (this Grid grid, int childIndex, int newColSpan)
		{
			Grid.SetColumnSpan ((FrameworkElement) grid.Children [childIndex], newColSpan);
		}

		public static void CheckDesired (this Grid grid, string message, params Size [] sizes)
		{
			for (int i = 0; i < grid.Children.Count; i++) {
				var poker = (MyContentControl) grid.Children [i];
				if (!poker.DesiredSize.Equals (sizes [i]))
					Assert.Fail ("{2}.{3} Expected measure result to be {0} but was {1}", sizes [i], poker.DesiredSize, message, i);
			}
		}

		public static void CheckRowHeights (this Grid grid, string message, params double [] heights)
		{
			for (int i = 0; i < grid.RowDefinitions.Count; i++)
				Assert.IsBetween (heights [i] - 0.55, heights [i] + 0.55, grid.RowDefinitions [i].ActualHeight, message + "." + i);
		}

		public static void CheckMeasureSizes (this Grid grid, string message, params Size [] sizes)
		{
			for (int i=0 ;i < grid.Children.Count; i++)
			{
				var poker = (MyContentControl) grid.Children [i];
				if (!poker.MeasureOverrideArg.Equals (sizes [i]))
					Assert.Fail ("{2}.{3} Expected measure argument to be {0} but was {1}", sizes [i], poker.MeasureOverrideArg, message, i);
			}

		}

		public static void CheckMeasureResult (this Grid grid, string message, params Size [] sizes)
		{
			for (int i = 0; i < grid.Children.Count; i++) {
				var poker = (MyContentControl) grid.Children [i];
				if (!poker.MeasureOverrideResult.Equals (sizes [i]))
					Assert.Fail ("{2}.{3} Expected measure result to be {0} but was {1}", sizes [i], poker.MeasureOverrideResult, message, i);
			}
		}

		public static void InvalidateSubtree (this UIElement element)
		{
			element.InvalidateArrange ();
			element.InvalidateMeasure ();
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount (element); i++)
				((UIElement) VisualTreeHelper.GetChild (element, i)).InvalidateSubtree ();
		}
	}

	public partial class GridTest
	{
		static Grid CreateGridWithChildren ()
		{
			Grid grid = new Grid { Name = "GridUnderTest" };
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (2, GridUnitType.Star), new GridLength (3, GridUnitType.Star));
			grid.AddColumns (new GridLength (1, GridUnitType.Star), new GridLength (2, GridUnitType.Star), new GridLength (3, GridUnitType.Star));

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (new MyContentControl { Content = new Rectangle { Fill = new SolidColorBrush (Colors.Red), MinWidth = 15, MinHeight = 15 } }, i, j, 1, 1);
			return grid;
		}

		static readonly Size Infinity = new Size (double.PositiveInfinity, double.PositiveInfinity);

		[TestMethod]
		[MoonlightBug]
		public void ChildInvalidatesGrid ()
		{
			var child = new MyContentControl (50, 50);
			Grid grid = new Grid ();
			grid.Children.Add (child);
			grid.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#1");

			((FrameworkElement) child.Content).Height = 60;
			((FrameworkElement) child.Content).Width = 10;

			grid.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (10, 60), grid.DesiredSize, "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void ExpandStarsInBorder ()
		{
			Grid grid = CreateGridWithChildren ();

			var parent = new Border ();
			parent.Child = grid;

			TestPanel.Width = 75;
			TestPanel.Height = 75;

			CreateAsyncTest (parent,
				() => {
					grid.CheckRowHeights ("#1", 12, 25, 38);

					grid.HorizontalAlignment = HorizontalAlignment.Left;
					grid.VerticalAlignment = VerticalAlignment.Center;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#2", 12, 15, 15);

					grid.Width = 50;
					grid.Height = 50;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#3", 8, 17, 25);

					grid.ClearValue (Grid.HorizontalAlignmentProperty);
					grid.ClearValue (Grid.VerticalAlignmentProperty);
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#4", 8, 17, 25);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ExpandStarsInCanvas ()
		{
			Grid grid = CreateGridWithChildren ();

			var parent = new Canvas ();
			parent.Children.Add (grid);

			TestPanel.Width = 75;
			TestPanel.Height = 75;

			CreateAsyncTest (parent,
				() => {
					grid.CheckRowHeights ("#1", 15, 15, 15);

					grid.HorizontalAlignment = HorizontalAlignment.Left;
					grid.VerticalAlignment = VerticalAlignment.Center;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#2", 15, 15, 15);

					grid.Width = 50;
					grid.Height = 50;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#3", 8, 17, 25);

					grid.ClearValue (Grid.HorizontalAlignmentProperty);
					grid.ClearValue (Grid.VerticalAlignmentProperty);
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#4", 8, 17, 25);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ExpandStarsInGrid ()
		{
			Grid grid = CreateGridWithChildren ();

			var parent = new Grid ();
			parent.AddRows (new GridLength (75));
			parent.AddColumns (new GridLength (75));
			parent.AddChild (grid, 0, 0, 1, 1);

			TestPanel.Width = 75;
			TestPanel.Height = 75;

			CreateAsyncTest (parent,
				() => {
					grid.CheckRowHeights ("#1", 12, 25, 38);

					grid.HorizontalAlignment = HorizontalAlignment.Left;
					grid.VerticalAlignment = VerticalAlignment.Center;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#2", 12, 15, 15);

					grid.Width = 50;
					grid.Height = 50;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#3", 8, 17, 25);

					grid.ClearValue (Grid.HorizontalAlignmentProperty);
					grid.ClearValue (Grid.VerticalAlignmentProperty);
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#4", 8, 17, 25);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ExpandStarsInStackPanel ()
		{
			Grid grid = CreateGridWithChildren ();

			var parent = new StackPanel ();
			parent.Children.Add (grid);

			TestPanel.Width = 75;
			TestPanel.Height = 75;

			CreateAsyncTest (parent,
				() => {
					grid.CheckRowHeights ("#1", 15, 15, 15);

					grid.HorizontalAlignment = HorizontalAlignment.Left;
					grid.VerticalAlignment = VerticalAlignment.Center;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#2", 15, 15, 15);

					grid.Width = 50;
					grid.Height = 50;
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#3", 8, 17, 25);

					grid.ClearValue (Grid.HorizontalAlignmentProperty);
					grid.ClearValue (Grid.VerticalAlignmentProperty);
					parent.InvalidateSubtree ();
				}, () => {
					grid.CheckRowHeights ("#4", 8, 17, 25);

				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureMaxAndMin ()
		{
			Grid g = new Grid ();
			var child = new MyContentControl (50, 50);
			g.AddColumns (GridLength.Auto);
			g.AddRows (GridLength.Auto, GridLength.Auto);
			g.AddChild (child, 0, 0, 1, 1);

			CreateAsyncTest (g,
				() => {
					Assert.AreEqual (Infinity, child.MeasureOverrideArg, "#1");

					// Force a redraw
					g.Children.Clear ();
					g.Children.Add (child);
					g.RowDefinitions [0].MaxHeight = 20;
				}, () => {
					Assert.AreEqual (Infinity, child.MeasureOverrideArg, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureMaxAndMin2 ()
		{
			Grid g = new Grid ();
			var child = new MyContentControl (50, 50);
			g.AddColumns (new GridLength (50));
			g.AddRows (new GridLength (50), new GridLength (50));
			g.AddChild (child, 0, 0, 1, 1);

			CreateAsyncTest (g,
				() => {
					Assert.AreEqual (new Size (50, 50), child.MeasureOverrideArg, "#1");

					// Force a redraw
					g.Children.Clear ();
					g.Children.Add (child);
					g.RowDefinitions [0].MaxHeight = 20;
				}, () => {
					Assert.AreEqual (new Size (50, 20), child.MeasureOverrideArg, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureMaxAndMin3 ()
		{
			Grid g = new Grid ();
			var child = new MyContentControl (50, 50);
			g.AddColumns (new GridLength (50));
			g.AddRows (new GridLength (20), new GridLength (20));
			g.AddChild (child, 0, 0, 2, 2);

			g.RowDefinitions [0].MaxHeight = 5;
			g.RowDefinitions [1].MaxHeight = 30;

			CreateAsyncTest (g,
				() => {
					Assert.AreEqual (25, child.MeasureOverrideArg.Height, "#1");
					g.RowDefinitions [0].MaxHeight = 10;
				}, () => {
					Assert.AreEqual (30, child.MeasureOverrideArg.Height, "#2");
					g.RowDefinitions [0].MaxHeight = 20;
				}, () => {
					Assert.AreEqual (40, child.MeasureOverrideArg.Height, "#3");
				}
			);
		}

		[TestMethod]
		public void MeasureAutoRows ()
		{
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new MyContentControl { Width = 50, Height = 50 }, 0, 0, 2, 1);
			grid.AddChild (new MyContentControl { Width = 50, Height = 60 }, 0, 1, 1, 1);

			grid.Measure (new Size (0, 0));
			grid.CheckMeasureSizes ("#1", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#2");

			grid.Measure (new Size (50, 40));
			grid.CheckMeasureSizes ("#3", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (50, 40), grid.DesiredSize, "#4");

			grid.Measure (new Size (500, 400));
			grid.CheckMeasureSizes ("#5", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (100, 60), grid.DesiredSize, "#6");
		}

		[TestMethod]
		public void MeasureAutoRows2 ()
		{
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);

			MyContentControl c = new MyContentControl (50, 50);
			grid.AddChild (c, 0, 0, 2, 1);
			grid.AddChild (new MyContentControl (50, 60), 0, 1, 1, 1);
			grid.AddChild (new MyContentControl (50, 20), 0, 1, 1, 1);

			grid.Measure (new Size (500, 400));
			grid.CheckMeasureSizes ("#1", new Size (50, inf), new Size (50, inf), new Size (50, inf));
			Assert.AreEqual (new Size (100, 60), grid.DesiredSize, "#2");

			grid.ChangeRow (2, 1);
			grid.Measure (new Size (500, 400));
			grid.CheckMeasureSizes ("#3", new Size (50, inf), new Size (50, inf), new Size (50, inf));
			Assert.AreEqual (new Size (100, 80), grid.DesiredSize, "#4");

			// FIXME: Hack to work around an invalidation issue with grid
			grid.InvalidateMeasure ();
			((FrameworkElement) c.Content).Height = 100;
			grid.Measure (new Size (500, 400));
			grid.CheckMeasureSizes ("#5", new Size (50, inf), new Size (50, inf), new Size (50, inf));
			Assert.AreEqual (new Size (100, 100), grid.DesiredSize, "#6");

			grid.ChangeRow (2, 2);
			grid.Measure (new Size (500, 400));
			grid.CheckMeasureSizes ("#7", new Size (50, inf), new Size (50, inf), new Size (50, inf));
			Assert.AreEqual (new Size (100, 120), grid.DesiredSize, "#8");
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureAutoRows3 ()
		{
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new MyContentControl (50, 50), 0, 1, 2, 1);
			grid.AddChild (new MyContentControl (50, 60), 1, 1, 1, 1);
			grid.AddChild (new MyContentControl (50, 70), 0, 1, 3, 1);

			CreateAsyncTest (grid, () => {
				grid.CheckRowHeights ("#1", 3.33, 63.33, 3.33);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureAutoRows4 ()
		{
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new MyContentControl (50, 30), 0, 1, 3, 1);
			grid.AddChild (new MyContentControl (50, 90), 0, 1, 1, 1);
			grid.AddChild (new MyContentControl (50, 50), 0, 1, 2, 1);

			grid.AddChild (new MyContentControl (50, 70), 1, 1, 4, 1);
			grid.AddChild (new MyContentControl (50, 120), 1, 1, 2, 1);
			grid.AddChild (new MyContentControl (50, 30), 2, 1, 3, 1);

			grid.AddChild (new MyContentControl (50, 10), 3, 1, 1, 1);
			grid.AddChild (new MyContentControl (50, 50), 3, 1, 2, 1);
			grid.AddChild (new MyContentControl (50, 80), 3, 1, 2, 1);

			grid.AddChild (new MyContentControl (50, 20), 4, 1, 1, 1);

			CreateAsyncTest (grid, () => {
				grid.CheckRowHeights ("#1", 90, 60, 60, 35, 45);
			});
		}

		[TestMethod]
		public void MeasureAutoAndFixedRows ()
		{
			Grid grid = new Grid {  };

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (new GridLength (20), new GridLength (20));
			grid.AddChild (new MyContentControl (50, 50), 0, 1, 2, 1);

			grid.Measure (Infinity);
			grid.CheckRowHeights ("#1", 20, 20);
			grid.CheckMeasureSizes ("#2", new Size (50, 40));
			Assert.AreEqual (new Size (100, 40), grid.DesiredSize, "#3");

			grid.RowDefinitions [0].Height = new GridLength (30);
			grid.Measure (Infinity);
			grid.CheckRowHeights ("#4", 30, 20);
			grid.CheckMeasureSizes ("#5", new Size (50, 50));
			Assert.AreEqual (new Size (100, 50), grid.DesiredSize, "#6");

			grid.RowDefinitions.Insert (0, new RowDefinition { Height = GridLength.Auto });
			grid.Measure (Infinity);
			grid.CheckRowHeights ("#7", double.PositiveInfinity, 30, 20);
			grid.CheckMeasureSizes ("#8", new Size (50, double.PositiveInfinity));
			Assert.AreEqual (new Size (100, 70), grid.DesiredSize, "#9");

			grid.Children.Clear ();
			grid.AddChild (new MyContentControl (50, 150), 0, 1, 2, 1);
			grid.Measure (Infinity);
			grid.CheckDesired ("#13", new Size (50, 150));
			grid.CheckRowHeights ("#10", double.PositiveInfinity, 30, 20);
			grid.CheckMeasureSizes ("#11", new Size (50, double.PositiveInfinity));
			grid.CheckMeasureResult ("#12", new Size (50, 150));
			Assert.AreEqual (new Size (100, 170), grid.DesiredSize, "#12");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureAutoAndStarRows ()
		{
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, new GridLength (1, GridUnitType.Star), GridLength.Auto, GridLength.Auto);

			grid.AddChild (new MyContentControl { Width = 50, Height = 50 }, 0, 0, 3, 1);
			grid.AddChild (new MyContentControl { Width = 50, Height = 60 }, 1, 0, 3, 1);

			grid.Measure (new Size (100, 100));
			grid.CheckRowHeights ("#1", inf, inf, 100, inf, inf);
			grid.CheckMeasureSizes ("#2", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (50, 60), grid.DesiredSize, "#3");

			grid.RowDefinitions [2].MaxHeight = 15;
			grid.Measure (new Size (100, 100));
			grid.CheckRowHeights ("#4", inf, inf, 15, inf, inf);
			grid.CheckMeasureSizes ("#5", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (50, 15), grid.DesiredSize, "#6");

			grid.RowDefinitions.Clear ();
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, new GridLength (1, GridUnitType.Star), GridLength.Auto);
			grid.Measure (new Size (100, 100));
			grid.CheckRowHeights ("#7", inf, inf, inf, 50, inf);
			grid.CheckMeasureSizes ("#8", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (50, 77), grid.DesiredSize, "#9");

			grid.RowDefinitions [3].MaxHeight = 15;
			grid.Measure (new Size (100, 100));
			grid.CheckRowHeights ("#10", inf, inf, inf, 15, inf);
			grid.CheckMeasureSizes ("#11", new Size (50, 50), new Size (50, 60));
			Assert.AreEqual (new Size (50, 65), grid.DesiredSize, "#12");
		}

		[TestMethod]
		public void MeasureStarRowsNoChild ()
		{
			// Measuring the rows initialises the sizes to Infinity for 'star' elements
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star));
			grid.AddColumns (new GridLength (1, GridUnitType.Star));

			// Initial values
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#1");
			Assert.AreEqual (0, grid.RowDefinitions [0].ActualHeight, "#2");
			Assert.AreEqual (0, grid.ColumnDefinitions [0].ActualWidth, "#3");

			// After measure
			grid.Measure (Infinity);
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#4");
			Assert.AreEqual (inf, grid.RowDefinitions [0].ActualHeight, "#5");
			Assert.AreEqual (inf, grid.ColumnDefinitions [0].ActualWidth, "#6");

			// Measure again
			grid.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#7");
			Assert.AreEqual (inf, grid.RowDefinitions [0].ActualHeight, "#8");
			Assert.AreEqual (inf, grid.ColumnDefinitions [0].ActualWidth, "#9");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureStarRowsWithChild ()
		{
			// Measuring the rows initialises the sizes to Infinity for 'star' elements
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star));
			grid.Children.Add (new MyContentControl (50, 50));

			// Initial values
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#1");
			Assert.AreEqual (0, grid.RowDefinitions [0].ActualHeight, "#2");

			// After measure
			grid.Measure (Infinity);
			Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#4");
			Assert.AreEqual (inf, grid.RowDefinitions [0].ActualHeight, "#5");

			// Measure again
			grid.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#7");
			Assert.AreEqual (100, grid.RowDefinitions [0].ActualHeight, "#8");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureStarRowsWithChild2 ()
		{
			// Measuring the rows initialises the sizes to Infinity for 'star' elements
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Star));
			grid.Children.Add (new MyContentControl (50, 50));

			// Initial values
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#1");
			grid.CheckRowHeights ("#1", 0, 0);

			// After measure
			grid.Measure (Infinity);
			Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#4");
			grid.CheckRowHeights ("#2", inf, inf);

			// Measure again
			grid.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#7");
			grid.CheckRowHeights ("#3", 50, 50);
		}

		[TestMethod]
		[Asynchronous]
		public void RowspanAutoTest ()
		{
			// This test demonstrates the following rules:
			// 1) Elements with RowSpan/ColSpan == 1 distribute their height first
			// 2) The rest of the elements distribute height in LIFO order
			Grid grid = new Grid ();
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);
			grid.AddColumns (new GridLength (50));

			var child50 = new MyContentControl (50, 50);
			var child60 = new MyContentControl (50, 60);

			grid.AddChild (child50, 0, 0, 1, 1);
			grid.AddChild (child60, 0, 0, 1, 1);

			CreateAsyncTest (grid,
				() => {
					// Check the initial values
					grid.CheckRowHeights ("#1", 60, 0, 0);

					// Now make the smaller element use rowspan = 2
					Grid.SetRowSpan (child50, 2);
				}, () => {
					grid.CheckRowHeights ("#2", 60, 0, 0);

					// Then make the larger element us rowspan = 2
					Grid.SetRowSpan (child50, 1);
					Grid.SetRowSpan (child60, 2);
				}, () => {
					grid.CheckRowHeights ("#3", 55, 5, 0);

					// Swap the order in which they are added to the grid
					grid.Children.Clear ();
					grid.AddChild (child60, 0, 0, 2, 0);
					grid.AddChild (child50, 0, 0, 1, 0);
				}, () => {
					// Swapping the order has no effect here
					grid.CheckRowHeights ("#4", 55, 5, 0);

					// Then give both rowspan = 2
					Grid.SetRowSpan (child50, 2);
				}, () => {
					grid.CheckRowHeights ("#5", 30, 30, 0);

					// Finally give the larger element rowspan = 3
					Grid.SetRowSpan (child60, 3);
				}, () => {
					grid.CheckRowHeights ("#6", 28.333, 28.333, 3.333);

					// Swap the order in which the elements are added again
					grid.Children.Clear ();
					grid.AddChild (child50, 0, 0, 2, 0);
					grid.AddChild (child60, 0, 0, 3, 0);
				}, () => {
					grid.CheckRowHeights ("#7", 25, 25, 20);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void SizeExceedsBounds ()
		{
			Grid grid = new Grid ();
			grid.RowDefinitions.Add (new RowDefinition { Height = new GridLength (50), MaxHeight = 40, MinHeight = 60 });
			grid.AddChild (new MyContentControl (50, 50), 0, 0, 0, 0);
			CreateAsyncTest (grid, () => {
				Assert.AreEqual (60, grid.RowDefinitions [0].ActualHeight, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void SizeExceedsBounds2 ()
		{
			Grid grid = new Grid ();
			grid.RowDefinitions.Add (new RowDefinition { Height = new GridLength (50), MaxHeight = 60, MinHeight = 40 });
			grid.RowDefinitions.Add (new RowDefinition { Height = new GridLength (50), MaxHeight = 60, MinHeight = 40 });
			grid.AddChild (new MyContentControl (100, 1000), 0, 0, 0, 0);
			CreateAsyncTest (grid,
				() => {
					Assert.AreEqual (50, grid.RowDefinitions [0].ActualHeight, "#1");
					grid.ChangeRowSpan (0, 2);
				}, () => {
					Assert.AreEqual (50, grid.RowDefinitions [0].ActualHeight, "#1");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void StarRows ()
		{
			GridUnitType star = GridUnitType.Star;
			Grid grid = new Grid { Width = 100, Height = 210 };
			grid.AddRows (new GridLength (1, star), new GridLength (2, star));
			grid.AddChild (new MyContentControl (50, 50), 0, 0, 0, 0);
			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 70, 140);
				}, () => {
					grid.AddRows (new GridLength (30));
				}, () => {
					grid.CheckRowHeights ("#2", 60, 120, 30);

					// Add a child to the fixed row
					grid.AddChild (new MyContentControl (50, 80), 2, 0, 0, 0);
				}, () => {
					grid.CheckRowHeights ("#3", 60, 120, 30);

					// Make the child span the last two rows
					grid.ChangeRow (1, 1);
					grid.ChangeRowSpan (1, 2);
				}, () => {
					grid.CheckRowHeights ("#4", 60, 120, 30);

					// Add another fixed row and move the large child to span both
					grid.AddRows (new GridLength (30));
					grid.ChangeRow (1, 2);
				}, () => {
					grid.CheckRowHeights ("#5", 50, 100, 30, 30);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void StarRows2 ()
		{
			GridUnitType star = GridUnitType.Star;
			Grid grid = new Grid { Width = 100, Height = 210 };
			grid.AddRows (new GridLength (1, star), new GridLength (2, star));
			grid.AddChild (new MyContentControl (50, 50), 0, 0, 0, 0);
			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 70, 140);
				}, () => {
					grid.AddRows (GridLength.Auto);
				}, () => {
					grid.CheckRowHeights ("#2", 70, 140, 0);

					// Add a child to the fixed row
					grid.AddChild (new MyContentControl (50, 80), 2, 0, 0, 0);
				}, () => {
					// FIXME: There is some rounding done here, but i don't fully
					// understand when or where. We give a final size of 43.333 instead
					// of 43, and 86.666 instead of 87.
					//grid.CheckRowHeights ("#3", 43, 87, 80);
					Assert.IsBetween (43, 44, grid.RowDefinitions [0].ActualHeight, "#3a");
					Assert.IsBetween (86, 87, grid.RowDefinitions [1].ActualHeight, "#3b");
					Assert.AreEqual (80, grid.RowDefinitions [2].ActualHeight, "#3c");

					// Make the child span the last two rows
					grid.ChangeRow (1, 1);
					grid.ChangeRowSpan (1, 2);
				}, () => {
					grid.CheckRowHeights ("#4", 70, 140, 0);

					// Add another fixed row and move the large child to span both
					grid.AddRows (GridLength.Auto);
					grid.ChangeRow (1, 2);
				}, () => {
					// FIXME: Same as above
					//grid.CheckRowHeights ("#5", 43, 87, 40, 40);
					Assert.IsBetween (43, 44, grid.RowDefinitions [0].ActualHeight, "#3a");
					Assert.IsBetween (86, 87, grid.RowDefinitions [1].ActualHeight, "#3b");
					Assert.AreEqual (40, grid.RowDefinitions [2].ActualHeight, "#3c");
					Assert.AreEqual (40, grid.RowDefinitions [3].ActualHeight, "#3c");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void StarRows3 ()
		{
			GridLength oneStar = new GridLength (1, GridUnitType.Star);
			Grid grid = new Grid ();
			grid.AddRows (oneStar, oneStar, oneStar);
			grid.AddColumns (oneStar, oneStar, oneStar);

			Canvas canvas = new Canvas { Width = 120, Height = 120 };
			canvas.Children.Add (grid);
			grid.AddChild (new MyContentControl {
				Content = new Rectangle {
					Height = 100,
					Width = 100,
					Fill = new SolidColorBrush (Colors.Transparent)
				}
			}, 1, 1, 1, 1);

			CreateAsyncTest (canvas,
				() => { }, () => { }, () => { },
				() => {
					grid.CheckMeasureResult ("#1", new Size (100, 100));
					grid.CheckRowHeights ("#2", 0, 100, 0);
				}
			);
		}

		[TestMethod]
		[MoonlightBug]
		public void StarRows4 ()
		{
			GridLength oneStar = new GridLength (1, GridUnitType.Star);
			Grid grid = new Grid { Name = "MyGrid" };
			grid.AddRows (oneStar, oneStar, oneStar);
			grid.AddColumns (oneStar, oneStar, oneStar);

			grid.AddChild (new MyContentControl { Content = new Rectangle { Width = 240, Height = 240 } }, 0, 0, 3, 3);
			grid.AddChild (new MyContentControl { Content = new Rectangle { Width = 150, Height = 150 } }, 0, 0, 1, 1);

			TestPanel.Children.Add (grid);

			TestPanel.Measure (new Size (240, 240));
			TestPanel.Arrange (new Rect (0, 0, 480, 480));

			grid.CheckMeasureSizes ("#2", new Size (240, 240), new Size (80, 80));
			grid.CheckMeasureResult ("#3", new Size (240, 240), new Size (80, 80));
			grid.CheckDesired ("#4", new Size (240, 240), new Size (80, 80));

			grid.CheckRowHeights ("#1", 80, 80, 80);
		}

		[TestMethod]
		[MoonlightBug]
		public void StarRows5 ()
		{
			GridLength oneStar = new GridLength (1, GridUnitType.Star);
			Grid grid = new Grid { Name = "MyGrid", HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center };
			grid.AddRows (oneStar, oneStar, oneStar);
			grid.AddColumns (oneStar, oneStar, oneStar);

			grid.AddChild (new MyContentControl { Content = new Rectangle { Width = 240, Height = 240 } }, 0, 0, 3, 3);
			grid.AddChild (new MyContentControl { Content = new Rectangle { Width = 150, Height = 150 } }, 0, 0, 1, 1);

			TestPanel.Children.Add (grid);

			grid.Measure (new Size (240, 240));
			grid.Arrange (new Rect (0, 0, 120, 120));

			grid.CheckMeasureSizes ("#2", new Size (240, 240), new Size (80, 80));
			grid.CheckMeasureResult ("#3", new Size (240, 240), new Size (80, 80));
			grid.CheckDesired ("#4", new Size (240, 240), new Size (80, 80));

			grid.CheckRowHeights ("#1", 80, 80, 80);
		}

		[TestMethod]
		[Asynchronous]
		public void StarRowsWithChild ()
		{
			// Measuring the rows initialises the sizes to Infinity for 'star' elements
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star));
			grid.Children.Add (new MyContentControl (50, 50));

			// Initial values
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#1");
			Assert.AreEqual (0, grid.RowDefinitions [0].ActualHeight, "#2");

			TestPanel.Height = 400;
			CreateAsyncTest (grid,
				() => {
					Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#4");
					Assert.AreEqual (400, grid.RowDefinitions [0].ActualHeight, "#5");
					TestPanel.Height = 100;
				}, () => {
					Assert.AreEqual (100, grid.RowDefinitions [0].ActualHeight, "#8");
					Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#7");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void StarRowsWithChild2 ()
		{
			double inf = double.PositiveInfinity;
			Grid grid = new Grid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Star));
			grid.Children.Add (new MyContentControl (50, 50));

			// Initial values
			Assert.AreEqual (new Size (0, 0), grid.DesiredSize, "#1");
			grid.CheckRowHeights ("#2", 0, 0);

			TestPanel.Height = 400;
			CreateAsyncTest (grid,
				() => {
					Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#3");
					grid.CheckRowHeights ("#4", 200, 200);

					TestPanel.Width = 100;
					TestPanel.Height = 100;
				}, () => {
					Assert.AreEqual (new Size (50, 50), grid.DesiredSize, "#5");
					grid.CheckRowHeights ("#6", 50, 50);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoRows ()
		{
			// This checks that rows expand to be large enough to hold the largest child
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 1, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 0, 1, 1, 1);

			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 60, 0);
					Grid.SetRow ((FrameworkElement) grid.Children [1], 1);
				}, () => {
					grid.CheckRowHeights ("#2", 50, 60);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoRows2 ()
		{
			// Start off with two elements in the first row with the smaller element having rowspan = 2
			// and see how rowspan affects the rendering.
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 2, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 0, 1, 1, 1);

			// Start off with both elements at row 1, and the smaller element having rowspan = 2
			CreateAsyncTest (grid,
				() => {
					// If an element spans across multiple rows and one of those rows
					// is already large enough to contain that element, it puts itself
					// entirely inside that row
					grid.CheckRowHeights ("#1", 60, 0, 0);

					grid.ChangeRow (1, 1);
				}, () => {
					// An 'auto' row which has no children whose rowspan/colspan
					// *ends* in that row has a height of zero
					grid.CheckRowHeights ("#2", 0, 60, 0);
					grid.ChangeRow (1, 2);
				}, () => {
					// If an element which spans multiple rows is the only element in
					// the rows it spans, it divides evenly between the rows it spans
					grid.CheckRowHeights ("#2", 25, 25, 60);
					grid.ChangeRow (1, 0);
					grid.ChangeRow (0, 1);
				}, () => {
					grid.CheckRowHeights ("#2", 60, 25, 25);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoRows3 ()
		{
			// Start off with two elements in the first row with the larger element having rowspan = 2
			// and see how rowspan affects the rendering.
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 1, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 0, 1, 2, 1);

			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 55, 5, 0);
					grid.ChangeRow (1, 1);
				}, () => {
					grid.CheckRowHeights ("#2", 50, 30, 30);
					grid.ChangeRow (1, 2);
				}, () => {
					grid.CheckRowHeights ("#3", 50, 0, 60);
					grid.ChangeRow (1, 0);
					grid.ChangeRow (0, 1);
				}, () => {
					grid.CheckRowHeights ("#3", 5, 55, 0);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoRows4 ()
		{
			// See how rowspan = 3 affects this with 5 rows.
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto);

			// Give first child a rowspan of 2
			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 1, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 0, 1, 3, 1);

			CreateAsyncTest (grid,
				() => {
					// If an element spans across multiple rows and one of those rows
					// is already large enough to contain that element, it puts itself
					// entirely inside that row
					grid.CheckRowHeights ("#1", 53.33, 3.33, 3.33, 0, 0);
					grid.ChangeRow (1, 1);
				}, () => {
					// An 'auto' row which has no children whose rowspan/colspan
					// *ends* in that row has a height of zero
					grid.CheckRowHeights ("#2", 50, 20, 20, 20, 0);
					grid.ChangeRow (1, 2);
				}, () => {
					// If an element which spans multiple rows is the only element in
					// the rows it spans, it divides evenly between the rows it spans
					grid.CheckRowHeights ("#3", 50, 0, 20, 20, 20);

					grid.ChangeRow (1, 0);
					grid.ChangeRow (0, 1);
				}, () => {
					// If there are two auto rows beside each other and an element spans those
					// two rows, the total height is averaged between the two rows.
					grid.CheckRowHeights ("#4", 3.33, 53.33, 3.33, 0, 0);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoRows5 ()
		{
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 3, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 1, 0, 3, 1);

			// When calculating the heights of automatic rows, the children added to the grid
			// distribute their height in the opposite order in which they were added.
			CreateAsyncTest (grid,
				() => {
					// Here the element with height 60 distributes its height first
					grid.CheckRowHeights ("#1", 3.33, 23.33, 23.33, 20, 0);
					grid.ChangeRow (1, 1);

					grid.ChangeRow (0, 1);
					grid.ChangeRow (1, 0);
				}, () => {
					// Reversing the rows does not stop the '60' element from
					// Distributing its height first
					grid.CheckRowHeights ("#2", 20, 23.33, 23.33, 3.33, 0);

					// Now reverse the order in which the elements are added so that
					// the '50' element distributes first.
					grid.Children.Clear ();
					grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 1, 0, 3, 1);
					grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 3, 1);

				}, () => {
					grid.CheckRowHeights ("#3", 16.66, 25.55, 25.55, 8.88, 0);
					grid.ChangeRow (1, 1);

					grid.ChangeRow (0, 1);
					grid.ChangeRow (1, 0);
				}, () => {
					grid.CheckRowHeights ("#4", 16.66, 25.55, 25.55, 8.88, 0);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoAndFixedRows ()
		{
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, new GridLength (15), GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 3, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 1, 0, 3, 1);

			// If an element spans multiple rows and one of them is *not* auto, it attempts to put itself
			// entirely inside that row
			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 0, 0, 60, 0, 0);

					// Forcing a maximum height on the fixed row makes it distribute
					// remaining height among the 'auto' rows.
					grid.RowDefinitions [2].MaxHeight = 15;
				}, () => {
					grid.CheckRowHeights ("#2", 6.25, 28.75, 15, 22.5, 0);

					grid.RowDefinitions.Clear ();
					grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, new GridLength (15), GridLength.Auto);
				}, () => {
					grid.CheckRowHeights ("#3", 16.66, 16.66, 16.66, 60, 0);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void AutoAndFixedRows2 ()
		{
			TestPanel.Width = 200;
			TestPanel.Height = 1000;
			Grid grid = new Grid {  };
			grid.AddColumns (new GridLength (50), new GridLength (50), new GridLength (50));
			grid.AddRows (new GridLength (30), new GridLength (40), GridLength.Auto, new GridLength (50));
			grid.AddChild (new MyContentControl (600, 600), 0, 0, 4, 4);
			grid.AddChild (new MyContentControl (80, 70), 0, 1, 1, 1);
			grid.AddChild (new MyContentControl (50, 60), 1, 0, 1, 1);
			grid.AddChild (new MyContentControl (10, 500), 1, 1, 1, 1);

			CreateAsyncTest (grid, () => {
				grid.CheckRowHeights ("#1", 190, 200, 0, 210);
				grid.CheckMeasureSizes ("#2",
											new Size (150, double.PositiveInfinity),
											new Size (50, 30),
											new Size (50, 40),
											new Size (50, 40));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void AutoAndFixedRows3 ()
		{
			Grid grid = new Grid { Width = 10, Height = 10 };
			grid.AddColumns (new GridLength (50), new GridLength (50));
			grid.AddRows (new GridLength (20), new GridLength (20));

			grid.AddChild (new MyContentControl (50, 50), 0, 1, 2, 1);

			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 20, 20);
					grid.RowDefinitions.Insert (0, new RowDefinition { Height = GridLength.Auto });
				}, () => {
					grid.CheckRowHeights ("#2", 0, 50, 20);
					grid.RowDefinitions [1].MaxHeight = 35;
				}, () => {
					grid.CheckRowHeights ("#3", 15, 35, 20);
					grid.RowDefinitions [1].MaxHeight = 20;
					grid.ChangeRowSpan (0, 4);
				}, () => {
					grid.CheckRowHeights ("#4", 0, 20, 30);
					grid.AddRows (new GridLength (20));
				}, () => {
					grid.CheckRowHeights ("#5", 0, 20, 20, 20);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void AutoAndStarRows ()
		{
			TestPanel.MaxHeight = 160;
			Grid grid = new Grid ();

			grid.AddColumns (new GridLength (50));
			grid.AddRows (GridLength.Auto, GridLength.Auto, new GridLength (1, GridUnitType.Star), GridLength.Auto, GridLength.Auto);

			grid.AddChild (new LayoutPoker { Width = 50, Height = 50 }, 0, 0, 3, 1);
			grid.AddChild (new LayoutPoker { Width = 50, Height = 60 }, 1, 0, 3, 1);

			// Elements will put themselves entirely inside a 'star' row if they can
			CreateAsyncTest (grid,
				() => {
					grid.CheckRowHeights ("#1", 0, 0, 160, 0, 0);

					// Forcing a maximum height on the star row doesn't spread
					// remaining height among the auto rows.
					grid.RowDefinitions [2].MaxHeight = 15;
				}, () => {
					grid.CheckRowHeights ("#2", 0, 0, 15, 0, 0);

					grid.RowDefinitions.Clear ();
					grid.AddRows (GridLength.Auto, GridLength.Auto, GridLength.Auto, new GridLength (1, GridUnitType.Star), GridLength.Auto);
				}, () => {
					grid.CheckRowHeights ("#3", 16.66, 16.66, 16.66, 110, 0);

					grid.RowDefinitions [3].MaxHeight = 15;
				}, () => {
					grid.CheckRowHeights ("#3", 16.66, 16.66, 16.66, 15, 0);
				}
			);
		}
	}

	class MyContentControl : ContentControl
	{
		public Size MeasureOverrideArg;
		public Size ArrangeOverrideArg;
		public Size MeasureOverrideResult;
		public Size ArrangeOverrideResult;

		public MyContentControl ()
		{
		}

		public MyContentControl (int width, int height)
		{
			Content = new Rectangle { Width = width, Height = height, Fill = new SolidColorBrush (Colors.Green) };
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			if (Parent is MyGrid)
				((MyGrid) Parent).ArrangedElements.Add (this);
			ArrangeOverrideArg = finalSize;
			ArrangeOverrideResult = base.ArrangeOverride (finalSize);
			return ArrangeOverrideResult;
		}

		protected override Size MeasureOverride (Size availableSize)
		{
			if (Parent is MyGrid)
				((MyGrid) Parent).MeasuredElements.Add (this);
			MeasureOverrideArg = availableSize;
			MeasureOverrideResult = base.MeasureOverride (availableSize);
			return MeasureOverrideResult;
		}
	}
}
