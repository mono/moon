//
// Unit tests for SelectorAutomationPeer
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
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class SelectorItemAutomationPeerTest : ItemAutomationPeerTest {
		
		public class SelectorItemAutomationPeerConcrete : SelectorItemAutomationPeer, FrameworkElementAutomationPeerContract {
			public SelectorItemAutomationPeerConcrete (UIElement owner)
				: base (owner)
			{
			}

			#region Wrapper Methods

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public string GetAcceleratorKeyCore_ ()
			{
				return base.GetAcceleratorKeyCore ();
			}

			public string GetAccessKeyCore_ ()
			{
				return base.GetAccessKeyCore ();
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetAutomationIdCore_ ()
			{
				return base.GetAutomationIdCore ();
			}

			public Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public Point GetClickablePointCore_ ()
			{
				return base.GetClickablePointCore ();
			}

			public string GetHelpTextCore_ ()
			{
				return base.GetHelpTextCore ();
			}

			public string GetItemStatusCore_ ()
			{
				return base.GetItemStatusCore ();
			}

			public string GetItemTypeCore_ ()
			{
				return base.GetItemTypeCore ();
			}

			public string GetLocalizedControlTypeCore_ ()
			{
				return base.GetLocalizedControlTypeCore ();
			}

			public AutomationOrientation GetOrientationCore_ ()
			{
				return base.GetOrientationCore ();
			}

			public bool HasKeyboardFocusCore_ ()
			{
				return base.HasKeyboardFocusCore ();
			}

			public bool IsEnabledCore_ ()
			{
				return base.IsEnabledCore ();
			}

			public bool IsKeyboardFocusableCore_ ()
			{
				return base.IsKeyboardFocusableCore ();
			}

			public bool IsOffscreenCore_ ()
			{
				return base.IsOffscreenCore ();
			}

			public bool IsPasswordCore_ ()
			{
				return base.IsPasswordCore ();
			}

			public bool IsRequiredForFormCore_ ()
			{
				return base.IsRequiredForFormCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			#endregion
		}

		public class SelectorItemControlConcrete : ListBoxItem {
			public SelectorItemControlConcrete ()
				: base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new SelectorItemAutomationPeerConcrete (this);
			}
		}

		[TestMethod]
		public override void GetPattern ()
		{
			AutomationPeer ap = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Assert.IsNull (ap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (ap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (ap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (ap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (ap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (ap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (ap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (ap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (ap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (ap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (ap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (ap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (ap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (ap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (ap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (ap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (ap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
		}

		[TestMethod]
		public override void GetItemType_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");

			string itemType = "My Item Type";

			fe.SetValue (AutomationProperties.ItemTypeProperty, itemType);
			Assert.AreEqual (itemType, feap.GetItemType (), "GetItemType #1");
			Assert.AreEqual (itemType, feap.GetItemTypeCore_ (), "GetItemTypeCore #1");

			fe.SetValue (AutomationProperties.ItemTypeProperty, null);
			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType #2");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore #2");
		}

		[TestMethod]
		public override void GetName_AttachedProperty0 ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (string.Empty, feap.GetName (), "GetName #1");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (string.Empty, feap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #2");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1 ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract tbap = CreateConcreteFrameworkElementAutomationPeer (element);

			string textblockName = "Hello world:";
			string nameProperty = "TextBox name";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #0");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, tbap.GetName (), "GetName #1");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #3");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #4");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #5");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #5");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "SelectorItem ContentElement.");

			bool concreteLoaded = false;
			ListBoxItem concrete = CreateConcreteFrameworkElement () as ListBoxItem;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			TestPanel.Children.Add (concrete);

			// StackPanel with two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => concreteLoaded, "SelectorItemConcreteLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				concrete.Content = stackPanel;
			});
			EnqueueConditional (() => concreteLoaded && stackPanelLoaded, "SelectorItemConcreteLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				// Is 2 because StackPanel is wrapped into 1 ListBoxItem
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add another TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text3" });
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "SelectorItem ContentElement.");

			bool listboxLoaded = false;
			ListBox listbox = new ListBox ();
			ListBoxItem concrete = CreateConcreteFrameworkElement () as ListBoxItem;
			listbox.Loaded += (o, e) => listboxLoaded = true;
			listbox.Items.Add (concrete);
			TestPanel.Children.Add (listbox);

			EnqueueConditional (() => listboxLoaded, "ListBoxLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #0");

				Assert.AreEqual (string.Empty, children[0].GetName (), "children[0].GetName #0");
				
				concrete.Content = "ListBoxItem0";
				Assert.AreEqual ("ListBoxItem0", children[0].GetName (), "children[0].GetName #1");

				concrete.Content = "Hellooooooooo";
				Assert.AreEqual ("Hellooooooooo", children[0].GetName (), "children[0].GetName #2");

				Assert.IsNull (children [0].GetChildren (), "children[0].GetChildren");
			});
			EnqueueTestComplete ();
		}

		#region Pattern Tests

		[TestMethod]
		[Asynchronous]
		[SilverlightBug(@"A11y implementation doesn't work: Fails ""IsSelected #2""")]
		public virtual void ISelectionItemProvider_Methods ()
		{
			bool concreteLoaded = false;
			ListBox listbox = new ListBox ();
			ListBoxItem selectorItem = CreateConcreteFrameworkElement () as ListBoxItem;
			selectorItem.Content = "1";
			ListBoxItem secondSelectorItem = new ListBoxItem () { Content = "2" };
			listbox.Loaded += (o, e) => concreteLoaded = true;

			listbox.Items.Add (selectorItem);
			listbox.Items.Add (secondSelectorItem);

			TestPanel.Children.Add (listbox);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => {
				AutomationPeer peer1 = FrameworkElementAutomationPeer.CreatePeerForElement (selectorItem);
				AutomationPeer peer2 = FrameworkElementAutomationPeer.CreatePeerForElement (secondSelectorItem);

				ISelectionItemProvider selectionItemProvider1 = peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider1, "SelectionItem Provider #0");

				ISelectionItemProvider selectionItemProvider2 = peer2.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #1");

				// By default both are not selected
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #0");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #1");
				
				selectionItemProvider1.AddToSelection ();
				Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #2");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");

				Assert.Throws<InvalidOperationException> (() => {
					selectionItemProvider2.AddToSelection ();
				}, "AddToSelection #0");

				Assert.Throws<InvalidOperationException> (() => {
					selectionItemProvider2.Select ();
				}, "Select #1");

				selectionItemProvider1.RemoveFromSelection ();
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #4");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #5");

				selectionItemProvider1.Select ();
				Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #5");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #6");
			});
			EnqueueTestComplete ();
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new SelectorItemControlConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new SelectorItemAutomationPeerConcrete (element as SelectorItemControlConcrete);
		}
	}
}
