//
// UserControl Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Windows.Controls.Primitives;
using System.Windows.Markup;
using System.Windows.Shapes;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class UserControlTest : SilverlightTest {
		class DefaultStyleKey_TypeClass : UserControl {
			public DefaultStyleKey_TypeClass ()
			{
				DefaultStyleKey = typeof (DefaultStyleKey_TypeClass);
			}
		}

		class DefaultStyleKey_TypeClass2 : UserControl {
			public DefaultStyleKey_TypeClass2 ()
			{
				DefaultStyleKey = typeof (UserControl);
			}
		}

		class DefaultStyleKey_DifferentTypeClass : UserControl {
			public DefaultStyleKey_DifferentTypeClass ()
			{
				DefaultStyleKey = typeof (Button);
			}
		}

		class DefaultStyleKey_NonDOTypeClass : UserControl {
			public DefaultStyleKey_NonDOTypeClass ()
			{
				DefaultStyleKey = typeof (string);
			}
		}

		class DefaultStyleKey_NonTypeClass : UserControl {
			public DefaultStyleKey_NonTypeClass ()
			{
				DefaultStyleKey = "hi";
			}
		}
		
		public void DefaultStyleKeyTest ()
		{
			Assert.Throws (delegate { DefaultStyleKey_NonDOTypeClass ndotc = new DefaultStyleKey_NonDOTypeClass (); },
				       typeof (ArgumentException), "5");
			Assert.Throws (delegate { DefaultStyleKey_NonTypeClass ntc = new DefaultStyleKey_NonTypeClass (); },
				       typeof (ArgumentException), "6");
			Assert.Throws (delegate { DefaultStyleKey_TypeClass tc = new DefaultStyleKey_TypeClass (); },
				       typeof (ArgumentException), "1");
			Assert.Throws (delegate { DefaultStyleKey_TypeClass2 tc = new DefaultStyleKey_TypeClass2 (); },
				       typeof (ArgumentException), "2");
			Assert.Throws (delegate { DefaultStyleKey_DifferentTypeClass dtc = new DefaultStyleKey_DifferentTypeClass (); },
				       typeof (InvalidOperationException), "4");
		}

		class UserControlPoker : UserControl {

			public void SetContent (UIElement ui)
			{
				Content = ui;
			}

			public UIElement Content_ {
				get { return base.Content; }
				set { base.Content = value; }
			}

			public Size MeasureResult = new Size (0,0);
			public Size MeasureArg = new Size (0,0);
			public Size ArrangeResult = new Size (0,0);
			public Size ArrangeArg = new Size (0,0);
			public Size BaseArrangeResult = new Size (0,0);
			public Size BaseMeasureResult = new Size (0,0);
			public event EventHandler Measuring;
			public event MeasureOverrideHandler Measured;
			public event EventHandler Arranging;
			public event ArrangeOverrideHandler Arranged;
			public delegate void ArrangeOverrideHandler (Size real);
			public delegate void MeasureOverrideHandler (Size real);

			protected override Size MeasureOverride (Size availableSize)
			{
				MeasureArg = availableSize;
				Tester.WriteLine (string.Format ("Panel available size is {0}", availableSize));

				if (Measuring != null)
					Measuring (this, EventArgs.Empty);

				BaseMeasureResult = base.MeasureOverride (availableSize);

				if (Measured != null)
					Measured (BaseMeasureResult);
				else 
					MeasureResult = BaseMeasureResult;

				return MeasureResult;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				ArrangeArg = finalSize;
				Tester.WriteLine (string.Format ("Panel final size is {0}", finalSize));

				if (Arranging != null)
					Arranging (this, EventArgs.Empty);

				BaseArrangeResult = base.ArrangeOverride (finalSize);
				
				if (Arranged != null)
					Arranged (BaseArrangeResult);
				else
					ArrangeResult = BaseArrangeResult;

				return ArrangeResult;

			}

			public Size InvokeMeasureOverride (Size availableSize)
			{
				return base.MeasureOverride (availableSize);
			}

			public Size InvokeArrangeOverride (Size finalSize)
			{
				return base.ArrangeOverride (finalSize);
			}
		}

		class PanelPoker : Panel {

			public void SetContent (UIElement ui)
			{
				Children.Clear ();
				Children.Add (ui);
			}

			public Size MeasureResult = new Size (0,0);
			public Size MeasureArg = new Size (0,0);
			public Size ArrangeResult = new Size (0,0);
			public Size ArrangeArg = new Size (0,0);
			public Size BaseArrangeResult = new Size (0,0);
			public Size BaseMeasureResult = new Size (0,0);
			public event EventHandler Measuring;
			public event MeasureOverrideHandler Measured;
			public event EventHandler Arranging;
			public event ArrangeOverrideHandler Arranged;
			public delegate void ArrangeOverrideHandler (Size real);
			public delegate void MeasureOverrideHandler (Size real);

			protected override Size MeasureOverride (Size availableSize)
			{
				MeasureArg = availableSize;
				Tester.WriteLine (string.Format ("Panel available size is {0}", availableSize));

				if (Measuring != null)
					Measuring (this, EventArgs.Empty);

				BaseMeasureResult = base.MeasureOverride (availableSize);

				if (Measured != null)
					Measured (BaseMeasureResult);
				else 
					MeasureResult = BaseMeasureResult;

				return MeasureResult;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				ArrangeArg = finalSize;
				Tester.WriteLine (string.Format ("Panel final size is {0}", finalSize));

				if (Arranging != null)
					Arranging (this, EventArgs.Empty);

				BaseArrangeResult = base.ArrangeOverride (finalSize);
				
				if (Arranged != null)
					Arranged (BaseArrangeResult);
				else
					ArrangeResult = BaseArrangeResult;

				return ArrangeResult;

			}

			public Size InvokeMeasureOverride (Size availableSize)
			{
				return base.MeasureOverride (availableSize);
			}

			public Size InvokeArrangeOverride (Size finalSize)
			{
				return base.ArrangeOverride (finalSize);
			}
		}

		[TestMethod]
		public void ClippingCanvasTest_notree ()
		{
			var mine = new UserControlPoker () { Width = 30, Height = 30 };
			var content = new Canvas () { Width = 50, Height = 50 };
			
			mine.SetContent (content);

			mine.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (new Size (30, 30), mine.MeasureArg, "MeasureArg");
			Assert.AreEqual (new Size (30, 30), mine.MeasureResult, "MeasureResult");
			Assert.AreEqual (new Size (30, 30), mine.DesiredSize, "poker Desired");
			Assert.AreEqual (new Size (30, 30), content.DesiredSize, "canvas desired");

			mine.Arrange (new Rect (0,0, mine.DesiredSize.Width, mine.DesiredSize.Height));

			Assert.AreEqual (new Size (30, 30), mine.ArrangeArg, "ArrangeArg");
			Assert.AreEqual (new Size (30, 30), mine.ArrangeResult, "ArrangeResult");
			Assert.AreEqual (new Size (50, 50), new Size (content.ActualWidth, content.ActualHeight), "content actual");
			Assert.AreEqual (new Size (50, 50), content.RenderSize, "content rendersize");
			Assert.AreEqual (new Rect (0,0,30,30), LayoutInformation.GetLayoutSlot (content), "content slot");
			Assert.IsNull (LayoutInformation.GetLayoutClip (content), "clip");
		}

		[TestMethod]
		[Asynchronous]
		public void ClippingCanvasTest ()
		{
			var mine = new UserControlPoker () { Width = 30, Height = 30 };
			var content = new Canvas () { Width = 50, Height = 50 };
			
			mine.SetContent (content);

			CreateAsyncTest (mine, () => {
					Assert.AreEqual (new Size (30, 30), mine.MeasureArg, "MeasureArg");
					Assert.AreEqual (new Size (30, 30), mine.MeasureResult, "MeasureResult");
					Assert.AreEqual (new Size (30, 30), mine.DesiredSize, "poker Desired");
					Assert.AreEqual (new Size (30, 30), content.DesiredSize, "canvas desired");
					
					Assert.AreEqual (new Size (30, 30), mine.ArrangeArg, "ArrangeArg");
					Assert.AreEqual (new Size (30, 30), mine.ArrangeResult, "ArrangeArg");
					Assert.AreEqual (new Size (50, 50), new Size (content.ActualWidth, content.ActualHeight), "content actual");
					Assert.AreEqual (new Size (30, 30), mine.RenderSize, "uc rendersize");
					Assert.AreEqual (new Size (50, 50), content.RenderSize, "content rendersize");
					Assert.AreEqual (new Rect (0,0,30,30), LayoutInformation.GetLayoutSlot (content), "content slot");
					Assert.IsNull (LayoutInformation.GetLayoutClip (content), "clip");
				}
				);
		}

		[TestMethod]
		[Asynchronous]
		public void ClippingUserControlTest ()
		{
			var mine = new UserControlPoker () { Width = 30, Height = 30 };
			var content = new UserControlPoker () { Width = 50, Height = 50 };
			
			mine.SetContent (content);

			content.Measured += (Size result) => {
				content.MeasureResult = new Size (0,0);
			};

			content.Arranged += (Size result) => {
				content.ArrangeResult = content.ArrangeArg;
			};

			CreateAsyncTest (mine, () => {
					Assert.AreEqual (new Size (30, 30), mine.MeasureArg, "MeasureArg");
					Assert.AreEqual (new Size (30, 30), mine.MeasureResult, "MeasureResult");
					Assert.AreEqual (new Size (30, 30), mine.DesiredSize, "parent Desired");
					Assert.AreEqual (new Size (30, 30), content.DesiredSize, "content desired");
					
					Assert.AreEqual (new Size (30, 30), mine.ArrangeArg, "ArrangeArg");
					Assert.AreEqual (new Size (30, 30), mine.ArrangeResult, "ArrangeArg");
					Assert.AreEqual (new Size (50, 50), new Size (content.ActualWidth, content.ActualHeight), "content actual");
					Assert.AreEqual (new Size (30, 30), mine.RenderSize, "parent rendersize");
					Assert.AreEqual (new Size (50, 50), content.RenderSize, "content rendersize");
					Assert.AreEqual (new Rect (0,0,30,30), LayoutInformation.GetLayoutSlot (content), "content slot");
					Assert.IsNotNull (LayoutInformation.GetLayoutClip (content), "clip");

					RectangleGeometry rect = LayoutInformation.GetLayoutClip (content) as RectangleGeometry;
					Assert.IsNotNull (rect);
					
					Assert.AreEqual (LayoutInformation.GetLayoutSlot (content), rect.Rect, "clip == slot");
				}
				);
		}

		[TestMethod]
		[Asynchronous]
		public void ClippingPanelTest ()
		{
			var mine = new UserControlPoker () { Width = 30, Height = 30 };
			var content = new PanelPoker () { Width = 50, Height = 50 };
			
			mine.SetContent (content);

			content.Measured += (Size result) => {
				content.MeasureResult = new Size (0,0);
			};

			content.Arranged += (Size result) => {
				content.ArrangeResult = content.ArrangeArg;
			};

			CreateAsyncTest (mine, () => {
					Assert.AreEqual (new Size (30, 30), mine.MeasureArg, "MeasureArg");
					Assert.AreEqual (new Size (30, 30), mine.MeasureResult, "MeasureResult");
					Assert.AreEqual (new Size (30, 30), mine.DesiredSize, "parent Desired");
					Assert.AreEqual (new Size (30, 30), content.DesiredSize, "content desired");
					
					Assert.AreEqual (new Size (30, 30), mine.ArrangeArg, "ArrangeArg");
					Assert.AreEqual (new Size (30, 30), mine.ArrangeResult, "ArrangeArg");
					Assert.AreEqual (new Size (50, 50), new Size (content.ActualWidth, content.ActualHeight), "content actual");
					Assert.AreEqual (new Size (30, 30), mine.RenderSize, "parent rendersize");
					Assert.AreEqual (new Size (50, 50), content.RenderSize, "content rendersize");
					Assert.AreEqual (new Rect (0,0,30,30), LayoutInformation.GetLayoutSlot (content), "content slot");
					Assert.IsNotNull (LayoutInformation.GetLayoutClip (content), "clip");

					RectangleGeometry rect = LayoutInformation.GetLayoutClip (content) as RectangleGeometry;
					Assert.IsNotNull (rect);
					
					Assert.AreEqual (LayoutInformation.GetLayoutSlot (content), rect.Rect, "clip == slot");
				}
				);
		}

		[TestMethod]
		public void ChildNameScope ()
		{
			UserControlPoker b = new UserControlPoker ();
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border>
    <Path x:Name=""foo"" Data=""F1 M 10,10 20,20 10,20"" Stroke=""Red""/>
  </Border>
</Canvas>");
			Assert.IsNotNull (c.FindName ("foo"), "c before");

			b.SetContent (c);

			Assert.IsNull (b.FindName ("foo"), "b after");
			Assert.IsNotNull (c.FindName ("foo"), "c after");
		}

		[TestMethod]
		public void DefaultDesiredSizeTest ()
		{
			UserControlPoker p = new UserControlPoker ();
			Assert.AreEqual (new Size (0, 0), p.DesiredSize);
		}

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
			UserControlPoker p = new UserControlPoker ();

			Size s = new Size (10, 10);

			p.Measure (s);

			Assert.AreEqual (new Size (0, 0), p.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMarginMeasureTest ()
		{
			UserControlPoker p = new UserControlPoker ();

			p.Margin = new Thickness (10);

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10, 10), p.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMinWidthMeasureTest1 ()
		{
			UserControlPoker p = new UserControlPoker ();

			p.MinWidth = 50;

			Size s = new Size (10, 10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Width);
		}

		[TestMethod]
		public void ChildlessMinWidthMeasureTest2 ()
		{
			UserControlPoker p = new UserControlPoker ();

			p.MinWidth = 5;

			Size s = new Size (10, 10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Width);
		}

		[TestMethod]
		public void ChildlessMinHeightMeasureTest1 ()
		{
			UserControlPoker p = new UserControlPoker ();

			p.MinHeight = 50;

			Size s = new Size (10, 10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Height);
		}

		[TestMethod]
		public void ChildlessMinHeightMeasureTest2 ()
		{
			UserControlPoker p = new UserControlPoker ();

			p.MinHeight = 5;

			Size s = new Size (10, 10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Height);
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			UserControlPoker p = new UserControlPoker ();
			Rectangle r = new Rectangle ();

			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10, 10), p.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest2 ()
		{
			UserControlPoker p = new UserControlPoker ();
			Rectangle r = new Rectangle ();

			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (50, 50), p.DesiredSize);
		}

		[TestMethod]
		public void ChildThicknessMeasureTest1 ()
		{
			UserControlPoker p = new UserControlPoker ();
			Rectangle r = new Rectangle ();

			p.Margin = new Thickness (5);
			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10, 10), p.DesiredSize);
		}

		[TestMethod]
		public void ChildThicknessMeasureTest2 ()
		{
			UserControlPoker p = new UserControlPoker ();
			Rectangle r = new Rectangle ();

			p.Margin = new Thickness (5);
			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (60, 60), p.DesiredSize);
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			UserControlPoker uc = new UserControlPoker ();
			ControlTest.CheckDefaultMethods (uc);
		}

		[TestMethod]
		public void Content_Null ()
		{
			UserControlPoker uc = new UserControlPoker ();
			// new value is null
			uc.Content_ = null;
			// old value is null
			uc.Content_ = new Rectangle (); 
		}
	}
}
