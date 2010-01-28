//
// Unit tests for ItemsControlAutomationPeer
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

using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ItemsControlAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ItemsControlConcrete : ItemsControl {
			public ItemsControlConcrete () : base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ItemsControlAutomationPeerPoker (this);
			}
		}

		public class ItemsControlAutomationPeerPoker : ItemsControlAutomationPeer, FrameworkElementAutomationPeerContract {

			public ItemsControlAutomationPeerPoker (ItemsControlConcrete items) :
				base (items)
			{
			}

			#region Overridden Methods

			public List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
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

		[TestMethod]
		public override void GetPattern ()
		{
			ItemsControlAutomationPeerPoker icap = new ItemsControlAutomationPeerPoker (new ItemsControlConcrete ());

			Assert.IsNull (icap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (icap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (icap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (icap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (icap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (icap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (icap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (icap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (icap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (icap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (icap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (icap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (icap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (icap.GetPattern (PatternInterface.Window), "Window");
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ItemsControlConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ItemsControlAutomationPeerPoker (element as ItemsControlConcrete);
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ItemsControl ContentElement.");

			bool concreteLoaded = false;
			ItemsControlConcrete concrete = CreateConcreteFrameworkElement () as ItemsControlConcrete;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			TestPanel.Children.Add (concrete);

			// StackPanel with two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => concrete.ApplyTemplate ());
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				concrete.Items.Add (stackPanel);
				// Also one extra TextBlock
				concrete.Items.Add (new TextBlock () { Text = "Text2" });
			});
			EnqueueConditional (() => concreteLoaded && stackPanelLoaded, "ConcreteLoaded #1");
			Enqueue (() => {
				stackPanelLoaded = false;
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNull (peer.GetChildren (), "GetChildren #1");
				// We add another TextBlock and nothing changes
				stackPanel.Children.Add (new TextBlock () { Text = "Text3" });
				Assert.IsNull (peer.GetChildren (), "GetChildren #2");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void StructureChanged_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool concreteLoaded = false;
			ItemsControl concrete = CreateConcreteFrameworkElement () as ItemsControl;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			TestPanel.Children.Add (concrete);

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => concrete.ApplyTemplate ());
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement #0");
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "GetAutomationEventFrom #0");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				concrete.Items.Add (new ListBoxItem () { Content = "Item 0" });
			});
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement #1");
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #1");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				concrete.Items.RemoveAt (0);
			});
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement #2");
				AutomationEventTuple tuple
					= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #2");
			});
			EnqueueTestComplete ();
		}

	}
}
