//
// ScrollContentPresenter Unit Tests
//
// Author:
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
// Copyright 2009 Novell, Inc. (http://www.novell.com)
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
	[TestClass]
	public partial class ScrollContentPresenterTest : SilverlightTest
	{
		T Find<T> (FrameworkElement o, string name)
			where T : FrameworkElement
		{
			if (o == null)
				return null;
			T target = o.FindName (name) as T;
			if (target != null)
				return target;
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount (o); i++) {
				object result = Find<T> (VisualTreeHelper.GetChild (o, i) as FrameworkElement, name);
				if (result is T)
					return (T) result;
			}
			return null;
		}

		static readonly Size Infinity = new Size (double.PositiveInfinity, double.PositiveInfinity);

		MyContentControl ContentControlWithChild ()
		{
			return new MyContentControl {
				Content = new Rectangle {
					Width = 50,
					Height = 50
				}
			};
		}

		[TestMethod]
		public void MeasureTest_NoOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				Content = child
			};

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (presenter));
			presenter.Measure (new Size (100, 100));
			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (presenter));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (100, presenter.ActualHeight, "#1");
			Assert.AreEqual (100, presenter.ActualWidth, "#2");
			Assert.AreEqual (0, presenter.ExtentHeight, "#3");
			Assert.AreEqual (0, presenter.ExtentWidth, "#4");
			Assert.AreEqual (0, presenter.ViewportHeight, "#5");
			Assert.AreEqual (0, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (100, 100), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void MeasureOnly_ExtraSize ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsFalse (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (50, 50), presenter.DesiredSize, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
		}

		[TestMethod]
		public void MeasureOnly_ExtraSize_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsFalse (child.IsArranged, "#b");

			Assert.AreEqual (Infinity, child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (50, 50), presenter.DesiredSize, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
		}

		[TestMethod]
		public void MeasureOnly_LessSize ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsFalse (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (25, 25), presenter.DesiredSize, "#2");
			Assert.AreEqual (25, presenter.ExtentHeight, "#3");
			Assert.AreEqual (25, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
		}

		[TestMethod]
		public void MeasureOnly_LessSize_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsFalse (child.IsArranged, "#b");

			Assert.AreEqual (Infinity, child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (25, 25), presenter.DesiredSize, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
		}

		[TestMethod]
		public void Measure_ExtraSize ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideResult, "#f");

			Assert.AreEqual (100, presenter.ActualHeight, "#1");
			Assert.AreEqual (100, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (100, 100), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_ExtraSize_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (100, presenter.ActualHeight, "#1");
			Assert.AreEqual (100, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (100, 100), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (25, 25), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (25, presenter.ActualHeight, "#1");
			Assert.AreEqual (25, presenter.ActualWidth, "#2");
			Assert.AreEqual (25, presenter.ExtentHeight, "#3");
			Assert.AreEqual (25, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (25, 25), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (25, presenter.ActualHeight, "#1");
			Assert.AreEqual (25, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (25, 25), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize_LargerArrange ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (100, presenter.ActualHeight, "#1");
			Assert.AreEqual (100, presenter.ActualWidth, "#2");
			Assert.AreEqual (25, presenter.ExtentHeight, "#3");
			Assert.AreEqual (25, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (100, 100), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize_LargerArrange_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (100, presenter.ActualHeight, "#1");
			Assert.AreEqual (100, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (100, presenter.ViewportHeight, "#5");
			Assert.AreEqual (100, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (100, 100), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize_SmallerArrange ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 15, 15));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (25, presenter.ActualHeight, "#1");
			Assert.AreEqual (25, presenter.ActualWidth, "#2");
			Assert.AreEqual (25, presenter.ExtentHeight, "#3");
			Assert.AreEqual (25, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (25, 25), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_LessSize_SmallerArrange_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 15, 15));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (25, presenter.ActualHeight, "#1");
			Assert.AreEqual (25, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (25, presenter.ViewportHeight, "#5");
			Assert.AreEqual (25, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (25, 25), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_ExtraSize_LargerArrange ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 150, 150));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (150, 150), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (150, 150), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (150, presenter.ActualHeight, "#1");
			Assert.AreEqual (150, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (150, presenter.ViewportHeight, "#5");
			Assert.AreEqual (150, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (150, 150), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_ExtraSize_LargerArrange_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 150, 150));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (150, 150), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (150, 150), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (150, presenter.ActualHeight, "#1");
			Assert.AreEqual (150, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (150, presenter.ViewportHeight, "#5");
			Assert.AreEqual (150, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (150, 150), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_ExtraSize_SmallerArrange ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (50, presenter.ActualHeight, "#1");
			Assert.AreEqual (50, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (50, presenter.ViewportHeight, "#5");
			Assert.AreEqual (50, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (50, 50), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void Measure_ExtraSize_SmallerArrange_Scrollable ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (50, 50), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (50, presenter.ActualHeight, "#1");
			Assert.AreEqual (50, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (50, presenter.ViewportHeight, "#5");
			Assert.AreEqual (50, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (50, 50), presenter.RenderSize, "#7");
		}

		[TestMethod]
		public void ArrangeDoesNotUpdateOwner ()
		{
			ScrollViewer owner = new ScrollViewer ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = owner,
				Content = ContentControlWithChild ()
			};

			Assert.AreEqual (0, owner.ExtentHeight, "#1");
			Assert.AreEqual (0, owner.ExtentWidth, "#2");
			Assert.AreEqual (0, owner.ViewportHeight, "#3");
			Assert.AreEqual (0, owner.ViewportHeight, "#4");

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 50, 50));

			Assert.AreEqual (0, owner.ExtentHeight, "#5");
			Assert.AreEqual (0, owner.ExtentWidth, "#6");
			Assert.AreEqual (0, owner.ViewportHeight, "#7");
			Assert.AreEqual (0, owner.ViewportHeight, "#8");
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("Arrange isn't actually arranging here, so the test can't be performed")]
		public void ArrangeUpdatesOwner ()
		{
			ScrollViewer viewer = new ScrollViewer {
				HorizontalScrollBarVisibility = ScrollBarVisibility.Visible,
				VerticalScrollBarVisibility = ScrollBarVisibility.Visible
			};
			viewer.Content = ContentControlWithChild ();

			TestPanel.Width = 100;
			TestPanel.Height = 100;
			CreateAsyncTest (viewer, () => {
				ScrollContentPresenter p = Find<ScrollContentPresenter> (viewer, "ScrollContentPresenter");
				Assert.AreEqual (new Size (50, 50), new Size (p.ExtentWidth, p.ExtentHeight), "#1");
				Assert.AreEqual (new Size (73, 73), new Size (p.ViewportWidth, p.ViewportHeight), "#2");
				Assert.AreEqual (new Size (50, 50), new Size (viewer.ExtentWidth, viewer.ExtentHeight), "#3");
				Assert.AreEqual (new Size (73, 73), new Size (viewer.ViewportWidth, viewer.ViewportHeight), "#4");

				p.Measure (new Size (40, 40));
				p.Arrange (new Rect (0, 0, 22, 22));

				Assert.AreEqual (new Size (50, 50), new Size (p.ExtentWidth, p.ExtentHeight), "#5");
				Assert.AreEqual (new Size (17, 17), new Size (p.ViewportWidth, p.ViewportHeight), "#6");
				Assert.AreEqual (new Size (50, 50), new Size (viewer.ExtentWidth, viewer.ExtentHeight), "#7");
				Assert.AreEqual (new Size (17, 17), new Size (viewer.ViewportWidth, viewer.ViewportHeight), "#8");
			});
		}

		[TestMethod]
		public void MeasureDoesNotUpdateOwner ()
		{
			ScrollViewer owner = new ScrollViewer ();
			var presenter = new ScrollContentPresenter {
				CanHorizontallyScroll = true,
				CanVerticallyScroll = true,
				ScrollOwner = owner,
				Content = ContentControlWithChild ()
			};

			Assert.AreEqual (0, owner.ExtentHeight, "#1");
			Assert.AreEqual (0, owner.ExtentWidth, "#2");
			Assert.AreEqual (0, owner.ViewportHeight, "#3");
			Assert.AreEqual (0, owner.ViewportHeight, "#4");

			presenter.Measure (new Size (25, 25));

			Assert.AreEqual (0, owner.ExtentHeight, "#5");
			Assert.AreEqual (0, owner.ExtentWidth, "#6");
			Assert.AreEqual (0, owner.ViewportHeight, "#7");
			Assert.AreEqual (0, owner.ViewportHeight, "#8");
		}

		[TestMethod]
		[Asynchronous]
		public void MeasureUpdatesOwner ()
		{
			ScrollViewer viewer = new ScrollViewer {
				HorizontalScrollBarVisibility = ScrollBarVisibility.Visible,
				VerticalScrollBarVisibility = ScrollBarVisibility.Visible
			};
			viewer.Content = ContentControlWithChild ();

			TestPanel.Width = 100;
			TestPanel.Height = 100;
			CreateAsyncTest (viewer, () => {
				ScrollContentPresenter p = Find<ScrollContentPresenter> (viewer, "ScrollContentPresenter");
				Assert.AreEqual (new Size (50, 50), new Size (p.ExtentWidth, p.ExtentHeight), "#1");
				Assert.AreEqual (new Size (73, 73), new Size (p.ViewportWidth, p.ViewportHeight), "#2");
				Assert.AreEqual (new Size (50, 50), new Size (viewer.ExtentWidth, viewer.ExtentHeight), "#3");
				Assert.AreEqual (new Size (73, 73), new Size (viewer.ViewportWidth, viewer.ViewportHeight), "#4");

				p.CanHorizontallyScroll = true;
				p.CanVerticallyScroll = true;

				p.Measure (new Size (25, 25));

				Assert.AreEqual (new Size (50, 50), new Size (p.ExtentWidth, p.ExtentHeight), "#5");
				Assert.AreEqual (new Size (17, 17), new Size (p.ViewportWidth, p.ViewportHeight), "#6");
				Assert.AreEqual (new Size (50, 50), new Size (viewer.ExtentWidth, viewer.ExtentHeight), "#7");
				Assert.AreEqual (new Size (17, 17), new Size (viewer.ViewportWidth, viewer.ViewportHeight), "#8");
			});
		}
	}
}
