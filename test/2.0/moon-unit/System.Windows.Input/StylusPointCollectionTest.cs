//
// Unit tests for System.Windows.Input.StylusPointCollection
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
using System.Windows.Input;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Markup;

namespace MoonTest.System.Windows.Input {

	[TestClass]
	public partial class StylusPointCollectionTest {

		[TestMethod]
		public void StylusPoint_DefaultValues ()
		{
			var p = new StylusPoint ();
			Assert.AreEqual (0, p.X, "#1");
			Assert.AreEqual (0, p.Y, "#2");
			Assert.AreEqual (0, p.PressureFactor, "#3");

			p = new StylusPoint (1, 2);
			Assert.AreEqual (1, p.X, "#4");
			Assert.AreEqual (2, p.Y, "#5");
			Assert.AreEqual (0.5, p.PressureFactor, "#6");
		}

		[TestMethod]
		public void StylusPoint_NegativeValues ()
		{
			var p = new StylusPoint { X = -1000, Y = -1000 };
			Assert.AreEqual (-1000, p.X, "#1");
			Assert.AreEqual (-1000, p.Y, "#2");
			Assert.Throws<ArgumentOutOfRangeException> (() => p.PressureFactor = -1, "#3");
		}

		[TestMethod]
		public void StylusPoint_LargeValues ()
		{
			var p = new StylusPoint { X = 1000, Y = 1000 };
			Assert.AreEqual (1000, p.X, "#1");
			Assert.AreEqual (1000, p.Y, "#2");
			Assert.Throws<ArgumentOutOfRangeException> (() => p.PressureFactor = 1.1f, "#3");
		}

		public void ModifyValuesInManaged ()
		{
			var c = (StylusPointCollection) XamlReader.Load (@"
<StylusPointCollection	xmlns=""http://schemas.microsoft.com/client/2007""
						xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<StylusPoint X=""1"" Y=""2"" /> 
</StylusPointCollection>
");
			var expected = new StylusPoint(1, 2);
			Assert.AreEqual (1, c.Count, "#1");
			Assert.AreEqual (expected, c [0], "#2");

			var p = c [0];
			p.X = 10;
			Assert.AreEqual (expected, c [0], "#3");
		}

		[TestMethod]
		[MinRuntimeVersion (4)]
		[MoonlightBug ("Actual value is '[StylusPoint: PressureFactor=0, X=1, Y=2]' while the expected value was '[StylusPoint: PressureFactor=0.5, X=1, Y=2]'. #2")]
		public void ModifyValuesInManaged_sl4 ()
		{
			ModifyValuesInManaged ();
		}

		[TestMethod]
		[MaxRuntimeVersion (3)]
		public void ModifyValuesInManaged_sl3 ()
		{
			ModifyValuesInManaged ();
		}

		[TestMethod]
		public void Add_StylusPoint_Validation ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			StylusPoint sp = new StylusPoint ();
			spc.Add (sp);
			Assert.AreEqual (1, spc.Count, "Count");
		}

		[TestMethod]
		public void Add_StylusPoint_Collection_Validation ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			// not an ArgumentNullException (or NRE)
			Assert.Throws<ArgumentException> (delegate {
				spc.Add ((StylusPointCollection) null);
			}, "null");
		}

		[TestMethod]
		public void Add_StylusPointCollection_Empty ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			StylusPointCollection child = new StylusPointCollection ();
			spc.Add (child);
			// twice
			spc.Add (child);
			// self
			spc.Add (spc);
			Assert.AreEqual (0, spc.Count, "Count-3");
		}

		[TestMethod]
		[MoonlightBug ("missing duplicate check")]
		public void Add_StylusPointCollection_MultipleTimes ()
		{
			StylusPointCollection spc = new StylusPointCollection ();
			StylusPointCollection child = new StylusPointCollection ();
			child.Add (new StylusPoint (1, 2));
			spc.Add (child);
			Assert.AreEqual (1, spc.Count, "Count-1");
			// twice
			Assert.Throws<InvalidOperationException> (delegate {
				spc.Add (child);
			}, "child already added");
			Assert.AreEqual (1, spc.Count, "Count-2");
			Assert.Throws<InvalidOperationException> (delegate {
				spc.Add (spc);
			}, "child already added/2");
			Assert.AreEqual (1, spc.Count, "Count-3");
		}
	}
}

