//
// Unit tests for ItemAutomationPeer
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
	public class ItemAutomationPeerTest {

		public class ItemAutomationPeerPoker : ItemAutomationPeer {

			public ItemAutomationPeerPoker (UIElement items) :
				base (items)
			{
			}

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public string GetItemTypeCore_ ()
			{
				return base.GetItemTypeCore ();
			}

			public ItemsControlAutomationPeer ItemsControlAutomationPeer_ {
				get { return base.ItemsControlAutomationPeer; }
			}

			public object Item_ {
				get { return base.Item; }
			}
		}

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new ItemAutomationPeerPoker (null);
			});
		}

		[TestMethod]
		public void CtorWantsAContentControl ()
		{
			// needs a ContentControl even if the ctor accept an UIElement
			Assert.Throws<InvalidCastException> (delegate {
				new ItemAutomationPeerPoker (new Slider ());
			});
		}

		[TestMethod]
		public void Protected ()
		{
			ContentControl cc = new ContentControl ();
			ItemAutomationPeerPoker iap = new ItemAutomationPeerPoker (cc);
			Assert.AreEqual (String.Empty, iap.GetNameCore_ (), "GetNameCore");
			Assert.AreEqual (String.Empty, iap.GetItemTypeCore_ (), "GetItemTypeCore");
			Assert.IsNull (iap.ItemsControlAutomationPeer_, "ItemsControlAutomationPeer");
			Assert.IsTrue (Object.ReferenceEquals (cc, iap.Item_), "Item");
		}
	}
}
