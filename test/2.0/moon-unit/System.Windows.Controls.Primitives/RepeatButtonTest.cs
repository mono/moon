//
// RepeatButton Unit Tests
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

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls.Primitives {

	[TestClass]
	public class RepeatButtonTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			RepeatButton rb = new RepeatButton ();
			// default properties on RepeatButton
			Assert.AreEqual (500, rb.Delay, "Delay");
			Assert.AreEqual (33, rb.Interval, "Interval");

			// default properties on Control...
			ControlTest.CheckDefaultProperties (rb);
		}

		[TestMethod]
		public void Delay ()
		{
			RepeatButton rb = new RepeatButton ();
			Assert.Throws<ArgumentException> (delegate {
				rb.Delay = -1;
			}, "negative");

			rb.Delay = 0;
			Assert.AreEqual (0, rb.Delay, "0");

			rb.Delay = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, rb.Delay, "MaxValue");
		}

		[TestMethod]
		public void Delay_SetValue ()
		{
			RepeatButton rb = new RepeatButton ();
			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.DelayProperty, -1);
			}, "negative");

			rb.SetValue (RepeatButton.DelayProperty, 0);
			Assert.AreEqual (0, rb.Delay, "0");

			rb.SetValue (RepeatButton.DelayProperty, Int32.MaxValue);
			Assert.AreEqual (Int32.MaxValue, rb.Delay, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.DelayProperty, null);
			}, "non-int");
			Assert.AreEqual (Int32.MaxValue, rb.Delay, "unchanged");
		}

		[TestMethod]
		public void Interval ()
		{
			RepeatButton rb = new RepeatButton ();
			Assert.Throws<ArgumentException> (delegate {
				rb.Interval = -1;
			}, "negative");

			Assert.Throws<ArgumentException> (delegate {
				rb.Interval = 0;
			}, "0");

			rb.Interval = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, rb.Interval, "MaxValue");
		}

		[TestMethod]
		public void Interval_SetValue ()
		{
			RepeatButton rb = new RepeatButton ();
			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.IntervalProperty, -1);
			}, "negative");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.IntervalProperty, 0);
			}, "0");

			rb.SetValue (RepeatButton.IntervalProperty, Int32.MaxValue);
			Assert.AreEqual (Int32.MaxValue, rb.Interval, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.IntervalProperty, null);
			}, "non-int");
			Assert.AreEqual (Int32.MaxValue, rb.Interval, "unchanged");
		}
	}
}
