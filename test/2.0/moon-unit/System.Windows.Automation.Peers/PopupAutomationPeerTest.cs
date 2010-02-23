//
// Unit tests for ButtonBaseAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
	public class PopupAutomationPeerTest : FrameworkElementAutomationPeerTest {

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Popup popup = new Popup ();
			AutomationPeer peer = null;
			StackPanel stackPanel = null;

			CreateAsyncTest (popup,
			() => {
				// StackPanel and two TextBlocks
				stackPanel = new StackPanel ();
				stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
				stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			},
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (popup);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				popup.Child = stackPanel;
			},
			() => {
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Window, peer.GetAutomationControlType (), "GetAutomationControlType");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("Popup", peer.GetClassName (), "GetClassName");
		}

		[TestMethod]
		[Asynchronous]
		public override void IsOffScreen ()
		{
			Popup popup = new Popup ();
			AutomationPeer peer = null;

			CreateAsyncTest (popup,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (popup);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsTrue (peer.IsOffscreen (), "#0");
			},
			() => popup.IsOpen = true,
			() => Assert.IsFalse (peer.IsOffscreen (), "#1"),
			() => popup.IsOpen = false,
			() => Assert.IsTrue (peer.IsOffscreen (), "#2"));
		}

		[TestMethod]
		[Asynchronous]
		public override void IsOffScreen_Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			Popup fe = CreateConcreteFrameworkElement () as Popup;
			fe.SetValue (Canvas.TopProperty, (double) 10);
			fe.SetValue (Canvas.LeftProperty, (double) 30);
			fe.SetValue (Canvas.WidthProperty, (double) 150);
			fe.SetValue (Canvas.HeightProperty, (double) 230);

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (fe,
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNull (tuple, "#0");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #0");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Visible;
			},
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNull (tuple, "#1");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #1");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Collapsed;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNotNull (tuple, "#2");
				Rect newValue = (Rect) tuple.NewValue;

				Assert.AreEqual (0, newValue.X, "#4");
				Assert.AreEqual (0, newValue.Y, "#5");
				Assert.AreEqual (0, newValue.Width, "#6");
				Assert.AreEqual (0, newValue.Height, "#7");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #2");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Visible;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNull (tuple, "#8");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.IsOpen = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNotNull (tuple, "#9");
				Rect newValue = (Rect) tuple.NewValue;
				Rect oldValue = (Rect) tuple.OldValue;

				Assert.AreNotEqual (newValue.X, oldValue.X, "#10");
				Assert.AreNotEqual (newValue.Y, oldValue.Y, "#11");
				Assert.AreNotEqual (newValue.Width, oldValue.Width, "#12");
				Assert.AreNotEqual (newValue.Height, oldValue.Height, "#13");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #3");
				Assert.IsFalse ((bool) tuple.NewValue, "IsOffscreen #4");
				Assert.IsTrue ((bool) tuple.OldValue, "IsOffscreen #5");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.IsOpen = false;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNotNull (tuple, "#14");
				Rect newValue = (Rect) tuple.NewValue;
				Rect oldValue = (Rect) tuple.OldValue;

				Assert.AreNotEqual (newValue.X, oldValue.X, "#15");
				Assert.AreNotEqual (newValue.Y, oldValue.Y, "#16");
				Assert.AreNotEqual (newValue.Width, oldValue.Width, "#17");
				Assert.AreNotEqual (newValue.Height, oldValue.Height, "#18");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #6");
				Assert.IsTrue ((bool) tuple.NewValue, "IsOffscreen #7");
				Assert.IsFalse ((bool) tuple.OldValue, "IsOffscreen #8");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void IsOffScreen_Event1 ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			Canvas parent0 = new Canvas ();
			parent0.Height = 100;
			parent0.Width = 100;

			StackPanel parent1 = new StackPanel ();
			parent1.Height = 200;
			parent1.Width = 100;

			parent0.Children.Add (parent1);

			FrameworkElement fe = CreateConcreteFrameworkElement ();

			fe.SetValue (Canvas.WidthProperty, (double) 150);
			fe.SetValue (Canvas.HeightProperty, (double) 230);
			parent1.Children.Add (fe);

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (parent0,
			() => {
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Visible;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #4");
			},
			() => {
				// Testing when our parent is not Visible
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Collapsed;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #1");
			},
			() => {
				EventsManager.Instance.Reset ();
				parent1.Visibility = Visibility.Collapsed;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #4");
			},
			() => {
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Visible;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #5");
			},
			() => {
				EventsManager.Instance.Reset ();
				parent1.Visibility = Visibility.Visible;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #7");
			});
		}

		public override void Null ()
		{
			// We can't create an instance of the peer class because is not public API
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new Popup ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return null;
		}

	}
}
