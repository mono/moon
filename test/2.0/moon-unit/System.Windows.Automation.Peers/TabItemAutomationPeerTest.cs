   //
// Unit tests for ItemAutomationPeer
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class TabItemAutomationPeerTest : FrameworkElementAutomationPeerTest {

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ItemAutomation is ContentElement.");
			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			TabItem tabItem1 = new TabItem () { Content = new TextBlock () { Text = "TextBlock" } };
			AutomationPeer peer = null;
			StackPanel stackPanel = null;

			tabControl.Items.Add (tabItem);
			tabControl.Items.Add (tabItem1);

			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer tabPeer = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				List<AutomationPeer> children = tabPeer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (2, children.Count, "GetChildren.Count #0");

				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				Assert.IsNotNull (peer, "#0");
			},
			() => tabItem.IsSelected = true,
			() => {
				stackPanel = new StackPanel ();
				stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
				stackPanel.Children.Add (new TextBlock () { Text = "Text1" });

				Assert.IsNull (peer.GetChildren (), "GetChildren #1");
			},
			() => tabItem.Content = stackPanel,
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #2");
				Assert.AreEqual (2, children.Count, "GetChildren.Count #2");
			},
			() => stackPanel.Children.Add (new TextBlock () { Text = "Text2" }),
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #3");
				Assert.AreEqual (3, children.Count, "GetChildren.Count #3");
			},
			() => tabItem1.IsSelected = true,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem1);
				Assert.IsNotNull (peer, "#1");

				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #4");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #4");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildrenChanged ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			tabControl.Items.Add (tabItem);
			AutomationPeer peerTab = null;
			AutomationPeer peer = null;
			AutomationEventTuple tuple = null;

			CreateAsyncTest (tabControl,
			() => {
				peerTab = FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);
				Assert.IsNotNull (peerTab, "#0");

				List<AutomationPeer> children = peerTab.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #0");

				peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				Assert.IsNotNull (peer, "#0");
				children = peer.GetChildren ();
				Assert.IsNull (children, "GetChildren #1");
			},
			() => {
				StackPanel panel = new StackPanel ();
				tabItem.Content = panel;
				panel.Children.Add (new TextBlock () { Text = "Content" });
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #0");

				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #2");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #2");
			},
			() => {
				StackPanel panel = (StackPanel) tabItem.Content;
				panel.Children.Add (new TextBlock () { Text = "Content" });
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #1");

				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #3");
				Assert.AreEqual (2, children.Count, "GetChildren.Count #3");
			});
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			TabItem tabItem = CreateConcreteFrameworkElement () as TabItem;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem) as AutomationPeer;
			Assert.AreEqual (AutomationControlType.TabItem, peer.GetAutomationControlType (), "#0");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			TabItem tabItem = CreateConcreteFrameworkElement () as TabItem;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem) as AutomationPeer;
			Assert.AreEqual ("TabItem", peer.GetClassName (), "#0");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetItemType_AttachedProperty ()
		{
			TabControl tabControl = new TabControl ();
			CreateAsyncTest (tabControl,
			() => {
				TabItem tabItem = new TabItem ();
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);

				Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType");

				string itemType = "My Item Type";

				tabItem.SetValue (AutomationProperties.ItemTypeProperty, itemType);
				Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType #1");

				tabItem.SetValue (AutomationProperties.ItemTypeProperty, null);
				Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType #2");
			});
		}

		[TestMethod]
		public override void GetName_AttachedProperty0 ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName");
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				AutomationPropertyEventTuple tuple = null;

				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#0");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.NameProperty, "Attached Name");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#1");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.NameProperty, "Name");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#4");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.NameProperty, null);
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty1 ()
		{
			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);

				string textblockName = "Hello world:";
				string nameProperty = "TextBox name";

				TextBlock textblock = new TextBlock ();
				textblock.Text = textblockName;

				tabItem.SetValue (AutomationProperties.NameProperty, nameProperty);
				Assert.AreEqual (string.Empty, peer.GetName (), "GetName #0");

				tabItem.SetValue (AutomationProperties.LabeledByProperty, textblock);
				Assert.AreEqual (textblockName, peer.GetName (), "GetName #1");

				textblock.Text = null;
				Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");

				textblock.Text = string.Empty;
				Assert.AreEqual (string.Empty, peer.GetName (), "GetName #3");

				tabItem.SetValue (AutomationProperties.NameProperty, null);
				Assert.AreEqual (string.Empty, peer.GetName (), "GetName #4");

				tabItem.SetValue (AutomationProperties.LabeledByProperty, null);
				Assert.AreEqual (string.Empty, peer.GetName (), "GetName #5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty1Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				AutomationPropertyEventTuple tuple = null;

				TextBlock textblock = new TextBlock () { Text = "Hello world:" };
				AutomationPeer textblockPeer = FrameworkElementAutomationPeer.CreatePeerForElement (textblock);

				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#0");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.NameProperty, "My name");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#1");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.LabeledByProperty, textblock);
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#2");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
				Assert.IsNotNull (tuple, "#3");

				EventsManager.Instance.Reset ();
				textblock.Text = null;
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#4");

				tuple = EventsManager.Instance.GetAutomationEventFrom (textblockPeer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#5");

				EventsManager.Instance.Reset ();
				tabItem.SetValue (AutomationProperties.LabeledByProperty, null);
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#6");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
				Assert.IsNotNull (tuple, "#7");
			});
		}

		[TestMethod]
		public override void GetPattern ()
		{
			AutomationPeer feap = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Assert.IsNull (feap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (feap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (feap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (feap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (feap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (feap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (feap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (feap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();
			tabControl.Items.Add (tabItem);

			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				Assert.IsNotNull (peer, "#0");
				Assert.IsTrue (peer.IsKeyboardFocusable (), "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsKeyboardFocusableNoParent ()
		{
			TabControl tabControl = new TabControl ();
			TabItem tabItem = new TabItem ();

			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (tabItem);
				Assert.IsNotNull (peer, "#0");
				Assert.IsFalse (peer.IsKeyboardFocusable (), "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent ();
		}

		[TestMethod]
		public override void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new TabItemAutomationPeer (null);
			});
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new TabItem ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// TabItemAutomationPeer is sealed.
			return null;
		}
	}

}
