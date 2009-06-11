//
// Unit tests for ButtonAutomationPeer
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
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ButtonAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ButtonConcrete : Button {
			public ButtonConcrete ()
				: base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ButtonAutomationPeerPoker (this);
			}
		}

		public class ButtonAutomationPeerPoker : ButtonAutomationPeer, FrameworkElementAutomationPeerContract {

			public ButtonAutomationPeerPoker (Button owner)
				: base (owner)
			{
			}

			#region Overridden Methods

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
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

			public global::System.Windows.Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public global::System.Windows.Point GetClickablePointCore_ ()
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
		public override void GetAutomationControlType ()
		{
			ButtonAutomationPeerPoker bapp = new ButtonAutomationPeerPoker (new Button ());
			Assert.AreEqual (AutomationControlType.Button, bapp.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Button, bapp.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("Button", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("Button", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetPattern ()
		{
			ButtonAutomationPeerPoker bap = new ButtonAutomationPeerPoker (new Button ());

			Assert.IsNull (bap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (bap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (bap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (bap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (bap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (bap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (bap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (bap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (bap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (bap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (bap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (bap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (bap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (bap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (bap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (bap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (bap.GetPattern (PatternInterface.Invoke), "Invoke");
		}

		[TestMethod]
		public void IInvokeProvider_Invoke ()
		{
			// FIXME: Implement when MoonAtkBridge is ready
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ButtonElement ContentElement.");

			bool buttonLoaded = false;
			Button button = CreateConcreteFrameworkElement () as Button;
			button.Loaded += (o, e) => buttonLoaded = true;
			TestPanel.Children.Add (button);

			// StackPanel and two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => buttonLoaded, "ButtonLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				button.Content = stackPanel;
			});
			EnqueueConditional (() => buttonLoaded && stackPanelLoaded, "ButtonLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
			EnqueueTestComplete ();
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ButtonConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ButtonAutomationPeerPoker (element as ButtonConcrete);
		}
	}
}
