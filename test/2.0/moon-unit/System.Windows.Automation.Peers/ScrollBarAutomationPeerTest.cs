//
// Unit tests for ScrollBarAutomationPeer
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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ScrollBarAutomationPeerTest : RangeBaseAutomationPeerTest {

		public class ScrollBarAutomationPeerPoker : ScrollBarAutomationPeer, FrameworkElementAutomationPeerContract {

			public ScrollBarAutomationPeerPoker (ScrollBar owner)
				: base (owner)
			{
			}

			#region Overridden methods

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			#endregion

			#region Wrapper Methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
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
		[Asynchronous]
		public override void GetChildren ()
		{
			bool scrollbarLoaded = false;
			ScrollBar scrollbar = new ScrollBar ();
			scrollbar.Loaded += (o, e) => scrollbarLoaded = true;
			TestPanel.Children.Add (scrollbar);

			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (scrollbar);

			EnqueueConditional (() => scrollbarLoaded, "ScrollBarLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollbar);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				List<AutomationPeer> children = sbapp.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (5, children.Count, "GetChildren #1");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void StructureChanged_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool scrollbarLoaded = false;
			bool scrollbarLayoutUpdated = false;
			ScrollBar scrollbar = new ScrollBar ();
			scrollbar.Orientation = Orientation.Vertical;
			scrollbar.Loaded += (o, e) => scrollbarLoaded = true;
			TestPanel.Children.Add (scrollbar);

			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (scrollbar);

			EnqueueConditional (() => scrollbarLoaded, "ScrollBarLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollbar);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				List<AutomationPeer> children = sbapp.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (5, children.Count, "GetChildren #1");
				scrollbar.LayoutUpdated += (o, e) => scrollbarLayoutUpdated = true;
				EventsManager.Instance.Reset ();
				scrollbar.Orientation = Orientation.Horizontal;
			});
			EnqueueConditional (() => scrollbarLayoutUpdated, "ScrollBarLayoutUpdated #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollbar);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				AutomationEventTuple tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "AutomationEvents.StructureChanged #0");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (new ScrollBar ());
			Assert.AreEqual (AutomationControlType.ScrollBar, sbapp.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.ScrollBar, sbapp.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClickablePoint ()
		{
			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (new ScrollBar ());

			Assert.IsTrue (double.IsNaN (sbapp.GetClickablePoint ().X), "GetClickablePoint X");
			Assert.IsTrue (double.IsNaN (sbapp.GetClickablePoint ().Y), "GetClickablePoint Y");
			Assert.IsTrue (double.IsNaN (sbapp.GetClickablePointCore_ ().X), "GetClickablePointCore X");
			Assert.IsTrue (double.IsNaN (sbapp.GetClickablePointCore_ ().Y), "GetClickablePointCore Y");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (new ScrollBar ());
			Assert.AreEqual ("ScrollBar", sbapp.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("ScrollBar", sbapp.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetOrientation ()
		{
			ScrollBar scrollbar = new ScrollBar ();
			ScrollBarAutomationPeerPoker sbapp = new ScrollBarAutomationPeerPoker (scrollbar);

			scrollbar.Orientation = Orientation.Horizontal;
			Assert.AreEqual (AutomationOrientation.Horizontal, sbapp.GetOrientation (), "GetOrientation #0");
			Assert.AreEqual (AutomationOrientation.Horizontal, sbapp.GetOrientationCore_ (), "GetOrientationCore #0");

			scrollbar.Orientation = Orientation.Vertical;
			Assert.AreEqual (AutomationOrientation.Vertical, sbapp.GetOrientation (), "GetOrientation #1");
			Assert.AreEqual (AutomationOrientation.Vertical, sbapp.GetOrientationCore_ (), "GetOrientationCore #1");
		}

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

		[TestMethod]
		[Asynchronous]
		public override void TestHasKeyboardFocusAfterPattern ()
		{
			ScrollBar fe = CreateConcreteFrameworkElement () as ScrollBar;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IRangeValueProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IRangeValueProvider) peer.GetPattern (PatternInterface.RangeValue);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.SetValue (.5),
			() => Assert.IsFalse (peer.HasKeyboardFocus (), "#1"));
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ScrollBar ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ScrollBarAutomationPeerPoker (element as ScrollBar);
		}
	}
}
