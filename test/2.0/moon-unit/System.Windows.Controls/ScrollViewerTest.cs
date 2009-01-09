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

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ScrollViewerTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			ScrollViewer cp = new ScrollViewer ();
			Assert.AreEqual (Visibility.Visible, cp.ComputedHorizontalScrollBarVisibility, "ComputedHorizontalScrollBarVisibility");
			Assert.AreEqual (Visibility.Visible, cp.ComputedVerticalScrollBarVisibility, "ComputedVerticalScrollBarVisibility");
			Assert.AreEqual (0.0, cp.ExtentHeight, "ExtentHeight");
			Assert.AreEqual (0.0, cp.ExtentWidth, "ExtentWidth");
			Assert.AreEqual (0.0, cp.HorizontalOffset, "HorizontalOffset");
			Assert.AreEqual (0.0, cp.VerticalOffset, "VerticalOffset");
			Assert.AreEqual (ScrollBarVisibility.Disabled, cp.HorizontalScrollBarVisibility, "HorizontalScrollBarVisibility");
			Assert.AreEqual (ScrollBarVisibility.Disabled, cp.VerticalScrollBarVisibility, "VerticalScrollBarVisibility");
			Assert.AreEqual (0.0, cp.ViewportHeight, "ViewportHeight");
			Assert.AreEqual (0.0, cp.ViewportWidth, "ViewportWidth");

			// default properties on ContentControl
			ContentControlTest.CheckDefaultProperties (cp);
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
	}
}
