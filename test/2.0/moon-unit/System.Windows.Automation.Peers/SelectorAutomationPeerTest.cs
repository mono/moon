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
	public class SelectorAutomationPeerTest : FrameworkElementAutomationPeerTest {

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
		[Ignore("Uncomment when ScrollViewerPeer is implemented")]
		public override void ContentTest ()
		{
			// Since SelectorConcrete uses ListBox as base class we 
			// need to base on its implementation 
			Assert.IsTrue (IsContentPropertyElement (), "Selector ContentElement.");

			bool concreteLoaded = false;
			bool concreteLayoutUpdate = false;
			SelectorConcrete concrete = CreateConcreteFrameworkElement () as SelectorConcrete;
			concrete.Width = 300;
			concrete.Height = 300;
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.LayoutUpdated += (o, e) => concreteLayoutUpdate = true;
			TestPanel.Children.Add (concrete);

			// StackPanel with two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => concreteLoaded, "ConcreteLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");

				concreteLayoutUpdate = false;
				concrete.Items.Add (stackPanel);
				// Also one extra TextBlock
				concrete.Items.Add (new TextBlock () { Text = "Text2" });
			});
			EnqueueConditional (() => concreteLoaded && stackPanelLoaded && concreteLayoutUpdate, "ConcreteLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");

				// Is 2 because StackPanel is wrapped into 1 ListBoxItem
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add another TextBlock
				concreteLayoutUpdate = false;
				stackPanel.Children.Add (new TextBlock () { Text = "Text3" });
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #2");
				concrete.Items.Add (new TextBlock () { Text = "Text4" });
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #3");
			});
			EnqueueTestComplete ();
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
