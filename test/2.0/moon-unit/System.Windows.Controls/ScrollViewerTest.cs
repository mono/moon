//
// Unit tests for ScrollViewer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using System.Windows.Media;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ScrollViewerTest : SilverlightTest {

		public class TestScrollViewer
		{
			public ScrollBar HorizontalScrollbar {
				get { return Find <ScrollBar> (ScrollViewer, "HorizontalScrollBar"); }
			}

			public Thumb HorizontalThumb {
				get { return Find<Thumb> (HorizontalScrollbar, "HorizontalThumb"); }
			}

			public ScrollViewer ScrollViewer {
				get; private set;
			}

			public ScrollContentPresenter ScrollPresenter {
				get { return Find <ScrollContentPresenter> (ScrollViewer, "ScrollContentPresenter"); }
			}

			public ScrollBar VerticalScrollbar {
				get { return Find<ScrollBar> (ScrollViewer, "VerticalScrollBar"); }
			}

			public Thumb VerticalThumb {
				get { return Find<Thumb> (VerticalScrollbar, "VerticalThumb"); }
			}

			public TestScrollViewer ()
			{
				ScrollViewer = new ScrollViewer ();
			}

			public TestScrollViewer (int width, int height)
			{
				ScrollViewer = new ScrollViewer { Width = width, Height = height };
			}

			T Find<T> (DependencyObject o, string name)
				where T : FrameworkElement
			{
				for (int i = 0; i < VisualTreeHelper.GetChildrenCount (o); i++) {
					FrameworkElement e = VisualTreeHelper.GetChild (o, i) as FrameworkElement;
					if (e == null)
						continue;
					if (e.Name == name)
						return e as T;

					object child = Find<T> (e, name);
					if (child != null)
						return (T)child;
				}
				return null;
			}
		}

		[TestMethod]
		[Asynchronous]
		public void AfterRender ()
		{
			ScrollViewer viewer = new ScrollViewer ();
			CreateAsyncTest (viewer, delegate {
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (viewer), "#1");
				Border border = (Border)VisualTreeHelper.GetChild (viewer, 0);

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (border), "#2");
				Grid grid = (Grid) VisualTreeHelper.GetChild (border, 0);


				Assert.AreEqual (4, VisualTreeHelper.GetChildrenCount (grid), "#3");
				Assert.IsTrue (grid.Children [0] is ScrollContentPresenter, "#4");
				Assert.IsTrue (grid.Children [1] is Rectangle, "#5");
				Assert.IsTrue (grid.Children [2] is ScrollBar, "#6");
				Assert.IsTrue (grid.Children [3] is ScrollBar, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ExtentHeightWidthTest ()
		{
			TestScrollViewer t = new TestScrollViewer ();
			t.ScrollViewer.Content = new Canvas {
				Width = 60,
				Height = 100,
				Background = new SolidColorBrush (Colors.Red)
			};
			CreateAsyncTest (t.ScrollViewer,
				() => t.ScrollViewer.ApplyTemplate (),
				() => Assert.AreEqual (100, t.ScrollViewer.ExtentHeight, "#1"),
				() => Assert.AreEqual (60, t.ScrollViewer.ExtentWidth, "#2")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ExtentHeightWidthTest2 ()
		{
			TestScrollViewer t = new TestScrollViewer (100, 100);
			t.ScrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
			t.ScrollViewer.Content = new Canvas {
				Width = 60,
				Height = 100,
				Background = new SolidColorBrush (Colors.Red)
			};
			CreateAsyncTest (t.ScrollViewer,
				() => t.ScrollViewer.ApplyTemplate (),
				() => Assert.AreEqual (100, t.ScrollViewer.ExtentHeight, "#1"),
				() => Assert.AreEqual (60, t.ScrollViewer.ExtentWidth, "#2")
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("This fails on the bots but passes locally. Be careful removing this")]
		public void ExtentHeightWidthTest3 ()
		{
			StackPanel panel = new StackPanel ();
			panel.Children.Add (new Rectangle { Width = 100, Height = 60 });
			panel.Children.Add (new Rectangle { Width = 110, Height = 70});

			TestScrollViewer t = new TestScrollViewer ();
			t.ScrollViewer.Content = panel;

			CreateAsyncTest (t.ScrollViewer,
				() => t.ScrollViewer.ApplyTemplate (),
				() => {
					// Check that the desired size is what is used for the Extents
					Assert.AreEqual (130, t.ScrollViewer.ExtentHeight, "#1");
					Assert.AreEqual (110, t.ScrollViewer.ExtentWidth, "#2");
					Assert.AreEqual (new Size (110, 130), panel.DesiredSize, "#3");

					// And that the Presenter shows the same limits
					Assert.AreEqual (130, t.ScrollPresenter.ExtentHeight, "#4");
					Assert.AreEqual (110, t.ScrollPresenter.ExtentWidth, "#5");
				}, () => {
					// Resize the element and check the changes
					panel.Width = 1000;
				}, () => {
					Assert.AreEqual (1000, t.ScrollViewer.ExtentWidth, "#6");
					Assert.AreEqual (1000, t.ScrollPresenter.ExtentWidth, "#6");
				}
			);
		}

		[TestMethod]
		public void ReadOnlyProperties ()
		{
			ScrollViewer cp = new ScrollViewer ();
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ComputedHorizontalScrollBarVisibilityProperty, Visibility.Collapsed);
			}, "ComputedHorizontalScrollBarVisibilityProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ComputedVerticalScrollBarVisibilityProperty, Visibility.Collapsed);
			}, "ComputedVerticalScrollBarVisibility");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ExtentHeightProperty, 1.0);
			}, "ExtentHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ExtentWidthProperty, 1.0);
			}, "ExtentWidthProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.HorizontalOffsetProperty, 1.0);
			}, "HorizontalOffsetProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ScrollableHeightProperty, 1.0);
			}, "ScrollableHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ScrollableWidthProperty, 1.0);
			}, "ScrollableWidthProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.VerticalOffsetProperty, 1.0);
			}, "VerticalOffsetProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ViewportHeightProperty, 1.0);
			}, "ViewportHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ViewportWidthProperty, 1.0);
			}, "ViewportWidthProperty");
		}

		[TestMethod]
		public void ReadOnlyProperties_BadValues ()
		{
			ScrollViewer cp = new ScrollViewer ();

			// normally bad value types would throw ArgumentException but not for read-only properties
			// and (the important part is) that is not possible using the PropertyChangedCallback to
			// simulate a read-only property (like ScrollViewer from beta1 was doing)

			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ComputedHorizontalScrollBarVisibilityProperty, true);
			}, "ComputedHorizontalScrollBarVisibilityProperty-wrong-value-type");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ComputedVerticalScrollBarVisibilityProperty, true);
			}, "ComputedVerticalScrollBarVisibility");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ExtentHeightProperty, true);
			}, "ExtentHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ExtentWidthProperty, true);
			}, "ExtentWidthProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.HorizontalOffsetProperty, true);
			}, "HorizontalOffsetProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ScrollableHeightProperty, true);
			}, "ScrollableHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ScrollableWidthProperty, true);
			}, "ScrollableWidthProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.VerticalOffsetProperty, true);
			}, "VerticalOffsetProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ViewportHeightProperty, true);
			}, "ViewportHeightProperty");
			Assert.Throws<InvalidOperationException> (delegate {
				cp.SetValue (ScrollViewer.ViewportWidthProperty, true);
			}, "ViewportWidthProperty");
		}

		[TestMethod]
		public void StaticMethods ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				ScrollViewer.GetHorizontalScrollBarVisibility (null);
			}, "GetHorizontalScrollBarVisibility");
			Assert.Throws<ArgumentNullException> (delegate {
				ScrollViewer.GetVerticalScrollBarVisibility (null);
			}, "GetVerticalScrollBarVisibility");

			Assert.Throws<ArgumentNullException> (delegate {
				ScrollViewer.SetHorizontalScrollBarVisibility (null, ScrollBarVisibility.Auto);
			}, "SetHorizontalScrollBarVisibility-null");
			Assert.Throws<ArgumentNullException> (delegate {
				ScrollViewer.SetVerticalScrollBarVisibility (null, ScrollBarVisibility.Auto);
			}, "SetVerticalScrollBarVisibility-null");

			ScrollViewer sv = new ScrollViewer ();
			ScrollBarVisibility bad = (ScrollBarVisibility) Int32.MinValue;

			ScrollViewer.SetHorizontalScrollBarVisibility (sv, bad);
			Assert.AreEqual (bad, sv.HorizontalScrollBarVisibility, "HorizontalScrollBarVisibility/Bad");

			ScrollViewer.SetVerticalScrollBarVisibility (sv, bad);
			Assert.AreEqual (bad, sv.VerticalScrollBarVisibility, "VerticalScrollBarVisibility/Bad");

			RowDefinition rd = new RowDefinition ();
			ScrollViewer.SetHorizontalScrollBarVisibility (rd, bad);
			ScrollViewer.SetVerticalScrollBarVisibility (rd, bad);
		}

		[TestMethod]
		[Asynchronous]
		public void ThumbResizes ()
		{
			Rectangle r  = new Rectangle { Width = 700, Height = 500, Fill = new SolidColorBrush (Colors.Red) };
			TestScrollViewer t = new TestScrollViewer (200, 200);
			t.ScrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;
			t.ScrollViewer.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;

			t.ScrollViewer.Content = r;

			CreateAsyncTest (t.ScrollViewer,
				() => t.ScrollViewer.ApplyTemplate (),
				() => {
					Assert.IsBetween(50, 51, t.VerticalThumb.Height, "#1");
					Assert.IsBetween (36, 37, t.HorizontalThumb.Width, "#2");
				}, () => {
					r.Width = 400;
					r.Height = 250;
				}, () => {
					Assert.IsBetween (101, 102, t.VerticalThumb.Height, "#3");
					Assert.IsBetween (63, 64, t.HorizontalThumb.Width, "#4");
				}
			);
		}
	}
}
