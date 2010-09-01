//
// Unit tests for RadioButtonAutomationPeer
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Automation;
using System.Collections.Generic;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class RadioButtonAutomationPeerTest : ToggleButtonAutomationPeerTest {

		public class RadioButtonAutomationPeerPoker : RadioButtonAutomationPeer, FrameworkElementAutomationPeerContract {

			public RadioButtonAutomationPeerPoker (RadioButton owner)
				: base (owner)
			{
			}

			#region Overriden methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public object GetPattern_ (PatternInterface iface)
			{
				return base.GetPattern (iface);
			}

			#endregion

			#region Wrapper methods

			public IRawElementProviderSimple ProviderFromPeer_ (AutomationPeer peer)
			{
				return ProviderFromPeer (peer);
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
			}

			public string GetAcceleratorKeyCore_ ()
			{
				return base.GetAcceleratorKeyCore ();
			}

			public string GetAccessKeyCore_ ()
			{
				return base.GetAccessKeyCore ();
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

			#endregion
		}

		[TestMethod]
		public override void GetPattern ()
		{
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ())
					as FrameworkElementAutomationPeer;

			Assert.IsNull (peer.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (peer.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (peer.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (peer.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (peer.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (peer.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (peer.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (peer.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (peer.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (peer.GetPattern (PatternInterface.Window), "Window");
			Assert.IsNull (peer.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (peer.GetPattern (PatternInterface.Toggle), "Toggle");

			Assert.IsNotNull (peer.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("RadioButton", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("RadioButton", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.RadioButton, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.RadioButton, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		[Asynchronous]
		public override void TestHasKeyboardFocusAfterPattern ()
		{
			ToggleButton fe = CreateConcreteFrameworkElement ()
				as ToggleButton;
			fe.Content = "Radiobutton";

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			ISelectionItemProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (ISelectionItemProvider) peer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.Select (),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#1"));
		}

		[TestMethod]
		public override void ToggleProvider_Toggle ()
		{
			RadioButton radioButton = CreateConcreteFrameworkElement () as RadioButton;
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (radioButton);

			radioButton.IsEnabled = false;

			IToggleProvider toggleProvider = feap as IToggleProvider;

			try {
				toggleProvider.Toggle ();
				Assert.Fail ("Should throw ElementNotEnabledException");
			} catch (ElementNotEnabledException) { }

			radioButton.IsEnabled = true;

			// Test two-state toggling
			radioButton.IsThreeState = false;

			radioButton.IsChecked = false;
			Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Start two-state toggle: Unchecked");

			toggleProvider.Toggle ();
			Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "First two-state toggle: Checked");

			// Test three-state toggling
			radioButton.IsThreeState = true;

			radioButton.IsChecked = false;
			Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Start three-state Checked");

			toggleProvider.Toggle ();
			Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "First three-state toggle: Checked");

			// NOTE: I don't think it will ever be indeterminate
		}

		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_AddToSelection ()
		{
			RadioButton radioButton = CreateConcreteFrameworkElement () as RadioButton;

			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (radioButton);
			TestPanel.Children.Add (stackPanel);

			Enqueue (( ) => {
				AutomationPeer peer
					= FrameworkElementAutomationPeer.CreatePeerForElement (radioButton);

				ISelectionItemProvider selectionItem
					= peer.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem);

				radioButton.IsEnabled = false;
				try {
					selectionItem.AddToSelection ();
					Assert.Fail ("InvalidOperationException not raised when disabled");
				} catch (InvalidOperationException) { }

				radioButton.IsEnabled = true;
				try {
					selectionItem.AddToSelection ();
					Assert.Fail ("InvalidOperationException not raised when enabled");
				} catch (InvalidOperationException) { }
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_RemoveFromSelection ()
		{
			RadioButton radioButton = CreateConcreteFrameworkElement () as RadioButton;

			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (radioButton);
			TestPanel.Children.Add (stackPanel);

			Enqueue (( ) => {
				AutomationPeer peer
					= FrameworkElementAutomationPeer.CreatePeerForElement (radioButton);

				ISelectionItemProvider selectionItem
					= peer.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem);

				radioButton.IsEnabled = false;
				selectionItem.RemoveFromSelection ();

				radioButton.IsChecked = true;
				try {
					selectionItem.RemoveFromSelection ();
				} catch (InvalidOperationException) { }

				radioButton.IsChecked = false;
				radioButton.IsEnabled = true;
				selectionItem.RemoveFromSelection ();

				radioButton.IsChecked = true;
				try {
					selectionItem.RemoveFromSelection ();
				} catch (InvalidOperationException) { }
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_Select ()
		{
			RadioButton rb1 = CreateConcreteFrameworkElement () as RadioButton;
			RadioButton rb2 = CreateConcreteFrameworkElement () as RadioButton;
			rb1.GroupName = rb2.GroupName = "Fruit";

			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (rb1);
			stackPanel.Children.Add (rb2);
			TestPanel.Children.Add (stackPanel);

			Enqueue (( ) => {
				AutomationPeer radioPeer1
					= FrameworkElementAutomationPeer.CreatePeerForElement (rb1);
				AutomationPeer radioPeer2
					= FrameworkElementAutomationPeer.CreatePeerForElement (rb2);

				ISelectionItemProvider selectionItem1
					= radioPeer1.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem1);

				ISelectionItemProvider selectionItem2
					= radioPeer2.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem2);

				rb1.IsEnabled = false;
				try {
					selectionItem1.Select ();
					Assert.Fail ("ElementNotEnabledException not raised when disabled");
				} catch (ElementNotEnabledException) { }

				rb1.IsEnabled = true;
				selectionItem1.Select ();
				Assert.IsTrue (rb1.IsChecked == true,
				               "Select didn't check rb1");
				Assert.IsTrue (rb2.IsChecked == false,
				               "rb2 is selected when rb1 should be");

				// This shouldn't raise any exceptions
				selectionItem1.Select ();

				selectionItem2.Select ();
				Assert.IsTrue (rb2.IsChecked == true,
				               "Select didn't check rb2");
				Assert.IsTrue (rb1.IsChecked == false,
				               "rb1 is selected when rb2 should be");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_IsSelected ()
		{
			RadioButton rb1 = CreateConcreteFrameworkElement () as RadioButton;
			RadioButton rb2 = CreateConcreteFrameworkElement () as RadioButton;
			rb1.GroupName = rb2.GroupName = "Fruit";

			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (rb1);
			stackPanel.Children.Add (rb2);
			TestPanel.Children.Add (stackPanel);

			Enqueue (( ) => {
				AutomationPeer radioPeer1
					= FrameworkElementAutomationPeer.CreatePeerForElement (rb1);
				AutomationPeer radioPeer2
					= FrameworkElementAutomationPeer.CreatePeerForElement (rb2);

				ISelectionItemProvider selectionItem1
					= radioPeer1.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem1);

				ISelectionItemProvider selectionItem2
					= radioPeer2.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem2);

				rb1.IsEnabled = true;
				rb1.IsChecked = true;
				Assert.IsTrue (selectionItem1.IsSelected,
				               "rb1 should be selected when checked");
				Assert.IsFalse (selectionItem2.IsSelected,
				                "rb2 is selected when rb1 should be");

				rb1.IsEnabled = false;
				rb1.IsChecked = true;
				Assert.IsTrue (selectionItem1.IsSelected,
				               "rb1 should be selected when checked");
				Assert.IsFalse (selectionItem2.IsSelected,
				                "rb2 is selected when rb1 should be");

				rb2.IsEnabled = true;
				rb2.IsChecked = true;
				Assert.IsTrue (selectionItem2.IsSelected,
				               "rb2 should be selected when checked");
				Assert.IsFalse (selectionItem1.IsSelected,
				                "rb1 is selected when rb2 should be");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_SelectionContainer ()
		{
			RadioButton rb1 = CreateConcreteFrameworkElement () as RadioButton;
			RadioButton rb2 = CreateConcreteFrameworkElement () as RadioButton;
			rb1.GroupName = rb2.GroupName = "Fruit";

			ListBox listBox = new ListBox ();
			listBox.Items.Add (rb1);
			listBox.Items.Add (rb2);
			TestPanel.Children.Add (listBox);

			RadioButton rb3 = CreateConcreteFrameworkElement () as RadioButton;
			RadioButton rb4 = CreateConcreteFrameworkElement () as RadioButton;

			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (rb3);
			stackPanel.Children.Add (rb4);
			TestPanel.Children.Add (stackPanel);

			Enqueue (( ) => {
				FrameworkElementAutomationPeer radioPeer1
					= CreateConcreteFrameworkElementAutomationPeer (rb1)
						as FrameworkElementAutomationPeer;
				FrameworkElementAutomationPeer radioPeer3
					= CreateConcreteFrameworkElementAutomationPeer (rb3)
						as FrameworkElementAutomationPeer;

				ISelectionItemProvider selectionItem1
					= radioPeer1.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem1);

				ISelectionItemProvider selectionItem3
					= radioPeer3.GetPattern (PatternInterface.SelectionItem)
						as ISelectionItemProvider;
				Assert.IsNotNull (selectionItem3);

				Assert.IsNull (selectionItem1.SelectionContainer,
				               "selectionItem1's container is not null");
				Assert.IsNull (selectionItem3.SelectionContainer,
				               "selectionItem2's container is not null");
			});
			EnqueueTestComplete ();
		}


		[TestMethod]
		[Asynchronous]
		public void SelectionItemProvider_IsSelectedEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			RadioButton radioButton = CreateConcreteFrameworkElement () as RadioButton;
			AutomationPeer peer
				= FrameworkElementAutomationPeer.CreatePeerForElement (radioButton);
			AutomationPropertyEventTuple tuple = null;
			ISelectionItemProvider selectionProvider
				= (ISelectionItemProvider) peer.GetPattern (PatternInterface.SelectionItem);

			CreateAsyncTest (radioButton,
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsThreeState = false;
				radioButton.IsChecked = false;
			},
			// Test two-state toggling
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (tuple, "#0");
			},
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsChecked = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (tuple, "#1");
				Assert.IsFalse ((bool) tuple.OldValue, "#2");
				Assert.IsTrue ((bool) tuple.NewValue, "#3");
			},
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsChecked = false;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (tuple, "#4");
				Assert.IsTrue ((bool) tuple.OldValue, "#5");
				Assert.IsFalse ((bool) tuple.NewValue, "#6");
			},
			// Test three-state toggling
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsThreeState = true;
				radioButton.IsChecked = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (tuple, "#7");
				Assert.IsFalse ((bool) tuple.OldValue, "#8");
				Assert.IsTrue ((bool) tuple.NewValue, "#9");
			},
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsChecked = null;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNull (tuple, "#10");
			},
			() => {
				EventsManager.Instance.Reset ();
				radioButton.IsChecked = false;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       SelectionItemPatternIdentifiers.IsSelectedProperty);
				Assert.IsNotNull (tuple, "#13");
				Assert.IsTrue ((bool) tuple.OldValue, "#14");
				Assert.IsFalse ((bool) tuple.NewValue, "#15");
			});
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new RadioButton ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new RadioButtonAutomationPeerPoker (element as RadioButton);
		}
	}
}

