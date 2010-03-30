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

		[TestMethod]
		[Asynchronous]
		public void ISelectionItemProvider_Methods ()
		{
			TabControl tabControl = new TabControl ();
			TabItem item0 = new TabItem () { Header = "Item0" };
			TabItem item1 = new TabItem () { Header = "Item1" };
			TabItem item2 = new TabItem () { Header = "Item2" };
			tabControl.Items.Add (item0);
			tabControl.Items.Add (item1);
			tabControl.Items.Add (item2);
			item0.IsSelected = true;

			AutomationPeer peer0 = null;
			AutomationPeer peer1 = null;
			AutomationPeer peer2 = null;

			ISelectionItemProvider selectionItemProvider0 = null;
			ISelectionItemProvider selectionItemProvider1 = null;
			ISelectionItemProvider selectionItemProvider2 = null;

			CreateAsyncTest (tabControl,
			() => {
				AutomationPeer tabControlPeer
					= FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);

				List<AutomationPeer> children = tabControlPeer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (3, children.Count, "GetChildren #1");

				peer0 = FrameworkElementAutomationPeer.CreatePeerForElement (item0);
				peer1 = FrameworkElementAutomationPeer.CreatePeerForElement (item1);
				peer2 = FrameworkElementAutomationPeer.CreatePeerForElement (item2);

				selectionItemProvider0
					= peer0.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider0, "SelectionItem Provider #1");

				selectionItemProvider1
					= peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider1, "SelectionItem Provider #2");

				selectionItemProvider2
					= peer2.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #3");

				Assert.IsTrue  (selectionItemProvider0.IsSelected, "IsSelected #1");
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #2");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");
			},
			() => { selectionItemProvider1.AddToSelection(); },
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #4");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #5");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #6");
			},
			() => { selectionItemProvider1.Select(); }, // Nothing changes, is already selected
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #7");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #8");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #9");
			},
			() => { selectionItemProvider1.RemoveFromSelection(); }, // Nothing happens, Remove does nothing.
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #10");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #11");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #12");
			},
			() => { selectionItemProvider2.AddToSelection (); }, // Doesn't throw exception
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #13");
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #14");
				Assert.IsTrue  (selectionItemProvider2.IsSelected, "IsSelected #15");
			},
			() => { selectionItemProvider0.Select (); },
			() => {
				Assert.IsTrue  (selectionItemProvider0.IsSelected, "IsSelected #16");
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #17");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #18");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ISelectionItemProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TabControl tabControl = new TabControl ();
			TabItem item0 = new TabItem () { Header = "Item0", Content = new TextBlock () { Text = "Item 0" } };
			TabItem item1 = new TabItem () { Header = "Item1", Content = new TextBlock () { Text = "Item 1" } };
			TabItem item2 = new TabItem () { Header = "Item2", Content = new TextBlock () { Text = "Item 2" } };
			tabControl.Items.Add (item0);
			tabControl.Items.Add (item1);
			tabControl.Items.Add (item2);
			item0.IsSelected = true;

			AutomationPeer peer0 = null;
			AutomationPeer peer1 = null;
			AutomationPeer peer2 = null;

			ISelectionItemProvider selectionItemProvider0 = null;
			ISelectionItemProvider selectionItemProvider1 = null;
			ISelectionItemProvider selectionItemProvider2 = null;

			AutomationPropertyEventTuple propertyTuple = null;

			CreateAsyncTest(tabControl,
			() => {
				AutomationPeer tabControlPeer
					= FrameworkElementAutomationPeer.CreatePeerForElement (tabControl);

				List<AutomationPeer> children = tabControlPeer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");

				peer0 = children [0];
				peer1 = children [1];
				peer2 = children [2];

				selectionItemProvider0
					= peer0.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull(selectionItemProvider0, "SelectionItem Provider #1");

				selectionItemProvider1
					= peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull(selectionItemProvider1, "SelectionItem Provider #2");

				selectionItemProvider2
					= peer2.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #3");

				selectionItemProvider1 = peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider1, "SelectionItem Provider #0");

				selectionItemProvider2 = peer2.GetPattern( PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #1");

				Assert.IsTrue  (selectionItemProvider0.IsSelected, "IsSelected #1");
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #2");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");

				EventsManager.Instance.Reset ();
				selectionItemProvider0.Select (); // Nothing really changes
			},
			() => {
				EventsManager.Instance.Reset ();
				selectionItemProvider1.AddToSelection ();
			},
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #4");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #5");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #6");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer0,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #0");
				Assert.IsTrue  ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #0");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #0");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #1");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #1");
				Assert.IsTrue  ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #1");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider1.Select (); // Nothing really changes
			},
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #7");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #8");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #9");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (propertyTuple, "GetPropertyAutomationEventFrom #2");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider1.RemoveFromSelection (); // Nothing really changes
			},
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #10");
				Assert.IsTrue  (selectionItemProvider1.IsSelected, "IsSelected #11");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #12");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (propertyTuple, "GetPropertyAutomationEventFrom #3");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider2.Select ();
			},
			() => {
				Assert.IsFalse (selectionItemProvider0.IsSelected, "IsSelected #13");
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #14");
				Assert.IsTrue  (selectionItemProvider2.IsSelected, "IsSelected #15");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #4");
				Assert.IsTrue  ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #4");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #4");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2,
				                                                               SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #5");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #5");
				Assert.IsTrue  ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #5");
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
