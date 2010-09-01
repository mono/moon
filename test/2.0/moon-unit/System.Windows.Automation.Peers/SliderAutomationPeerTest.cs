//
// Unit tests for SliderAutomationPeer
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
using System.Collections.Generic;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class SliderAutomationPeerTest : RangeBaseAutomationPeerTest {

		public class SliderPoker  : Slider {
			public SliderPoker () : base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new SliderAutomationPeerPoker (this);
			}
		}

		public class SliderAutomationPeerPoker : SliderAutomationPeer, FrameworkElementAutomationPeerContract {

			public SliderAutomationPeerPoker (Slider owner)
				: base (owner)
			{
			}
			
			#region FrameworkElementAutomationPeerContract Members

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

			public global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
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

			#region Overriden Methods

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public Point GetClickablePointCore_ ()
			{
				return base.GetClickablePointCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			#endregion
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			SliderAutomationPeerPoker peer = new SliderAutomationPeerPoker (new SliderPoker ());
			Assert.AreEqual (AutomationControlType.Slider, peer.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Slider, peer.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			SliderAutomationPeerPoker peer = new SliderAutomationPeerPoker (new SliderPoker ());
			Assert.AreEqual ("Slider", peer.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("Slider", peer.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetClickablePoint ()
		{
			SliderAutomationPeerPoker peer = new SliderAutomationPeerPoker (new SliderPoker ());

			Assert.IsTrue (double.IsNaN (peer.GetClickablePoint ().X), "GetClickablePoint X");
			Assert.IsTrue (double.IsNaN (peer.GetClickablePoint ().Y), "GetClickablePoint Y");
			Assert.IsTrue (double.IsNaN (peer.GetClickablePointCore_ ().X), "GetClickablePointCore X");
			Assert.IsTrue (double.IsNaN (peer.GetClickablePointCore_ ().Y), "GetClickablePointCore Y");
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("This fails on the bots but passes locally. Be careful removing this")]
		public override void GetChildren ()
		{
			bool sliderLoaded = false;
			Slider slider = new Slider ();
			slider.Loaded += (o, e) => sliderLoaded = true;
			TestPanel.Children.Add (slider);

			SliderAutomationPeerPoker sapp = new SliderAutomationPeerPoker (slider);

			EnqueueConditional (() => sliderLoaded, "SliderLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (slider);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				List<AutomationPeer> children = sapp.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (3, children.Count, "GetChildren #1");
			});
			EnqueueTestComplete ();
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
			Slider fe = CreateConcreteFrameworkElement () as Slider;
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
			return new SliderPoker ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new SliderAutomationPeerPoker (element as SliderPoker);
		}
	}
}
