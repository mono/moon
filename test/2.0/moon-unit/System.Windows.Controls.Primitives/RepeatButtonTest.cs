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
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls.Primitives {

	[TestClass]
	public partial class RepeatButtonTest {

		[TestMethod]
		public void Delay ()
		{
			RepeatButton rb = new RepeatButton ();
			Assert.Throws<ArgumentException> (delegate {
				rb.Delay = -1;
			}, "negative");
			Assert.AreEqual (-1, rb.Delay, "changed after exception");

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
			Assert.AreEqual (-1, rb.Delay, "changed after exception");

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
			Assert.AreEqual (-1, rb.Interval, "changed after exception negative");

			Assert.Throws<ArgumentException> (delegate {
				rb.Interval = 0;
			}, "0");
			Assert.AreEqual (0, rb.Interval, "changed after exception 0");

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
			Assert.AreEqual (-1, rb.Interval, "changed after exception negative");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.IntervalProperty, 0);
			}, "0");
			Assert.AreEqual (0, rb.Interval, "changed after exception 0");

			rb.SetValue (RepeatButton.IntervalProperty, Int32.MaxValue);
			Assert.AreEqual (Int32.MaxValue, rb.Interval, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RepeatButton.IntervalProperty, null);
			}, "non-int");
			Assert.AreEqual (Int32.MaxValue, rb.Interval, "unchanged");
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			RepeatButton rb = new RepeatButton ();
			rb.OnApplyTemplate ();
			ControlTest.CheckDefaultMethods (rb);
		}

		[TestMethod]
		public void CheckReadOnlyProperties ()
		{
			// <quote>There are some read-only dependency properties that are part
			// of the Silverlight 2 API, but these rely on internal support.</quote>
			// http://msdn.microsoft.com/en-us/library/cc903923(VS.95).aspx
			ButtonBaseTest.ReadOnlyProperties (new RepeatButton ());
		}

		[TestMethod]
		public void CheckReadOnlyXaml ()
		{
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsFocused=""true""/>
</Canvas>");
			}, "IsFocused/True");

			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsFocused=""false""/>
</Canvas>");
			}, "IsFocused/False/Default");

			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsMouseOver=""true""/>
</Canvas>");
			}, "IsMouseOver/True");
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsMouseOver=""false""/>
</Canvas>");
			}, "IsMouseOver/False/Default");

			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsPressed=""true""/>
</Canvas>");
			}, "IsPressed/True");
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton IsPressed=""false""/>
</Canvas>");
			}, "IsPressed/False/Default");
		}
	}
}
