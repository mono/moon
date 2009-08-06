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

		public static void CheckRowHeights (this Grid grid, string message, params double [] heights)
		{
			for (int i = 0; i < grid.RowDefinitions.Count; i++)
				Assert.IsBetween (heights [i] - 0.1, heights [i] + 0.1, grid.RowDefinitions [i].ActualHeight, message + "." + i);
		}
	}

	public partial class GridTest
	{
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
		[MoonlightBug]
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
		[MoonlightBug]
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
		[MoonlightBug]
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
		[MoonlightBug]
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
		[MoonlightBug]
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
}
