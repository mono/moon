//
// Unit tests for ButtonBaseAutomationPeer
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
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ButtonBaseAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ButtonBasePoker : ButtonBase {
			public ButtonBasePoker ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ButtonBaseAutomationPeerPoker (this);
			}
		}

		public class ButtonBaseAutomationPeerPoker : ButtonBaseAutomationPeer, FrameworkElementAutomationPeerContract {

			public ButtonBaseAutomationPeerPoker (ButtonBase owner)
				: base (owner)
			{
			}

			#region Overridden methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			#endregion

			#region Wrapper Methods

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
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

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			#endregion
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			ContentTest ((ButtonBase) CreateConcreteFrameworkElement ());
		}

		protected void ContentTest (ButtonBase button)
		{
			Assert.IsTrue (IsContentPropertyElement (), "ButtonElement ContentElement.");

			bool buttonLoaded = false;
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
			Enqueue (() => {} );
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

		[TestMethod]
		public override void GetName ()
		{
			base.GetName ();

			// LAMESPEC: MSDN: A string that contains the name, minus the accelerator key. 

			ButtonBase button = CreateConcreteFrameworkElement () as ButtonBase;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
			Assert.IsNotNull (peer, "IsNotNull #0");

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #0");

			button.Content = "Hello I'm button";
			Assert.AreEqual ("Hello I'm button", peer.GetName (), "GetName #1");

			button.Content = "Hello I'm &button";
			Assert.AreEqual ("Hello I'm &button", peer.GetName (), "GetName #2");

			button.Content = "Hello I'm &&button";
			Assert.AreEqual ("Hello I'm &&button", peer.GetName (), "GetName #3");

			button.Content = null;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #4");

			TextBox textbox = new TextBox ();
			textbox.Text = "I'm textbox";
			button.Content = textbox;

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #5");

			TextBlock textblock = new TextBlock ();
			textblock.Text = "I'm textblock";
			button.Content = textblock;

			Assert.AreEqual ("I'm textblock", peer.GetName (), "GetName #6");

			button.Content = "I'm a button";
			Assert.AreEqual ("I'm a button", peer.GetName (), "GetName #7");

			// Now usin a stack panel with a textblock
			StackPanel panel = new StackPanel ();
			panel.Children.Add (new TextBlock () { Text = "Textblock in Stackpanel1" });

			button.Content = panel;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #8");

			panel.Children.Add (new TextBlock () { Text = "Textblock in Stackpanel2" });
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #9");

			button.Content = "What's up?";
			Assert.AreEqual ("What's up?", peer.GetName (), "GetName #10");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ButtonBase fe = CreateConcreteFrameworkElement () as ButtonBase;
			TextBlock textblock = new TextBlock () { Text = "Textblock" };
			fe.Content = textblock;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (fe,
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#0");
			},
			() => {
				EventsManager.Instance.Reset ();
				textblock.Text = "Hi";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#1");
				Assert.AreEqual ("Hi", (string) tuple.NewValue, "#2");
				Assert.AreEqual ("Textblock", tuple.OldValue, "#3");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, "Attached Name");
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#4");
				Assert.AreEqual ("Attached Name", (string) tuple.NewValue, "#5");
				Assert.AreEqual ("Hi", tuple.OldValue, "#6");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, "Name");
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#7");
				Assert.AreEqual ("Name", (string) tuple.NewValue, "#8");
				Assert.AreEqual ("Attached Name", (string) tuple.OldValue, "#9");
			},
			() => {
				// Even if change TextBlock.Text (our current Content) the value will be the same
				EventsManager.Instance.Reset ();
				textblock.Text = "New value";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#10");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, null);
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#11");
				Assert.AreEqual ("New value", (string) tuple.NewValue, "#12");
				Assert.AreEqual ("Name", (string) tuple.OldValue, "#13");
			},
			() => {
				EventsManager.Instance.Reset ();
				textblock.Text = "What's up?";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#14");
				Assert.AreEqual ("What's up?", (string) tuple.NewValue, "#15");
				Assert.AreEqual ("New value", (string) tuple.OldValue, "#16");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Content = "Hola";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#17");
				Assert.AreEqual ("Hola", (string) tuple.NewValue, "#18");
				Assert.AreEqual ("What's up?", (string) tuple.OldValue, "#19");
			});
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ButtonBasePoker ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ButtonBaseAutomationPeerPoker (element as ButtonBasePoker);
		}

		protected void Test_InvokeProvider_Events (ButtonBase element)
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);

			IInvokeProvider provider
				= (IInvokeProvider) peer.GetPattern (PatternInterface.Invoke);
			Assert.IsNotNull (provider, "GetPattern #0");

			EventsManager.Instance.Reset ();

			AutomationEventTuple tuple
				= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.InvokePatternOnInvoked);
			Assert.IsNull (tuple, "GetAutomationEventFrom #0");

			provider.Invoke ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.InvokePatternOnInvoked);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #1");
		}

		protected void Test_InvokeProvider_Invoke (ButtonBase button)
		{
			AutomationPeer peer
				= FrameworkElementAutomationPeer.CreatePeerForElement (button);
			IInvokeProvider invoke = peer.GetPattern (PatternInterface.Invoke) as IInvokeProvider;
			Assert.IsNotNull (invoke, "InvokeProvider is null");

			invoke.Invoke ();
			button.IsEnabled = false;

			Assert.Throws<ElementNotEnabledException> (() => invoke.Invoke ());

			button.IsEnabled = true;
			invoke.Invoke ();
		}

	}
}
