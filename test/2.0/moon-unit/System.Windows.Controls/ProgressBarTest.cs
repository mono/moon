//
// ProgressBar Unit Tests
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
using MoonTest.System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ProgressBarTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			ProgressBar pb = new ProgressBar ();
			// default properties on ProgressBar
			Assert.IsFalse (pb.IsIndeterminate, "IsIndeterminate");

			// default properties on RangeBase...
			CheckDefaultProperties (pb);
		}

		static public void CheckDefaultProperties (RangeBase rb)
		{
			// default properties on RangeBase
			Assert.AreEqual (0.1, rb.SmallChange, "SmallChange");
			Assert.AreEqual (1.0, rb.LargeChange, "LargeChange");
			Assert.AreEqual (100.0, rb.Maximum, "Maximum");
			Assert.AreEqual (0.0, rb.Minimum, "Minimum");
			Assert.AreEqual (0.0, rb.Value, "IsFocused");
		}

		[TestMethod]
		public void Properties ()
		{
			Slider s = new Slider ();
			// default properties on Slider
			s.IsDirectionReversed = !s.IsDirectionReversed;

			Assert.AreEqual (Orientation.Horizontal, s.Orientation, "Orientation");
			s.Orientation = (Orientation) Int32.MaxValue;
			Assert.AreEqual ((Orientation) Int32.MaxValue, s.Orientation, "MaxValue");
		}

		[TestMethod]
		public void ToStringTest ()
		{
			ProgressBar pb = new ProgressBar ();
			Assert.AreEqual ("System.Windows.Controls.ProgressBar Minimum:0 Maximum:100 Value:0", pb.ToString (), "ToString");
		}
	}
}
