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

using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class SelectorAutomationPeerTest : ItemsControlAutomationPeerTest {

		public class SelectorAutomationPeerConcrete : SelectorAutomationPeer, FrameworkElementAutomationPeerContract {
			public SelectorAutomationPeerConcrete (SelectorConcrete owner)
				: base (owner)
			{
			}

			#region Overridden methods

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			#endregion

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

		// Subclassing ListBox instead of Selector because there's no public nor 
		// protected constructor defined on Selcetor.
		public class SelectorConcrete : ListBox {

			public SelectorConcrete () : base ()
			{
			}
			
			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new SelectorAutomationPeerConcrete (this);
			}
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
			Assert.IsNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (feap.GetPattern (PatternInterface.Selection), "Selection");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			// This is returning "AutomationControlType.List" because subclassed ListBox instead
			// of Selector (because you just can't subclass selector!), however I think Custom should 
			// be returned instead. So, our Moonlight implementation is returning "AutomationControlType.Custom"
			// instead of "AutomationControlType.List"
		}

		[TestMethod]
		[Asynchronous]
		public virtual void ContentTest_NoTemplate ()
		{
			ContentTest_Template (new SelectorConcrete () { Template = null });
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			ContentTest_Template (new SelectorConcrete ());
		}

		protected virtual void ContentTest_Template (Selector concrete)
		{
			// We are going to add a lot of elements to show the scrollbars
			// notice we are using default Template
			bool concreteLoaded = false;
			bool concreteLayoutUpdate = false;
			concrete.Width = 250;
			concrete.Height = 300;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.LayoutUpdated += (o, e) => concreteLayoutUpdate = true;
			TestPanel.Children.Add (concrete);

			// StackPanel with two TextBlocks
			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
				Assert.IsNull (peer.GetChildren (), "GetChildren #0");

				concreteLayoutUpdate = false;
				for (int index = 0; index < 100; index++)
					concrete.Items.Add (string.Format ("Item {0}", index));
			});
			EnqueueConditional (() => concreteLoaded && concreteLayoutUpdate, "ConcreteLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				
				Assert.AreEqual (concrete.Template == null,
				                 peer.GetPattern (PatternInterface.Scroll) == null, 
				                 "ScrollPattern #0");
				Assert.AreEqual (100, peer.GetChildren ().Count, "GetChildren.Count #1");
				concreteLayoutUpdate = false;
				concrete.Items.Add ("I'm looooooooooooooooooooooooooooooooooooooooooooong!");
			});
			EnqueueConditional (() => concreteLoaded && concreteLayoutUpdate, "ConcreteLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (101, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
			EnqueueTestComplete ();
		}

		#region Pattern Tests

		[TestMethod]
		[Asynchronous]
		[SilverlightBug("A11y implementation doesn't work")]
		public virtual void ISelectionProvider_Methods ()
		{
			bool concreteLoaded = false;
			Selector selector = CreateConcreteFrameworkElement () as Selector;
			selector.Width = 300;
			selector.Height = 400;
			selector.Loaded += (o, e) => concreteLoaded = true;

			selector.Items.Add (new TextBlock () { Text = "Item0" });
			selector.Items.Add (new TextBlock () { Text = "Item1" });

			TestPanel.Children.Add (selector);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (selector);

				ISelectionProvider selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider, "Selection Provider");

				Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #0");
				Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #0");

				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #0");

				selector.SelectedIndex = 1;
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #1");

				Assert.AreEqual (1, selector.SelectedIndex, "SelectedIndex #0");
				
				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.AreEqual (1, selection.Length, "GetSelection #2");
				IRawElementProviderSimple first = selection [0];

				selector.Items.Add (new TextBlock () { Text = "Item1" });
				selector.SelectedIndex = 0;
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #3");
				Assert.AreEqual (1, selection.Length, "GetSelection #4");

				selector.SelectedIndex = 1;
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #5");
				Assert.AreEqual (1, selection.Length, "GetSelection #6");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void ISelectionProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool concreteLoaded = false;
			Selector selector = CreateConcreteFrameworkElement () as Selector;
			selector.Width = 300;
			selector.Height = 400;
			selector.Loaded += (o, e) => concreteLoaded = true;

			selector.Items.Add (new TextBlock () { Text = "Item0" });
			selector.Items.Add (new TextBlock () { Text = "Item1" });

			AutomationPeer peer = null;
			AutomationPeer childPeer = null;
			IRawElementProviderSimple[] selection = null;
			AutomationPropertyEventTuple propertyTuple = null;
			ISelectionProvider selectionProvider = null;

			CreateAsyncTest (selector,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (selector);

				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider, "Selection Provider");

				Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #0");
				Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #0");

				selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #0");
			},
			() => { 
				EventsManager.Instance.Reset ();
				selector.SelectedIndex = 1; 
			},
			() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #1");
				Assert.AreEqual (1, selection.Length, "GetSelection #2");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #0");
				Assert.IsNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #0");
				Assert.IsNotNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #0");
				
				childPeer = new PeerFromProvider ().GetPeerFromProvider (selection [0]);
			},
			() => {
				EventsManager.Instance.Reset ();
				selector.Items.Add (new TextBlock () { Text = "Item1" });
				selector.SelectedIndex = 0;
			},
			() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #3");
				Assert.AreEqual (1, selection.Length, "GetSelection #4");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #1");
				Assert.IsNotNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #1");
				Assert.IsNotNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #1");

				Assert.AreNotEqual (selection [0],childPeer,"GetSelection #5");
			},
			() => { 
				EventsManager.Instance.Reset ();
				selector.SelectedIndex = -1;
			},
			() => {
				selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #6");
				
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, SelectionPatternIdentifiers.SelectionProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationPropertyEventFrom #2");
				Assert.IsNotNull (propertyTuple.OldValue, "GetPropertyAutomationEventFrom.OldValue #2");
				Assert.IsNull (propertyTuple.NewValue, "GetPropertyAutomationEventFrom.NewValue #2");
			});
		}

		#endregion

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TestIsNotKeyboardFocusable ();
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent ();
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new SelectorConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new SelectorAutomationPeerConcrete (element as SelectorConcrete);
		}
	}
}
