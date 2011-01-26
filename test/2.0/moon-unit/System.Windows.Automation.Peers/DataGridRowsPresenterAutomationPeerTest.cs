//
// Unit tests for DataGridRowsPresenterAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class DataGridRowsPresenterAutomationPeerTest : FrameworkElementAutomationPeerTest {

		// All these methods are tested in AllTests
		public override void GetChildren () {}
		public override void IsContentElement () {}
		public override void GetClassName () {}
		public override void ContentTest () {}

		[TestMethod]
		[MinRuntimeVersion (4)] // NRE in SL2, ANE in SL4
		public override void Null ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DataGridRowHeaderAutomationPeer (null);
			});
		}

		[TestMethod]
		public void AllTests ()
		{
			AutomationPeer peer
				= FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			// GetClassName
			Assert.AreEqual ("DataGridRowsPresenter", peer.GetClassName (), "GetClassNameCore");

			// IsContent
			Assert.IsFalse (peer.IsContentElement (), "IsContentElement");

			// GetChildren and Content
			List<AutomationPeer> children = peer.GetChildren ();
			Assert.IsNotNull (children, "#0");
			Assert.AreEqual (0, children.Count, "#1");
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new DataGridRowsPresenter ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// Can't subclass DataGridRowsPresenterAutomationPeer
			return null;
		}

	}

}
