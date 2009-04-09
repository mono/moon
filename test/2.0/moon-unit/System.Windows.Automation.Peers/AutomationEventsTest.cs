//
// Unit tests for AutomationEvents
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
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class AutomationEventsTest {

		[TestMethod]
		public void EnumerationValuesTest ()
		{
			Assert.AreEqual (0,  (int) AutomationEvents.ToolTipOpened, "AutomationEvents.ToolTipOpened");
			Assert.AreEqual (1,  (int) AutomationEvents.ToolTipClosed, "AutomationEvents.ToolTipClosed");
			Assert.AreEqual (2,  (int) AutomationEvents.MenuOpened, "AutomationEvents.MenuOpened");
			Assert.AreEqual (3,  (int) AutomationEvents.MenuClosed, "AutomationEvents.MenuClosed");
			Assert.AreEqual (4,  (int) AutomationEvents.AutomationFocusChanged, "AutomationEvents.AutomationFocusChanged");
			Assert.AreEqual (5,  (int) AutomationEvents.InvokePatternOnInvoked, "AutomationEvents.InvokePatternOnInvoked");
			Assert.AreEqual (6,  (int) AutomationEvents.SelectionItemPatternOnElementAddedToSelection, "AutomationEvents.SelectionItemPatternOnElementAddedToSelection");
			Assert.AreEqual (7,  (int) AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection, "AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection");
			Assert.AreEqual (8,  (int) AutomationEvents.SelectionItemPatternOnElementSelected, "AutomationEvents.SelectionItemPatternOnElementSelected");
			Assert.AreEqual (9,  (int) AutomationEvents.SelectionPatternOnInvalidated, "AutomationEvents.SelectionPatternOnInvalidated");
			Assert.AreEqual (10, (int) AutomationEvents.TextPatternOnTextSelectionChanged, "AutomationEvents.TextPatternOnTextSelectionChanged");
			Assert.AreEqual (11, (int) AutomationEvents.TextPatternOnTextChanged, "AutomationEvents.TextPatternOnTextChanged");
			Assert.AreEqual (12, (int) AutomationEvents.AsyncContentLoaded, "AutomationEvents.AsyncContentLoaded");
			Assert.AreEqual (13, (int) AutomationEvents.PropertyChanged, "AutomationEvents.PropertyChanged");
			Assert.AreEqual (14, (int) AutomationEvents.StructureChanged, "AutomationEvents.StructureChanged");
		}
	}
}
