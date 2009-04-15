//
// Unit tests for RepeatButtonAutomationPeer
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
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class RepeatButtonAutomationPeerTest {

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new RepeatButtonAutomationPeer (null);
			});
		}

		[TestMethod]
		public void Public ()
		{
			RepeatButtonAutomationPeer rbap = new RepeatButtonAutomationPeer (new RepeatButton ());
			Assert.AreEqual (AutomationControlType.Button, rbap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual ("RepeatButton", rbap.GetClassName (), "GetClassName");

			Assert.IsNull (rbap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (rbap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (rbap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsTrue (Object.ReferenceEquals (rbap, rbap.GetPattern (PatternInterface.Invoke)), "Invoke");
			Assert.IsNull (rbap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (rbap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (rbap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (rbap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (rbap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Window), "Window");
		}

		[TestMethod]
		public void Invoke ()
		{
			IInvokeProvider ip = new RepeatButtonAutomationPeer (new RepeatButton ());
			ip.Invoke ();
		}

		public class RepeatButtonAutomationPeerPoker : RepeatButtonAutomationPeer {

			public RepeatButtonAutomationPeerPoker (RepeatButton owner)
				: base (owner)
			{
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}
		}

		[TestMethod]
		public void Protected ()
		{
			RepeatButtonAutomationPeerPoker rbap = new RepeatButtonAutomationPeerPoker (new RepeatButton ());
			Assert.AreEqual (AutomationControlType.Button, rbap.GetAutomationControlTypeCore_ (), "GetAutomationControlType");
			Assert.AreEqual ("RepeatButton", rbap.GetClassNameCore_ (), "GetClassName");
		}
	}
}
