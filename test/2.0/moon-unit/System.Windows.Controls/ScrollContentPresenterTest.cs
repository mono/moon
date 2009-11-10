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
		[MoonlightBug]
		public void MeasureTest_ExtraSize_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

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
		public void MeasureTest_LessSize_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideResult, "#d");

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
		[MoonlightBug]
		public void MeasureTest_LessSize_LargerArrange_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 100, 100));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideResult, "#d");

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
		public void MeasureTest_LessSize_SmallerArrange_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (25, 25));
			presenter.Arrange (new Rect (0, 0, 15, 15));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (25, 25), child.MeasureOverrideResult, "#d");

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
		[MoonlightBug]
		public void MeasureTest_ExtraSize_LargerArrange_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 150, 150));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

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
		public void MeasureTest_ExtraSize_SmallerArrange_WithOwner ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 25, 25));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

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
		[MoonlightBug]
		public void MeasureTest_ExtraSize_SmallerArrange_WithOwner2 ()
		{
			var child = ContentControlWithChild ();
			var presenter = new ScrollContentPresenter {
				ScrollOwner = new ScrollViewer (),
				Content = child
			};

			presenter.Measure (new Size (100, 100));
			presenter.Arrange (new Rect (0, 0, 75, 75));

			Assert.IsTrue (child.IsMeasured, "#a");
			Assert.IsTrue (child.IsArranged, "#b");

			Assert.AreEqual (new Size (100, 100), child.MeasureOverrideArg, "#c");
			Assert.AreEqual (new Size (50, 50), child.MeasureOverrideResult, "#d");

			Assert.AreEqual (new Size (75, 75), child.ArrangeOverrideArg, "#e");
			Assert.AreEqual (new Size (75, 75), child.ArrangeOverrideArg, "#f");

			Assert.AreEqual (75, presenter.ActualHeight, "#1");
			Assert.AreEqual (75, presenter.ActualWidth, "#2");
			Assert.AreEqual (50, presenter.ExtentHeight, "#3");
			Assert.AreEqual (50, presenter.ExtentWidth, "#4");
			Assert.AreEqual (75, presenter.ViewportHeight, "#5");
			Assert.AreEqual (75, presenter.ViewportWidth, "#6");
			Assert.AreEqual (new Size (75, 75), presenter.RenderSize, "#7");
		}
	}
}
