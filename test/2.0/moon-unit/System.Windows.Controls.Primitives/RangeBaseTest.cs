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
using Mono.Moonlight.UnitTesting;

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
		public void SmallChange ()
		{
			ConcreteRange rb = new ConcreteRange ();
			Assert.Throws<ArgumentException> (delegate {
				rb.SmallChange = -1.0;
			}, "negative");

			rb.SmallChange = 0.0d;
			Assert.AreEqual (0.0d, rb.SmallChange, "0.0");

			rb.SmallChange = Double.MaxValue;
			Assert.AreEqual (Double.MaxValue, rb.SmallChange, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SmallChange = Double.NaN;
			}, "NAN");

			Assert.Throws<ArgumentException> (delegate {
				rb.SmallChange = Double.PositiveInfinity;
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				rb.SmallChange = Double.NegativeInfinity;
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void LargeChange ()
		{
			ConcreteRange rb = new ConcreteRange ();
			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.LargeChangeProperty, -0.1);
			}, "negative");

			rb.SetValue (RangeBase.LargeChangeProperty, 0.0);
			Assert.AreEqual (0.0d, rb.GetValue (RangeBase.LargeChangeProperty), "0.0");

			rb.SetValue (RangeBase.LargeChangeProperty, Double.MaxValue);
			Assert.AreEqual (Double.MaxValue, rb.GetValue (RangeBase.LargeChangeProperty), "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.LargeChangeProperty, Double.NaN);
			}, "NAN");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.LargeChangeProperty, Double.PositiveInfinity);
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.LargeChangeProperty, Double.NegativeInfinity);
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void Minimum ()
		{
			ConcreteRange rb = new ConcreteRange ();
			rb.Minimum = Double.MinValue;
			Assert.AreEqual (Double.MinValue, rb.Minimum, "MinValue");

			rb.Minimum = 0.0d;
			Assert.AreEqual (0.0d, rb.Minimum, "0.0");

			rb.Minimum = Double.MaxValue;
			Assert.AreEqual (Double.MaxValue, rb.Minimum, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.Minimum = Double.NaN;
			}, "NAN");

			Assert.Throws<ArgumentException> (delegate {
				rb.Minimum = Double.PositiveInfinity;
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				rb.Minimum = Double.NegativeInfinity;
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void Maximum ()
		{
			ConcreteRange rb = new ConcreteRange ();
			rb.SetValue (RangeBase.MaximumProperty, Double.MinValue);
			// Maximum cannot be under Minimum
			Assert.AreEqual (0.0d, rb.GetValue (RangeBase.MaximumProperty), "MinValue");
			rb.Minimum = Double.MinValue;
			rb.SetValue (RangeBase.MaximumProperty, Double.MinValue);
			Assert.AreEqual (Double.MinValue, rb.GetValue (RangeBase.MaximumProperty), "MinValue");

			rb.SetValue (RangeBase.MaximumProperty, 0.0);
			Assert.AreEqual (0.0d, rb.GetValue (RangeBase.MaximumProperty), "0.0");

			rb.SetValue (RangeBase.MaximumProperty, Double.MaxValue);
			Assert.AreEqual (Double.MaxValue, rb.GetValue (RangeBase.MaximumProperty), "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.MaximumProperty, Double.NaN);
			}, "NAN");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.MaximumProperty, Double.PositiveInfinity);
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				rb.SetValue (RangeBase.MaximumProperty, Double.NegativeInfinity);
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void Value ()
		{
			ConcreteRange rb = new ConcreteRange ();
			rb.Value = Double.MinValue;
			// Value cannot be under Minimum
			Assert.AreEqual (rb.Minimum, rb.Value, "MinValue/0");

			Assert.AreEqual (1.0d, rb.Maximum, "Maximum==1");
			rb.Minimum = -1000;

			// doing the above changes Maximum from 1.0 to 0.0 ?!?
			Assert.AreEqual (0.0d, rb.Maximum, "Maximum==0");

			rb.Value = Double.MinValue;
			Assert.AreEqual (rb.Minimum, rb.Value, "MinValue/-1000");

			rb.Minimum = Double.MinValue;
			rb.Value = Double.MinValue;
			Assert.AreEqual (Double.MinValue, rb.Value, "MinValue");

			rb.Value = 0.0d;
			Assert.AreEqual (0.0d, rb.Value, "0.0");

			rb.Value = Double.MaxValue;
			Assert.AreEqual (0.0d, rb.Value, "MaxValue/0");

			rb.Maximum = 1000;
			rb.Value = Double.MaxValue;
			Assert.AreEqual (rb.Maximum, rb.Value, "MaxValue/1000");

			rb.Maximum = Double.MaxValue;
			rb.Value = Double.MaxValue;
			Assert.AreEqual (Double.MaxValue, rb.Value, "MaxValue");

			Assert.Throws<ArgumentException> (delegate {
				rb.Value = Double.NaN;
			}, "NAN");

			Assert.Throws<ArgumentException> (delegate {
				rb.Value = Double.PositiveInfinity;
			}, "PositiveInfinity");

			Assert.Throws<ArgumentException> (delegate {
				rb.Value = Double.NegativeInfinity;
			}, "NegativeInfinity");
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ConcreteRange rb = new ConcreteRange ();
			ControlTest.CheckDefaultMethods (rb);
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
