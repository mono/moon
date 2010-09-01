//
// Unit tests for ComboBoxAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 olivier dufour olivier(dot)duff(at)gmail(dot)com
// Copyright (c) 2009 Novell, Inc. (http://www.novell.com)
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
	public class ComboBoxAutomationPeerTest : SelectorAutomationPeerTest {

		public class ComboBoxAutomationPeerConcrete
			: ComboBoxAutomationPeer, FrameworkElementAutomationPeerContract {

			public ComboBoxAutomationPeerConcrete (ComboBoxConcrete owner)
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

		public class ComboBoxConcrete : ComboBox {
			public ComboBoxConcrete () : base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ComboBoxAutomationPeerConcrete (this);
			}
		}

		[TestMethod]
		public override void GetPattern ()
		{
			ComboBoxAutomationPeerConcrete feap
				= (ComboBoxAutomationPeerConcrete)CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());

			Assert.IsNull (feap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (feap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (feap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (feap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (feap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (feap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (feap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (feap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNotNull (feap.GetPattern (PatternInterface.Selection), "Selection");

			// LAMESPEC: Value should be returned because ComboBoxAutomationPeer realizes the interface
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("ComboBox", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("ComboBox", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.ComboBox, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.ComboBox, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest_NoTemplate ()
		{
			ContentTest_Template (new ComboBoxConcrete () { Template = null });
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			ContentTest_Template (new ComboBoxConcrete ());
		}

		protected override void ContentTest_Template (Selector selectorConcrete)
		{
			Assert.IsTrue (IsContentPropertyElement (), "ComboBox ContentElement.");

			bool concreteLoaded = false;
			bool expanded = false;

			ComboBoxConcrete concrete = (ComboBoxConcrete) selectorConcrete;
			concrete.Width = 300;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.DropDownOpened += (o, e) => expanded = true;
			TestPanel.Children.Add (concrete);

			concrete.Items.Add ("Item 0");
			concrete.Items.Add ("Item 1");

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => concrete.ApplyTemplate ());
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
				Assert.AreEqual (concrete.Template == null,
				                 peer.GetChildren () != null, 
								 "GetChildren #0");
			});
			Enqueue (() => concrete.IsDropDownOpen = true);
			EnqueueConditional (() => expanded, "Expanded #0");
			Enqueue (() => concrete.IsDropDownOpen = false);
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
			});
			Enqueue (() => concrete.Items.Add (new TextBlock () { Text = "Item 2" }));
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TestIsKeyboardFocusable ();
		}

		[TestMethod]
		[Asynchronous]
		public void TestHasKeyboardFocusAfterPattern ()
		{
			StackPanel panel = new StackPanel ();
			ComboBox fe = CreateConcreteFrameworkElement () as ComboBox;
			fe.Items.Add ("Item 0");
			fe.Items.Add ("Item 1");

			Button button = new Button () { Content = "Button" };

			panel.Children.Add (fe);
			panel.Children.Add (button);

			AutomationPeer buttonPeer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IExpandCollapseProvider provider = null;

			CreateAsyncTest (panel,
			() => {
				provider = (IExpandCollapseProvider) peer.GetPattern (PatternInterface.ExpandCollapse);
				Assert.IsNotNull (provider, "#1");
			},
			() => provider.Expand (),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#2"),
			() => button.Focus (),
			() => Assert.IsTrue (buttonPeer.HasKeyboardFocus (), "#3"),
			() => provider.Collapse (),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#4")
			);
		}

		#region IExpandCollapseProvider Tests

		[TestMethod]
		[Asynchronous]
		public void ExpandCollapseProvider_Methods ()
		{
			bool concreteLoaded = false;
			bool concreteLayoutUpdated = false;
			ComboBoxConcrete concrete = CreateConcreteFrameworkElement () as ComboBoxConcrete;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.LayoutUpdated += (o, e) => concreteLayoutUpdated = true;
			concrete.Items.Add (new ComboBoxItem () { Content = "1" });
			concrete.Items.Add (new ComboBoxItem () { Content = "2" });
			concrete.Width = 300;
			TestPanel.Children.Add (concrete);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => concrete.ApplyTemplate ());
			Enqueue (() => {
				IExpandCollapseProvider expandCollapse = GetExpandCollapseProvider (concrete);
				Assert.AreEqual (ExpandCollapseState.Collapsed, 
					expandCollapse.ExpandCollapseState, "ExpandCollapseState #0");
				concreteLayoutUpdated = false;
				expandCollapse.Expand ();
			});
			EnqueueConditional (() => concreteLayoutUpdated, "ConcreteLayoutUpdated #0");
			Enqueue (() => {
				// Test event
				IExpandCollapseProvider expandCollapse = GetExpandCollapseProvider (concrete);
				Assert.AreEqual (ExpandCollapseState.Expanded,
					expandCollapse.ExpandCollapseState, "ExpandCollapseState #1");
				concreteLayoutUpdated = false;
				expandCollapse.Collapse ();
			});
			EnqueueConditional (() => concreteLayoutUpdated, "ConcreteLayoutUpdated #1");
			Enqueue (() => {
				// Test event
				IExpandCollapseProvider expandCollapse = GetExpandCollapseProvider (concrete);
				Assert.AreEqual (ExpandCollapseState.Collapsed,
					expandCollapse.ExpandCollapseState, "ExpandCollapseState #2");
				concreteLayoutUpdated = false;
			});
			EnqueueTestComplete ();
		}

		private IExpandCollapseProvider GetExpandCollapseProvider (FrameworkElement element)
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
			Assert.IsNotNull (peer, "CreatePeerForElement #0");

			IExpandCollapseProvider expandCollapse
				= peer.GetPattern (PatternInterface.ExpandCollapse) as IExpandCollapseProvider;
			Assert.IsNotNull (expandCollapse, "ExpandCollapseProvider  #0");
			return expandCollapse;
		}

		[TestMethod]
		[Asynchronous]
		public void ExpandCollapseProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool expanded = false;
			bool concreteLoaded = false;
			bool collapsed = false;
			ComboBox combobox = CreateConcreteFrameworkElement () as ComboBox;
			combobox.Width = 300;
			combobox.Height = 400;
			combobox.Loaded += (o, e) => concreteLoaded = true;
			combobox.DropDownOpened += (o, e) => expanded = true;
			combobox.DropDownClosed += (o, e) => collapsed = true;

			combobox.Items.Add (new TextBlock () { Text = "Item0" });
			combobox.Items.Add (new TextBlock () { Text = "Item1" });

			AutomationPeer peer = null;
			AutomationPropertyEventTuple propertyTuple = null;

			EventsManager.Instance.Reset ();
			TestPanel.Children.Add (combobox);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => combobox.ApplyTemplate ());
			Enqueue (() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (combobox);
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, ExpandCollapsePatternIdentifiers.ExpandCollapseStateProperty);
				Assert.IsNull (propertyTuple, "GetAutomationPropertyEventFrom #0");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				combobox.IsDropDownOpen = true;
			});
			EnqueueConditional (() => expanded, "Expanded #0");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, ExpandCollapsePatternIdentifiers.ExpandCollapseStateProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #1");
				Assert.AreEqual ((ExpandCollapseState) propertyTuple.OldValue, 
				                 ExpandCollapseState.Collapsed,
				                 "GetPropertyAutomationEventFrom.OldValue #0");
				Assert.AreEqual ((ExpandCollapseState) propertyTuple.NewValue, 
				                 ExpandCollapseState.Expanded,
				                 "GetPropertyAutomationEventFrom.NewValue #0");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				combobox.IsDropDownOpen = false;
			});
			EnqueueConditional (() => collapsed, "Collapsed #0");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, ExpandCollapsePatternIdentifiers.ExpandCollapseStateProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #2");
				Assert.AreEqual ((ExpandCollapseState) propertyTuple.OldValue, 
				                 ExpandCollapseState.Expanded,
				                 "GetPropertyAutomationEventFrom.OldValue #1");
				Assert.AreEqual ((ExpandCollapseState) propertyTuple.NewValue, 
				                 ExpandCollapseState.Collapsed,
				                 "GetPropertyAutomationEventFrom.NewValue #1");
			});
			EnqueueTestComplete ();
		}

		#endregion

		#region IValueProvider Tests

		[TestMethod]
		[Asynchronous]
		public void ValueProvider_Methods ()
		{		
			bool concreteLoaded = false;
			ComboBoxConcrete concrete = CreateConcreteFrameworkElement () as ComboBoxConcrete;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.Items.Add (new ComboBoxItem () { Content = "1" });
			concrete.Items.Add (new ComboBoxItem () { Content = "2" });
			concrete.Width = 300;
			TestPanel.Children.Add (concrete);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => concrete.ApplyTemplate ());
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "CreatePeerForElement #0");

				IValueProvider value
					= peer.GetPattern (PatternInterface.Value) as IValueProvider;
				// Yes is returning null!
				Assert.IsNull (value, "ValueProvider #0");
				
				// We are going to try again using explicit cast
				value = peer as IValueProvider;
				Assert.IsNotNull (value, "ValueProvider #1");

				// We can't change the value anyway
				Assert.IsTrue (value.IsReadOnly, "IsReadOnly #0");
				Assert.Throws<InvalidOperationException> (delegate {
					value.SetValue ("1");
				}, "SetValue #0");
			});
			EnqueueTestComplete ();
		}

		#endregion

		#region ISelectionProvider Tests

		[TestMethod]
		[Asynchronous]
		[SilverlightBug("A11y implementation doesn't work")]
		public override void ISelectionProvider_Methods ()
		{
			bool concreteLoaded = false;
			bool expanded = false;
			ComboBox combobox = CreateConcreteFrameworkElement () as ComboBox;
			combobox.Width = 300;
			combobox.Loaded += (o, e) => concreteLoaded = true;
			combobox.DropDownOpened += (o, e) => expanded = true;

			combobox.Items.Add ("Item0");
			combobox.Items.Add ("Item1");

			TestPanel.Children.Add (combobox);

			AutomationPeer peer = null;
			ISelectionProvider selectionProvider = null;
			IRawElementProviderSimple[] selection = null;

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => combobox.ApplyTemplate ());
			Enqueue (() => combobox.IsDropDownOpen = true);
			EnqueueConditional (() => expanded, "Expanded #0");
			Enqueue (() => combobox.IsDropDownOpen = false);
			Enqueue (() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (combobox);

				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider, "Selection Provider");

				Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #0");
				Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #0");

				selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #0");
			});
			Enqueue (() => combobox.SelectedIndex = 1);
			Enqueue (() => { 
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #1");
				Assert.AreEqual ("Item1", 
				                 new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), 
						 "Name #0");

				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.AreEqual (1, selection.Length, "GetSelection #2");
			});
			Enqueue (() => {
				combobox.Items.Add ("Item2");
				combobox.SelectedIndex = 0;
			});
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #3");
				Assert.AreEqual (1, selection.Length, "GetSelection #4");
				Assert.AreEqual ("Item0", 
				                 new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), 
						 "Name #2");
			});
			Enqueue (() => combobox.SelectedIndex = 1 );
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #5");
				Assert.AreEqual (1, selection.Length, "GetSelection #6");
				Assert.AreEqual ("Item1", 
				                 new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), 
						 "Name #3");
			});
			Enqueue (() => combobox.SelectedIndex = 2 );
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #7");
				Assert.AreEqual (1, selection.Length, "GetSelection #8");
				Assert.AreEqual ("Item2", 
				                 new PeerFromProvider ().GetPeerFromProvider (selection [0]).GetName (), 
						 "Name #4");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public override void ISelectionProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool expanded = false;
			bool concreteLoaded = false;
			ComboBox combobox = CreateConcreteFrameworkElement () as ComboBox;
			combobox.Width = 300;
			combobox.Height = 400;
			combobox.Loaded += (o, e) => concreteLoaded = true;
			combobox.DropDownOpened += (o, e) => expanded = true;

			combobox.Items.Add (new TextBlock () { Text = "Item0" });
			combobox.Items.Add (new TextBlock () { Text = "Item1" });

			AutomationPeer peer = null;
			AutomationPeer childPeer = null;
			IRawElementProviderSimple[] selection = null;
			AutomationPropertyEventTuple propertyTuple = null;
			ISelectionProvider selectionProvider = null;

			TestPanel.Children.Add (combobox);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => combobox.ApplyTemplate ());
			Enqueue (() => combobox.IsDropDownOpen = true);
			EnqueueConditional (() => expanded, "Expanded #0");
			Enqueue (() => combobox.IsDropDownOpen = false);
			Enqueue (() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (combobox);

				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider, "Selection Provider");

				Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #0");
				Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #0");

				selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #0");
			});
			Enqueue (() => { 
				EventsManager.Instance.Reset ();
				combobox.SelectedIndex = 1; 
			});
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #1");
				Assert.AreEqual (1, selection.Length, "GetSelection #2");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #0");
				Assert.IsNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #0");
				Assert.IsNotNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #0");
				
				childPeer = new PeerFromProvider ().GetPeerFromProvider (selection [0]);
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				combobox.Items.Add (new TextBlock () { Text = "Item1" });
				combobox.SelectedIndex = 0;
			});
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #3");
				Assert.AreEqual (1, selection.Length, "GetSelection #4");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #1");
				Assert.IsNotNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #1");
				Assert.IsNotNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #1");

				Assert.AreNotEqual (selection [0], childPeer, "GetSelection #5");
			});
			Enqueue (() => { 
				EventsManager.Instance.Reset ();
				combobox.SelectedIndex = -1;
			});
			Enqueue (() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #6");
				
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #2");
				Assert.IsNotNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #2");
				Assert.IsNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #2");
			});
			EnqueueTestComplete ();
		}


		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ComboBoxConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ComboBoxAutomationPeerConcrete (element as ComboBoxConcrete);
		}
	}
}

