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
using System.Windows.Markup;
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class UserControlTest {

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

		[TestMethod]
		[MoonlightBug]
		public void DefaultStyleKeyTest_NotWorking ()
		{
			Assert.Throws (delegate { DefaultStyleKey_TypeClass tc = new DefaultStyleKey_TypeClass (); },
				       typeof (ArgumentException), "1");
			Assert.Throws (delegate { DefaultStyleKey_TypeClass2 tc = new DefaultStyleKey_TypeClass2 (); },
				       typeof (ArgumentException), "2");
			Assert.Throws (delegate { DefaultStyleKey_DifferentTypeClass dtc = new DefaultStyleKey_DifferentTypeClass (); },
				       typeof (InvalidOperationException), "4");
		}

		public void DefaultStyleKeyTest_Working ()
		{
			Assert.Throws (delegate { DefaultStyleKey_NonDOTypeClass ndotc = new DefaultStyleKey_NonDOTypeClass (); },
				       typeof (ArgumentException), "5");
			Assert.Throws (delegate { DefaultStyleKey_NonTypeClass ntc = new DefaultStyleKey_NonTypeClass (); },
				       typeof (ArgumentException), "6");
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
		public void DefaultProperties ()
		{
			UserControlPoker uc = new UserControlPoker ();
			Assert.IsNull (uc.Content_);
			uc.Content_ = null;
			// TabStop default to false for UserControl
			ControlTest.CheckDefaultProperties (uc, false);
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			UserControlPoker uc = new UserControlPoker ();
			ControlTest.CheckDefaultMethods (uc);
		}
	}
}
