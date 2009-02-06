//
// Unit tests for RepeatBehavior
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
using System.Windows.Media.Animation;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media.Animation {

	[TestClass]
	public class RepeatBehaviorTest {

		[TestMethod]
		public void Defaults ()
		{
			RepeatBehavior rb = new RepeatBehavior ();
			Assert.IsTrue (rb.HasCount, "HasCount");
			Assert.IsFalse (rb.HasDuration, "HasCount");

			Assert.AreEqual (0.0, rb.Count, "Count");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (TimeSpan.Zero, rb.Duration, "Duration");
			}, "Duration");

			Assert.AreEqual ("0x", rb.ToString (), "ToString");
		}

		[TestMethod]
		public void DoubleConstructor ()
		{
			RepeatBehavior rb = new RepeatBehavior (1.0);
			Assert.IsTrue (rb.HasCount, "HasCount");
			Assert.IsFalse (rb.HasDuration, "HasCount");

			Assert.AreEqual (1.0, rb.Count, "Count");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (TimeSpan.Zero, rb.Duration, "Duration");
			}, "Duration");

			Assert.AreEqual ("1x", rb.ToString (), "ToString");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (-0.1);
			}, "Negative");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (Double.MinValue);
			}, "MinValue");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (Double.NegativeInfinity);
			}, "NegativeInfinity");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (Double.PositiveInfinity);
			}, "PositiveInfinity");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (Double.NaN);
			}, "NaN");

			rb = new RepeatBehavior (Double.MaxValue);
			Assert.IsTrue (rb.HasCount, "HasCount-Max");
			Assert.IsFalse (rb.HasDuration, "HasDuration-Max");
			Assert.AreEqual (Double.MaxValue, rb.Count, "Count-Max");
			Assert.AreEqual ("1.79769313486232E+308x", rb.ToString (), "ToString");
			Assert.IsFalse (rb.Equals (RepeatBehavior.Forever), "Max!=Forever");
		}

		[TestMethod]
		public void TimeSpanConstructor ()
		{
			RepeatBehavior rb = new RepeatBehavior (TimeSpan.Zero);
			Assert.IsFalse (rb.HasCount, "HasCount");
			Assert.IsTrue (rb.HasDuration, "HasDuration");

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (0, rb.Count, "Count");
			}, "Count");
			Assert.AreEqual (TimeSpan.Zero, rb.Duration, "Duration");

			Assert.AreEqual (TimeSpan.Zero.ToString (), rb.ToString (), "ToString");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (new TimeSpan (-1));
			}, "Negative");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				rb = new RepeatBehavior (TimeSpan.MinValue);
			}, "MinValue");

			rb = new RepeatBehavior (TimeSpan.MaxValue);
			Assert.IsFalse (rb.HasCount, "HasCount-Max");
			Assert.IsTrue (rb.HasDuration, "HasDuration-Max");
			Assert.AreEqual (TimeSpan.MaxValue, rb.Duration, "Duration");
			Assert.AreEqual ("10675199.02:48:05.4775807", rb.ToString (), "ToString");
			Assert.IsFalse (rb.Equals (RepeatBehavior.Forever), "Max!=Forever");
		}

		[TestMethod]
		public void Forever ()
		{
			RepeatBehavior rb = RepeatBehavior.Forever;
			Assert.IsFalse (rb.HasCount, "HasCount");
			Assert.IsFalse (rb.HasDuration, "HasCount");

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (0, rb.Count, "Count");
			}, "Count");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (TimeSpan.Zero, rb.Duration, "Duration");
			}, "Duration");

			Assert.AreEqual ("Forever", rb.ToString (), "ToString");

			Assert.AreEqual (rb, RepeatBehavior.Forever, "AreEqual");
			Assert.IsTrue (rb.Equals (RepeatBehavior.Forever), "Equals(RepeatBehavior)");
			Assert.IsTrue (RepeatBehavior.Forever.Equals ((object)rb), "Equals(object)");
			Assert.IsTrue (RepeatBehavior.Equals (rb, RepeatBehavior.Forever), "RepeatBehavior.Equals");
			Assert.IsFalse (Object.ReferenceEquals (rb, RepeatBehavior.Forever), "Same");
		}

		class RepeatBehaviorFormatter : IFormatProvider, ICustomFormatter {

			public object GetFormat (Type formatType)
			{
				return (formatType == typeof (ICustomFormatter)) ? this : null;
			}

			public string Format (string format, object arg, IFormatProvider formatProvider)
			{
				CallCount++;
				Assert.AreEqual (this, formatProvider, "formatProvider");
				if (arg.Equals ('x'))
					return "#";

				if (arg is double) {
					double d = (double) arg;
					if ((d == 2) || (d == Double.MaxValue))
						return String.Format ("[{0}]", d);
					Assert.Fail (d.ToString ());
					return null;
				} else if (arg is TimeSpan) {
					TimeSpan ts = (TimeSpan) arg;
					return String.Format ("[{0}]", ts);
				}

				Assert.Fail (arg.GetType ().ToString ());
				return null;
			}

			static public int CallCount = 0;
		}

		[TestMethod]
		public void RepeatBehaviorCountToStringIFormatProvider ()
		{
			RepeatBehavior rb = new RepeatBehavior (2);
			RepeatBehaviorFormatter.CallCount = 0;
			Assert.AreEqual ("2x", rb.ToString (null), "null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("[2]x", rb.ToString (new RepeatBehaviorFormatter ()), "RepeatBehaviorFormatter");
			// 1 time: one per double (1), no call for 'x'
			Assert.AreEqual (1, RepeatBehaviorFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void RepeatBehaviorCountToStringIFormattable ()
		{
			RepeatBehavior rb = new RepeatBehavior (Double.MaxValue);
			RepeatBehaviorFormatter.CallCount = 0;
			IFormattable f = (rb as IFormattable);
			Assert.AreEqual ("1.79769313486232E+308x", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("[1.79769313486232E+308]x", f.ToString (null, new RepeatBehaviorFormatter ()), "null,RepeatBehaviorFormatter");
			Assert.AreEqual (1, RepeatBehaviorFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("1.797693e+308x", f.ToString ("e", null), "e,null");
			Assert.AreEqual (1, RepeatBehaviorFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("[1.79769313486232E+308]x", f.ToString (String.Empty, new RepeatBehaviorFormatter ()), "Empty,RepeatBehaviorFormatter");
			Assert.AreEqual (2, RepeatBehaviorFormatter.CallCount, "CallCount-d");
		}

		[TestMethod]
		public void RepeatBehaviorDurationToStringIFormatProvider ()
		{
			RepeatBehavior rb = new RepeatBehavior (TimeSpan.MaxValue);
			RepeatBehaviorFormatter.CallCount = 0;
			Assert.AreEqual ("10675199.02:48:05.4775807", rb.ToString (null), "null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("10675199.02:48:05.4775807", rb.ToString (new RepeatBehaviorFormatter ()), "RepeatBehaviorFormatter");
			// unused
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void RepeatBehaviorDurationToStringIFormattable ()
		{
			RepeatBehavior rb = new RepeatBehavior (TimeSpan.Zero);
			RepeatBehaviorFormatter.CallCount = 0;
			IFormattable f = (rb as IFormattable);
			Assert.AreEqual ("00:00:00", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("00:00:00", f.ToString (null, new RepeatBehaviorFormatter ()), "null,RepeatBehaviorFormatter");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("00:00:00", f.ToString ("e", null), "e,null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("00:00:00", f.ToString (String.Empty, new RepeatBehaviorFormatter ()), "Empty,RepeatBehaviorFormatter");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-d");
		}

		[TestMethod]
		public void RepeatBehaviorForeverToStringIFormatProvider ()
		{
			RepeatBehavior rb = RepeatBehavior.Forever;
			RepeatBehaviorFormatter.CallCount = 0;
			Assert.AreEqual ("Forever", rb.ToString (null), "null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("Forever", rb.ToString (new RepeatBehaviorFormatter ()), "RepeatBehaviorFormatter");
			// unused
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void RepeatBehaviorForeverToStringIFormattable ()
		{
			RepeatBehavior rb = RepeatBehavior.Forever;
			RepeatBehaviorFormatter.CallCount = 0;
			IFormattable f = (rb as IFormattable);
			Assert.AreEqual ("Forever", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("Forever", f.ToString (null, new RepeatBehaviorFormatter ()), "null,RepeatBehaviorFormatter");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("Forever", f.ToString ("e", null), "e,null");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("Forever", f.ToString (String.Empty, new RepeatBehaviorFormatter ()), "Empty,RepeatBehaviorFormatter");
			Assert.AreEqual (0, RepeatBehaviorFormatter.CallCount, "CallCount-d");
		}
	}
}
