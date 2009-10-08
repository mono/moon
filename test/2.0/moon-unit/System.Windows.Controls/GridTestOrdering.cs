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
using Microsoft.Silverlight.Testing;
using System.Collections;

namespace MoonTest.System.Windows.Controls
{
	public partial class GridTest
	{
		static readonly double inf = double.PositiveInfinity;
		[TestMethod]
		[Asynchronous]

		MyContentControl ContentControlWithChild ()
		{
			return new MyContentControl { Content = new Rectangle { Width = 50, Height = 50, Fill = new SolidColorBrush (Colors.Red) } };
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyGrid grid = new MyGrid { Name = "GRIDTOTEST" };
			grid.AddRows (new GridLength (1, GridUnitType.Auto), new GridLength (50), new GridLength (1, GridUnitType.Star));
			grid.AddColumns (new GridLength (1, GridUnitType.Auto), new GridLength (50), new GridLength (1, GridUnitType.Star));
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements, c [0], c [1], c [3], c [4], c [6], c [2], c [5], c [6], c [7], c [8]);
				grid.CheckMeasureSizes ("#2", new Size (inf, inf), new Size (50, inf), new Size (400, inf),
											  new Size (inf, 50), new Size (50, 50), new Size (400, 50),
											  new Size (inf, 400), new Size (50, 400), new Size (400, 400));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder2 ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyContentControl star = ContentControlWithChild ();
			MyContentControl pixel = ContentControlWithChild ();
			MyContentControl auto = ContentControlWithChild ();

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), GridLength.Auto, new GridLength (30, GridUnitType.Pixel), new GridLength (30, GridUnitType.Pixel));
			grid.AddColumns (new GridLength (50, GridUnitType.Pixel));

			CreateAsyncTest (grid,
				() => {
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);

				}, () => {
					InSameOrder ("#1", grid.MeasuredElements, auto, pixel, star);
					grid.CheckMeasureSizes ("#a", new Size (50, 390), new Size (50, inf), new Size (50, 30));
					grid.CheckRowHeights ("#b", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#2", grid.MeasuredElements, pixel, auto, star);
					grid.CheckMeasureSizes ("#c", new Size (50, 390), new Size (50, 30), new Size (50, inf));
					grid.CheckRowHeights ("#d", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#3", grid.MeasuredElements, pixel, auto, star);
					grid.CheckMeasureSizes ("#e", new Size (50, 30), new Size (50, 390), new Size (50, inf));
					grid.CheckRowHeights ("#f", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#4", grid.MeasuredElements, pixel, auto, star);
					grid.CheckMeasureSizes ("#g", new Size (50, 30), new Size (50, inf), new Size (50, 390));
					grid.CheckRowHeights ("#h", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#5", grid.MeasuredElements, auto, pixel, star);
					grid.CheckMeasureSizes ("#i", new Size (50, inf), new Size (50, 30),new Size (50, 390));
					grid.CheckRowHeights ("#j", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#6", grid.MeasuredElements, auto, pixel, star);
					grid.CheckMeasureSizes ("#k", new Size (50, inf), new Size (50, 390), new Size (50, 30));
					grid.CheckRowHeights ("#l", 390, 50, 30, 30);
				}
			);
		}

		void InSameOrder (string message, IList collection, params object [] objects)
		{
			for (int i = 0; i < collection.Count; i++)
				Assert.AreSame (collection [i], objects [i], message + "." + i);
		}
	}

	class MyGrid : Grid
	{
		public List<MyContentControl> ArrangedElements = new List<MyContentControl> ();
		public List<MyContentControl> MeasuredElements = new List<MyContentControl> ();

		public void Reset ()
		{
			ArrangedElements.Clear ();
			MeasuredElements.Clear ();
		}
	}
}
