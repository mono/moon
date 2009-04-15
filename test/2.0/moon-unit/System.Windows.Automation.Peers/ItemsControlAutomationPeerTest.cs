//
// Unit tests for ItemsControlAutomationPeer
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
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ItemsControlAutomationPeerTest {

		public class ItemsControlAutomationPeerPoker : ItemsControlAutomationPeer {

			public ItemsControlAutomationPeerPoker (ItemsControl items) :
				base (items)
			{
			}

			public List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}
		}

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new ItemsControlAutomationPeerPoker (null);
			});
		}

		[TestMethod]
		public void GetPattern ()
		{
			ItemsControlAutomationPeerPoker icap = new ItemsControlAutomationPeerPoker (new ItemsControl ());

			Assert.IsNull (icap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (icap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (icap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (icap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (icap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (icap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (icap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (icap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (icap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (icap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (icap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (icap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (icap.GetPattern (PatternInterface.Window), "Window");
		}
	}
}
