//
// Unit tests for ToggleButtonAutomationPeer
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
	public class ToggleButtonAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ToggleButtonPoker : ToggleButton
		{
			public ToggleButtonPoker ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ToggleButtonAutomationPeerPoker (this);
			}
		}

		public class ToggleButtonAutomationPeerPoker : ToggleButtonAutomationPeer, FrameworkElementAutomationPeerContract {

			public ToggleButtonAutomationPeerPoker (ToggleButton owner)
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

		[TestMethod]
		public override void GetName ()
		{
			ToggleButton toggleButton = CreateConcreteFrameworkElement () as ToggleButton;
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (toggleButton);

			Assert.AreEqual (String.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetName");

			string name = "Yarr!";
			toggleButton.Content = new TextBlock {Text = name};
			Assert.AreEqual (name, feap.GetName (), "GetName #1");
			Assert.AreEqual (name, feap.GetNameCore_ (), "GetNameCore #1");

			name = "En Garde!";
			toggleButton.Content = name;
			Assert.AreEqual (name, feap.GetName (), "GetName #2");
			Assert.AreEqual (name, feap.GetNameCore_ (), "GetNameCore #2");

			toggleButton.Content = String.Empty;
			Assert.AreEqual (String.Empty, feap.GetName (), "GetName #2");
			Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetNameCore #2");

			toggleButton.Content = null;
			Assert.AreEqual (String.Empty, feap.GetName (), "GetName #3");
			Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetNameCore #3");
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
			Assert.IsNull (peer.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (peer.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (peer.GetPattern (PatternInterface.Window), "Window");
			Assert.IsNull (peer.GetPattern (PatternInterface.Value), "Value");

			Assert.IsNotNull (peer.GetPattern (PatternInterface.Toggle), "Toggle");
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
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Button, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Button, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ToggleButton ContentElement.");

			bool tbLoaded = false;
			ToggleButton toggleButton = CreateConcreteFrameworkElement () as ToggleButton;
			toggleButton.Loaded += (o, e) => tbLoaded = true;
			TestPanel.Children.Add (toggleButton);

			// StackPanel and two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => tbLoaded, "ToggleButton Loaded #0");
			Enqueue (() => {
				AutomationPeer peer
					= FrameworkElementAutomationPeer.CreatePeerForElement (
						toggleButton);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				toggleButton.Content = stackPanel;
			});
			EnqueueConditional (() => tbLoaded && stackPanelLoaded, "ToggleButton Loaded #1");
			Enqueue (() => {
				AutomationPeer peer
					= FrameworkElementAutomationPeer.CreatePeerForElement (
						toggleButton);
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
		[Asynchronous]
		public virtual void TestHasKeyboardFocusAfterPattern ()
		{
			ToggleButton fe = CreateConcreteFrameworkElement ()
				as ToggleButton;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IToggleProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IToggleProvider) peer.GetPattern (PatternInterface.Toggle);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.Toggle (),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#1"));
		}

		#region IToggleProvider tests

		[TestMethod]
		public virtual void ToggleProvider_ToggleState ()
		{
			ToggleButton toggleButton = CreateConcreteFrameworkElement ()
				as ToggleButton;
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (toggleButton);

			IToggleProvider toggleProvider = feap as IToggleProvider;

                        // Test two-state toggling
			toggleButton.IsThreeState = false;

			toggleButton.IsChecked = false;
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Start two-state toggle: Unchecked");

			toggleButton.IsChecked = true;
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "First two-state toggle: Checked");

			toggleButton.IsChecked = false;
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Second two-state toggle: Unchecked");

			toggleButton.IsChecked = true;
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Third two-state toggle: Checked");

                        // Test three-state toggling
                        toggleButton.IsThreeState = true;

			toggleButton.IsChecked = true;
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Start three-state Checked");

			toggleButton.IsChecked = false;
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "First three-state toggle: Unchecked");

			toggleButton.IsChecked = null;
                        Assert.AreEqual (ToggleState.Indeterminate, toggleProvider.ToggleState,
			                 "Second three-state toggle: Intermediate");

			toggleButton.IsChecked = true;
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Third three-state toggle: Checked");
		}

		[TestMethod]
		public virtual void ToggleProvider_Toggle ()
		{
			ToggleButton toggleButton = CreateConcreteFrameworkElement () as ToggleButton;
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (toggleButton);

			toggleButton.IsEnabled = false;

			IToggleProvider toggleProvider = feap as IToggleProvider;

			try {
				toggleProvider.Toggle ();
				Assert.Fail ("Should throw ElementNotEnabledException");
			} catch (ElementNotEnabledException) { }

			toggleButton.IsEnabled = true;

			// TODO: Test eventing

                        // Test two-state toggling
			toggleButton.IsThreeState = false;

			toggleButton.IsChecked = false;
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Start two-state toggle: Unchecked");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "First two-state toggle: Checked");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Second two-state toggle: Unchecked");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Third two-state toggle: Checked");

                        // Test three-state toggling
                        toggleButton.IsThreeState = true;

			toggleButton.IsChecked = true;
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Start three-state Checked");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.Indeterminate, toggleProvider.ToggleState,
			                 "First three-state toggle: Indeterminate");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
			                 "Second three-state Off: Intermediate");

			toggleProvider.Toggle ();
                        Assert.AreEqual (ToggleState.On, toggleProvider.ToggleState,
			                 "Third three-state toggle: Checked");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void ToggleProvider_ToggleEvents ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ToggleButton toggleButton = CreateConcreteFrameworkElement () as ToggleButton;
			AutomationPeer peer
				= FrameworkElementAutomationPeer.CreatePeerForElement (toggleButton);
			AutomationPropertyEventTuple tuple = null;
			IToggleProvider toggleProvider = (IToggleProvider) peer;

			CreateAsyncTest (toggleButton,
			() => {
				EventsManager.Instance.Reset ();
				toggleButton.IsThreeState = false;
				toggleButton.IsChecked = false;
			},
                        // Test two-state toggling
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNull (tuple, "#0");
				Assert.AreEqual (ToggleState.Off, toggleProvider.ToggleState,
				                "Start two-state toggle: Unchecked");
			},
			() => {
				EventsManager.Instance.Reset ();
				toggleProvider.Toggle ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#1");
				Assert.AreEqual (ToggleState.Off,
				                 (ToggleState) tuple.OldValue,
						 "#2");
				Assert.AreEqual (ToggleState.On,
				                 (ToggleState) tuple.NewValue,
						 "#3");
			},
			() => {
				EventsManager.Instance.Reset ();
				toggleProvider.Toggle ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#4");
				Assert.AreEqual (ToggleState.On,
				                 (ToggleState) tuple.OldValue,
						 "#5");
				Assert.AreEqual (ToggleState.Off,
				                 (ToggleState) tuple.NewValue,
						 "#6");
			},
                        // Test three-state toggling
			() => {
				EventsManager.Instance.Reset ();
				toggleButton.IsThreeState = true;
				toggleButton.IsChecked = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#7");
				Assert.AreEqual (ToggleState.Off,
				                 (ToggleState) tuple.OldValue,
						 "#8");
				Assert.AreEqual (ToggleState.On,
				                 (ToggleState) tuple.NewValue,
						 "#9");
			},
			() => {
				EventsManager.Instance.Reset ();
				toggleProvider.Toggle ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#10");
				Assert.AreEqual (ToggleState.On,
				                 (ToggleState) tuple.OldValue,
						 "#11");
				Assert.AreEqual (ToggleState.Indeterminate,
				                 (ToggleState) tuple.NewValue,
						 "#12");
			},
			() => {
				EventsManager.Instance.Reset ();
				toggleProvider.Toggle ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#13");
				Assert.AreEqual (ToggleState.Indeterminate,
				                 (ToggleState) tuple.OldValue,
						 "#14");
				Assert.AreEqual (ToggleState.Off,
				                 (ToggleState) tuple.NewValue,
						 "#15");
			},
			() => {
				EventsManager.Instance.Reset ();
				toggleProvider.Toggle ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       TogglePatternIdentifiers.ToggleStateProperty);
				Assert.IsNotNull (tuple, "#16");
				Assert.AreEqual (ToggleState.Off,
				                 (ToggleState) tuple.OldValue,
						 "#17");
				Assert.AreEqual (ToggleState.On,
				                 (ToggleState) tuple.NewValue,
						 "#18");
			});
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ToggleButtonPoker ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ToggleButtonAutomationPeerPoker (element as ToggleButton);
		}
	}
}

