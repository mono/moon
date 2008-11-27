//
// RangeBase Unit Tests
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
	public class RangeBaseTest {

		public class ConcreteRange : RangeBase {

			public double OldMaximum;
			public double NewMaximum;
			public double OldMinimum;
			public double NewMinimum;
			public double OldValue;
			public double NewValue;

			protected override void OnMaximumChanged (double oldMaximum, double newMaximum)
			{
				OldMaximum = oldMaximum;
				NewMaximum = newMaximum;
				base.OnMaximumChanged (oldMaximum, newMaximum);
			}

			protected override void OnMinimumChanged (double oldMinimum, double newMinimum)
			{
				OldMinimum = oldMinimum;
				NewMinimum = newMinimum;
				base.OnMinimumChanged (oldMinimum, newMinimum);
			}

			protected override void OnValueChanged (double oldValue, double newValue)
			{
				OldValue = oldValue;
				NewValue = newValue;
				base.OnValueChanged (oldValue, newValue);
			}
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ConcreteRange rb = new ConcreteRange ();
			// default properties on RangeBase
			Assert.AreEqual (0.1, rb.SmallChange, "SmallChange");
			Assert.AreEqual (1.0, rb.LargeChange, "LargeChange");
			Assert.AreEqual (1.0, rb.Maximum, "Maximum");
			Assert.AreEqual (0.0, rb.Minimum, "Minimum");
			Assert.AreEqual (0.0, rb.Value, "IsFocused");

			// default properties on Control...
			ControlTest.CheckDefaultProperties (rb);
		}

		[TestMethod]
		public void ToStringTest ()
		{
			ConcreteRange rb = new ConcreteRange ();
			Assert.AreEqual ("MoonTest.System.Windows.Controls.Primitives.RangeBaseTest+ConcreteRange Minimum:0 Maximum:1 Value:0", rb.ToString (), "ToString");
		}

		[TestMethod]
		public void OnMethods ()
		{
			ConcreteRange rb = new ConcreteRange ();

			rb.Maximum = 0.5;
			Assert.AreEqual (1.0, rb.OldMaximum, "OldMaximum");
			Assert.AreEqual (rb.Maximum, rb.NewMaximum, "NewMaximum");

			rb.Minimum = 0.5;
			Assert.AreEqual (0.0, rb.OldMinimum, "OldMinimum");
			Assert.AreEqual (rb.Minimum, rb.NewMinimum, "NewMinimum");

			rb.Value = 0.5;
			Assert.AreEqual (0.0, rb.OldValue, "OldValue");
			Assert.AreEqual (rb.Value, rb.NewValue, "NewValue");
		}

		[TestMethod]
		public void Events ()
		{
			ConcreteRange rb = new ConcreteRange ();
			double old = rb.Value;
			rb.ValueChanged += delegate (object sender, RoutedPropertyChangedEventArgs<double> e) {
				Assert.AreSame (rb, sender, "sender");
				Assert.IsNull (e.OriginalSource, "OriginalSource");
				Assert.AreEqual (rb.Value, e.NewValue, "NewValue");
				Assert.AreEqual (old, e.OldValue, "OldValue");
			};
			rb.Value = 1.0;
		}
	}
}
