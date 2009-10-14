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
using System.Linq;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Collections;
using System.Text;

namespace MoonTest.System.Windows.Controls
{
	public partial class GridTest
	{
		static readonly double inf = double.PositiveInfinity;

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

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (1, GridUnitType.Auto), new GridLength (50), new GridLength (1, GridUnitType.Star));
			grid.AddColumns (new GridLength (1, GridUnitType.Auto), new GridLength (50), new GridLength (1, GridUnitType.Star));
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [0], c [1], c [3], c [4], c [6], c [2], c [5], c [6], c [7], c [8]);
				grid.CheckMeasureArgs ("#2", new Size (inf, inf), new Size (50, inf), new Size (inf, 50),
											  new Size (50, 50), new Size (inf, inf), new Size (400, inf),
											  new Size (400, 50), new Size (inf, 400), new Size (50, 400), new Size (400, 400));
				grid.ReverseChildren ();
				grid.Reset ();
			}, () => {
				InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [4], c [5], c [7], c [8], c [2], c [3], c [6], c [2], c [0], c [1]);
				grid.CheckMeasureArgs ("#4", new Size (50, 50), new Size (inf, 50), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (400, 50),
											  new Size (400, inf), new Size (inf, 400), new Size (400, 400), new Size (50, 400));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder2 ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (50), new GridLength (1, GridUnitType.Auto), new GridLength (1, GridUnitType.Star));
			grid.AddColumns (new GridLength (50), new GridLength (1, GridUnitType.Auto), new GridLength (1, GridUnitType.Star));
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [0], c [1], c [3], c [4], c [7], c [2], c [5], c [7], c [6], c [8]);
				grid.CheckMeasureArgs ("#2", new Size (50, 50), new Size (inf, 50), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (400, 50),
											  new Size (400, inf), new Size (inf, 400), new Size (50, 400), new Size (400, 400));
				grid.ReverseChildren ();
				grid.Reset ();
			}, () => {
				InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [4], c [5], c [7], c [8], c [1], c [3], c [6], c [1], c [0], c [2]);
				grid.CheckMeasureArgs ("#4", new Size (inf, inf), new Size (50, inf), new Size (inf, 50),
											  new Size (50, 50), new Size (inf, inf), new Size (400, inf),
											  new Size (400, 50), new Size (inf, 400), new Size (400, 400), new Size (50, 400));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder3 ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (50), new GridLength (1, GridUnitType.Auto));
			grid.AddColumns (new GridLength (1, GridUnitType.Star), new GridLength (50), new GridLength (1, GridUnitType.Auto));
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [4], c [5], c [7], c [8], c [2], c [3], c [6], c [2], c [0], c [1]);
				grid.CheckMeasureArgs ("#2", new Size (50, 50), new Size (inf, 50), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (400, 50),
											  new Size (400, inf), new Size (inf, 400), new Size (400, 400), new Size (50, 400));
				grid.ReverseChildren ();
				grid.Reset ();
			}, () => {
				InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [0], c [1], c [3], c [4], c [6], c [2], c [5], c [6], c [7], c [8]);
				grid.CheckMeasureArgs ("#4", new Size (inf, inf), new Size (50, inf), new Size (inf, 50),
											  new Size (50, 50), new Size (inf, inf), new Size (400, inf),
											  new Size (400, 50), new Size (inf, 400), new Size (50, 400), new Size (400, 400));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder4 ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Auto), new GridLength (50));
			grid.AddColumns (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Auto), new GridLength (50));
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [4], c [5], c [7], c [8], c [1], c [3], c [6], c [1], c [0], c [2]);
				grid.CheckMeasureArgs ("#2", new Size (inf, inf), new Size (50, inf), new Size (inf, 50),
											  new Size (50, 50), new Size (inf, inf), new Size (400, inf),
											  new Size (400, 50), new Size (inf, 400), new Size (400, 400), new Size (50, 400));
				grid.ReverseChildren ();
				grid.Reset ();
			}, () => {
				InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [0], c [1], c [3], c [4], c [7], c [2], c [5], c [7], c [6], c [8]);
				grid.CheckMeasureArgs ("#4", new Size (50, 50), new Size (inf, 50), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (400, 50),
											  new Size (400, inf), new Size (inf, 400), new Size (50, 400), new Size (400, 400));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder6 ()
		{
			TestPanel.Width = 500;
			TestPanel.Height = 500;

			MyGrid grid = new MyGrid ();
			grid.AddRows (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Auto), new GridLength (50), new GridLength (1, GridUnitType.Star), GridLength.Auto);
			grid.AddColumns (new GridLength (1, GridUnitType.Star), new GridLength (1, GridUnitType.Auto), new GridLength (1, GridUnitType.Star), new GridLength (50), GridLength.Auto);
			PresentationFrameworkCollection<UIElement> c = grid.Children;
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 5; j++)
					grid.AddChild (ContentControlWithChild (), i, j, 1, 1);

			CreateAsyncTest (grid, () => {
				InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [5], c [6], c [7], c [9], c [10], c [11], c [13], c [14], c [15],
																									 c [1], c [3], c [4], c [8], c [12], c [1], c [3], c [0], c [2], c [13]);
				grid.CheckMeasureArgs ("#2", new Size (inf, inf), new Size (50, inf), new Size (inf, inf),new Size (inf, 50),
											  new Size (50, 50), new Size (inf, 50), new Size (inf, inf), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (inf, inf), new Size (350, inf),
											  new Size (350, 50), new Size (350, inf), new Size (inf, 350), new Size (inf, 350),
											  new Size (350, 350), new Size (50, 350));
				grid.ReverseChildren ();
				grid.Reset ();
			}, () => {
				InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), c [0], c [1], c [2], c [4], c [5], c [6], c [8], c [9], c [10],
																									 c [12], c [14], c [3], c [7], c [11], c [12], c [14], c[13], c[15]);
				grid.CheckMeasureArgs ("#4", new Size (inf, inf), new Size (50, inf), new Size (inf, inf), new Size (inf, 50),
											  new Size (50, 50), new Size (inf, 50), new Size (inf, inf), new Size (50, inf),
											  new Size (inf, inf), new Size (inf, inf), new Size (inf, inf), new Size (350, inf),
											  new Size (350, 50), new Size (350, inf), new Size (inf, 350), new Size (inf, 350),
											  new Size (50, 350), new Size (350, 350));
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void MeasureOrder5 ()
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
					InSameOrder ("#1", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), auto, pixel, star);
					grid.CheckMeasureArgs ("#a", new Size (50, inf), new Size (50, 30), new Size (50, 390));
					grid.CheckRowHeights ("#b", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#2", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), pixel, auto, star);
					grid.CheckMeasureArgs ("#c", new Size (50, 30), new Size (50, inf), new Size (50, 390));
					grid.CheckRowHeights ("#d", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#3", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), pixel, auto, star);
					grid.CheckMeasureArgs ("#e", new Size (50, 30), new Size (50, inf), new Size (50, 390));
					grid.CheckRowHeights ("#f", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#4", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), pixel, auto, star);
					grid.CheckMeasureArgs ("#g", new Size (50, 30), new Size (50, inf), new Size (50, 390));
					grid.CheckRowHeights ("#h", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#5", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), auto, pixel, star);
					grid.CheckMeasureArgs ("#i", new Size (50, inf), new Size (50, 30), new Size (50, 390));
					grid.CheckRowHeights ("#j", 390, 50, 30, 30);

					grid.Children.Clear ();
					grid.AddChild (auto, 1, 0, 1, 1);
					grid.AddChild (star, 0, 0, 1, 1);
					grid.AddChild (pixel, 2, 0, 1, 1);
					grid.Reset ();
				}, () => {
					InSameOrder ("#6", grid.MeasuredElements.Select (keypair => keypair.Key).ToArray (), auto, pixel, star);
					grid.CheckMeasureArgs ("#k", new Size (50, inf), new Size (50, 30), new Size (50, 390));
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
		public List<KeyValuePair<MyContentControl, Size>> ArrangedElements = new List<KeyValuePair<MyContentControl, Size>> ();
		public List<KeyValuePair<MyContentControl, Size>> MeasuredElements = new List<KeyValuePair<MyContentControl, Size>> ();

		public void Reset ()
		{
			ArrangedElements.Clear ();
			MeasuredElements.Clear ();
		}

		public void ReverseChildren ()
		{
			List<UIElement> children = new List<UIElement> (Children);
			children.Reverse ();
			Children.Clear ();
			children.ForEach (Children.Add);
		}
	}
}