//
// Unit tests for PatternInterface
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
using System.Collections.Generic;
using System.Windows.Automation.Peers;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class PatternInterfaceTest {

		[TestMethod]
		public void EnumerationValuesTest()
		{
			Assert.AreEqual (0,  (int) PatternInterface.Invoke, "PatternInterface.Invoke");
			Assert.AreEqual (1,  (int) PatternInterface.Selection, "PatternInterface.Selection");
			Assert.AreEqual (2,  (int) PatternInterface.Value, "PatternInterface.Value");
			Assert.AreEqual (3,  (int) PatternInterface.RangeValue, "PatternInterface.RangeValue");
			Assert.AreEqual (4,  (int) PatternInterface.Scroll, "PatternInterface.Scroll");
			Assert.AreEqual (5,  (int) PatternInterface.ScrollItem, "PatternInterface.ScrollItem");
			Assert.AreEqual (6,  (int) PatternInterface.ExpandCollapse, "PatternInterface.ExpandCollapse");
			Assert.AreEqual (7,  (int) PatternInterface.Grid, "PatternInterface.Grid");
			Assert.AreEqual (8,  (int) PatternInterface.GridItem, "PatternInterface.GridItem");
			Assert.AreEqual (9,  (int) PatternInterface.MultipleView, "PatternInterface.MultipleView");
			Assert.AreEqual (10, (int) PatternInterface.Window, "PatternInterface.Window");
			Assert.AreEqual (11, (int) PatternInterface.SelectionItem, "PatternInterface.SelectionItem");
			Assert.AreEqual (12, (int) PatternInterface.Dock, "PatternInterface.Dock");
			Assert.AreEqual (13, (int) PatternInterface.Table, "PatternInterface.Table");
			Assert.AreEqual (14, (int) PatternInterface.TableItem, "PatternInterface.TableItem");
			Assert.AreEqual (15, (int) PatternInterface.Toggle, "PatternInterface.Toggle");
			Assert.AreEqual (16, (int) PatternInterface.Transform, "PatternInterface.Transform");
		}
	}
}
