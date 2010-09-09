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

		[TestMethod]
		[Asynchronous]
		public void TestHasKeyboardFocusAfterPattern ()
		{
			ListBox listbox = new ListBox ();
			listbox.Items.Add ("Item 0");
			listbox.Items.Add ("Item 1");

			AutomationPeer peer1 = null;
			AutomationPeer peer2 = null;
			ISelectionItemProvider selectionItemProvider1 = null;
			ISelectionItemProvider selectionItemProvider2 = null;
			AutomationPeer listboxPeer = null;

			CreateAsyncTest (listbox,
			() => {
				listboxPeer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);

				peer1 = listboxPeer.GetChildren()[0];
				peer2 = listboxPeer.GetChildren()[1];

				selectionItemProvider1
					= peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider1, "SelectionItem Provider #0");

				selectionItemProvider2
					= peer2.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #1");

				// By default both are not selected
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #0");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #1");
			},
			() => selectionItemProvider1.Select (),
			() => {
				Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #2");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");
			},
			() => {
				Assert.IsTrue (peer1.HasKeyboardFocus (), "#1");
				Assert.IsFalse (peer2.HasKeyboardFocus (), "#2");
				Assert.IsFalse (listboxPeer.HasKeyboardFocus (), "#3");
			},
			() => { selectionItemProvider2.Select (); },
			() => {
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #4");
				Assert.IsTrue (selectionItemProvider2.IsSelected, "IsSelected #5");
			},
			() => {
				Assert.IsFalse (peer1.HasKeyboardFocus (), "#4");
				Assert.IsTrue (peer2.HasKeyboardFocus (), "#5");
				Assert.IsFalse (listboxPeer.HasKeyboardFocus (), "#6");
			},
			() => { selectionItemProvider2.RemoveFromSelection (); },
			() => {
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #6");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #7");
			},
			() => {
				Assert.IsTrue (peer1.HasKeyboardFocus (), "#7");
				Assert.IsFalse (peer2.HasKeyboardFocus (), "#8");
				Assert.IsFalse (listboxPeer.HasKeyboardFocus (), "#9");
			});
		}

		#region Pattern Tests

		[TestMethod]
		[Asynchronous]
		[SilverlightBug(@"A11y implementation doesn't work: Fails ""IsSelected #2""")]
		public virtual void ISelectionItemProvider_Methods_ListBoxItem ()
		{
			ISelectionItemProvider_Methods (new List<object>() { 
				new ListBoxItem() { Content = "1" },
				new ListBoxItem() { Content = "2" } 
			});
		}

		[TestMethod]
		[Asynchronous]
		[SilverlightBug(@"A11y implementation doesn't work: Fails ""AddToSelection #0""")]
		public virtual void ISelectionItemProvider_Methods_Strings ()
		{
			ISelectionItemProvider_Methods (new List<object>() { "1", "2" } );
		}

		protected void ISelectionItemProvider_Methods (List<object> items)
		{
			ListBox listbox = new ListBox ();
			foreach (object item in items)
				listbox.Items.Add (item);

			AutomationPeer peer1 = null;
			AutomationPeer peer2 = null;
			ISelectionItemProvider selectionItemProvider1 = null;
			ISelectionItemProvider selectionItemProvider2 = null;
            
			CreateAsyncTest (listbox,
				() => {
					AutomationPeer listboxPeer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);

					peer1 = listboxPeer.GetChildren()[0];
					peer2 = listboxPeer.GetChildren()[1];

					selectionItemProvider1 
						= peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
					Assert.IsNotNull(selectionItemProvider1, "SelectionItem Provider #0");

					selectionItemProvider2 
						= peer2.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
					Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #1");

					// By default both are not selected
					Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #0");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #1");
				},
				() => { selectionItemProvider1.AddToSelection(); },
				() => {
					Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #2");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");
				},
				() => { selectionItemProvider1.Select(); }, // Nothing really changes
				() => {
					Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #4");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #5");
					// Throws exception because an element is already selected
					Assert.Throws<InvalidOperationException> (delegate {
						selectionItemProvider2.AddToSelection ();
					}, "AddToSelection #0");

					Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #6");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #7");
				},
				() => { selectionItemProvider1.RemoveFromSelection (); },
				() => {
					Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #8");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #9");
				},
				() => { selectionItemProvider2.AddToSelection (); },
				() => { 
					Assert.IsFalse(selectionItemProvider1.IsSelected, "IsSelected #10");
					Assert.IsTrue(selectionItemProvider2.IsSelected, "IsSelected #11");
				},
				() => { selectionItemProvider2.Select (); }, // Nothing really changes
				() => { 
					Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #12");
					Assert.IsTrue (selectionItemProvider2.IsSelected, "IsSelected #13");
				},
				() => { selectionItemProvider2.RemoveFromSelection (); },
				() => { 
					Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #14");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #15");
				},
				() => { selectionItemProvider1.Select (); },
				() => {
					Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #16");
					Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #17");
				},
				() => { selectionItemProvider2.Select (); },
				() => {
					Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #18");
					Assert.IsTrue (selectionItemProvider2.IsSelected, "IsSelected #19");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public virtual void ISelectionItemProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ListBox listbox = new ListBox ();
			ListBoxItem selectorItem = CreateConcreteFrameworkElement () as ListBoxItem;
			selectorItem.Content = "1";
			ListBoxItem secondSelectorItem = new ListBoxItem () { Content = "2" };

			listbox.Items.Add (selectorItem);
			listbox.Items.Add (secondSelectorItem);

			AutomationPeer peer = null;
			AutomationPeer peer1 = null;
			AutomationPeer peer2 = null;
			ISelectionItemProvider selectionItemProvider1 = null;
			ISelectionItemProvider selectionItemProvider2 = null;
			AutomationEventTuple tuple = null;
			AutomationPropertyEventTuple propertyTuple = null;
			
			CreateAsyncTest(listbox,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);
				peer1 = FrameworkElementAutomationPeer.CreatePeerForElement(selectorItem);
				peer2 = FrameworkElementAutomationPeer.CreatePeerForElement(secondSelectorItem);

				selectionItemProvider1 = peer1.GetPattern (PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider1, "SelectionItem Provider #0");

				selectionItemProvider2 = peer2.GetPattern( PatternInterface.SelectionItem) as ISelectionItemProvider;
				Assert.IsNotNull (selectionItemProvider2, "SelectionItem Provider #1");

				// By default both are not selected
				Assert.IsFalse(selectionItemProvider1.IsSelected, "IsSelected #0");
				Assert.IsFalse(selectionItemProvider2.IsSelected, "IsSelected #1");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider1.AddToSelection();
			},
			() => {
				Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #2");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #3");

				tuple = EventsManager.Instance.GetAutomationEventFrom(peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #0");
				
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #0");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #0");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #0");

				EventsManager.Instance.Reset();
				selectionItemProvider1.Select (); // Nothing really changes
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom(peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #1");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (propertyTuple, "GetPropertyAutomationEventFrom #1");

				Assert.IsTrue (selectionItemProvider1.IsSelected, "IsSelected #4");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #5");

				// Throws exception because an element is already selected
				Assert.Throws<InvalidOperationException>(delegate {
					selectionItemProvider2.AddToSelection();
				}, "AddToSelection #0");

				Assert.IsTrue(selectionItemProvider1.IsSelected, "IsSelected #6");
				Assert.IsFalse(selectionItemProvider2.IsSelected, "IsSelected #7");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider1.RemoveFromSelection();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom(peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull(tuple, "GetAutomationEventFrom #2");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #3");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #2");
				Assert.IsTrue ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #1");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #1");

				Assert.IsFalse(selectionItemProvider1.IsSelected, "IsSelected #8");
				Assert.IsFalse(selectionItemProvider2.IsSelected, "IsSelected #9");
			},
			() => {
				EventsManager.Instance.Reset();
				selectionItemProvider2.AddToSelection();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom(peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #4");

				tuple = EventsManager.Instance.GetAutomationEventFrom(peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #5");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #3");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #2");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #2");

				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #10");
				Assert.IsTrue (selectionItemProvider2.IsSelected, "IsSelected #11");
			},
			() => {
				EventsManager.Instance.Reset ();
				selectionItemProvider2.Select (); // Nothing really changes
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom(peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #6");

				tuple = EventsManager.Instance.GetAutomationEventFrom(peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #7");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (propertyTuple, "GetPropertyAutomationEventFrom #4");

				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #12");
				Assert.IsTrue (selectionItemProvider2.IsSelected, "IsSelected #13");
			},
			() => { 
				EventsManager.Instance.Reset ();
				selectionItemProvider2.RemoveFromSelection (); 
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom(peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #7");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #6");
				Assert.IsTrue ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #4");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #4");
			
				Assert.IsFalse (selectionItemProvider1.IsSelected, "IsSelected #14");
				Assert.IsFalse (selectionItemProvider2.IsSelected, "IsSelected #15");
			},
			() => { 
				EventsManager.Instance.Reset ();
				selectionItemProvider1.Select();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #8");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #9");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #7");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #5");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #5");

				Assert.IsTrue(selectionItemProvider1.IsSelected, "IsSelected #16");
				Assert.IsFalse(selectionItemProvider2.IsSelected, "IsSelected #17");
			},
			() => { 
				EventsManager.Instance.Reset ();
				selectionItemProvider2.Select();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer1, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "GetAutomationEventFrom #10");
				
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer2, AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #11");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #8");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #6");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #6");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer1, SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (propertyTuple, "GetPropertyAutomationEventFrom #9");
				Assert.IsTrue ((bool) propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #7");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #7");

				Assert.IsFalse(selectionItemProvider1.IsSelected, "IsSelected #18");
				Assert.IsTrue(selectionItemProvider2.IsSelected, "IsSelected #19");

			},
			() => { 
				EventsManager.Instance.Reset ();
				listbox.Items.Remove (secondSelectorItem);
			},
			() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.SelectionContainerProperty);
				Assert.IsNotNull (tuple, "GetPropertyAutomationEventFrom #10");
				Assert.AreEqual (new PeerFromProvider ().GetPeerFromProvider ((IRawElementProviderSimple) propertyTuple.OldValue), 
				                 peer, 
						 "GetPropertyAutomationEventFrom.OldValue #8");
				Assert.IsNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #8");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #12");
			},
			() => { 
				EventsManager.Instance.Reset ();
				listbox.Items.Add (secondSelectorItem);
			},
			() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer2, SelectionItemPatternIdentifiers.SelectionContainerProperty);
				Assert.IsNotNull (tuple, "GetPropertyAutomationEventFrom #11");
				Assert.IsNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #9");
				Assert.AreEqual (new PeerFromProvider ().GetPeerFromProvider ((IRawElementProviderSimple) propertyTuple.NewValue), 
				                 peer, 
						 "GetPropertyAutomationEventFrom.NewValue #9");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #13");
			});
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
