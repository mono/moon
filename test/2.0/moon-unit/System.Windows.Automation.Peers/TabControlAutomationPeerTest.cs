//
// Unit tests for TabControlAutomationPeer
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
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class TabControlAutomationPeerTest : ItemsControlAutomationPeerTest {

		[TestMethod]
		public override void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new TabControlAutomationPeer (null);
			});
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Tab, peer.GetAutomationControlType (), "AutomationControlType");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("TabControl", peer.GetClassName (), "ClassName");
		}

		[TestMethod]
		public override void GetClickablePoint ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Point point = peer.GetClickablePoint ();
			Assert.IsTrue (double.IsNaN (point.X), "X");
			Assert.IsTrue (double.IsNaN (point.Y), "Y");
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TabControl tabControl = CreateConcreteFrameworkElement () as TabControl;
			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				Assert.IsFalse (peer.IsKeyboardFocusable (), "#0");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent ();
		}

		[TestMethod]
		public override void GetPattern ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Assert.IsNull (peer.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (peer.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (peer.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (peer.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (peer.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (peer.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (peer.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (peer.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (peer.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (peer.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (peer.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (peer.GetPattern (PatternInterface.Selection), "Selection");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			TabControl tabControl = CreateConcreteFrameworkElement () as TabControl;
			AutomationPeer peer = null;

			CreateAsyncTest (tabControl,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				tabControl.Items.Add (new TabItem ());
			},
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #1");
			},
			() => tabControl.Items.Add (new TabItem ()),
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #2");
				Assert.AreEqual (2, children.Count, "GetChildren.Count #2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void StructureChanged_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}
			AutomationPeer peer = null;
			TabControl tabControl = CreateConcreteFrameworkElement () as TabControl;

			CreateAsyncTest (tabControl,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement #0");
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "GetAutomationEventFrom #0");
			},
			() => {
				EventsManager.Instance.Reset ();
				tabControl.Items.Add (new TabItem () { Header = "Item 0" });
			},
			() => {
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #1");
			},
			() => {
				EventsManager.Instance.Reset ();
				tabControl.Items.RemoveAt (0);
			},
			() => {
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #2");
			});
		}

		// NOTE: No SelectionProvider UIA events raised by TabControlAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void ISelectionProvider_Methods ()
		{
			TabControl tabControl = new TabControl ();
			ISelectionProvider selectionProvider = null;
			AutomationPeer peer = null;

			CreateAsyncTest (tabControl,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				Assert.IsNotNull (peer, "#0");

				selectionProvider = (ISelectionProvider) peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider);
				Assert.IsNull (peer.GetChildren (), "#1");
			},
			() => {
				Assert.IsFalse (selectionProvider.CanSelectMultiple, "#2");
				Assert.IsTrue (selectionProvider.IsSelectionRequired, "#3");
				Assert.IsNull (selectionProvider.GetSelection (), "#4");

				tabControl.Items.Add (new TabItem () { Header = "Item0" });
				tabControl.Items.Add (new TabItem () { Header = "Item1" });
			},
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "#5");
				Assert.AreEqual (2, children.Count, "#6");

				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "#7");
				Assert.AreEqual (1, selection.Length, "#8");
				Assert.AreEqual ("Item0", new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), "#9");
			},
			() => tabControl.SelectedIndex = 1,
			() => {
				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "#10");
				Assert.AreEqual (1, selection.Length, "#11");
				Assert.AreEqual ("Item1", new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), "#12");
			});
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new TabControl ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// Can't subclass TabControlAutomationPeer
			return null;
		}
	}
}

