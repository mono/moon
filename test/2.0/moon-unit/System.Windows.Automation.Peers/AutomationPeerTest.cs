//
// Unit tests for AutomationPeer
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
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class AutomationPeerTest {

		[TestMethod]
		public void ListenerExists ()
		{
			Assert.IsFalse (AutomationPeer.ListenerExists ((AutomationEvents) Int32.MinValue), "Bad value");
		}

 		[TestMethod]
 		public void EventsSource ()
 		{
 			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();
 			ConcreteAutomationPeer peer = new ConcreteAutomationPeer (element);
 			Assert.IsNull (peer.EventsSource, "#0");
 
 			ConcreteAutomationPeer another = new ConcreteAutomationPeer (element);
 			peer.EventsSource = another;
 			Assert.AreSame (another, peer.EventsSource, "#1");
 		}
 
 		[TestMethod]
 		public void ProviderFromPeer ()
 		{
 			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();
 			ConcreteAutomationPeer peer = new ConcreteAutomationPeer (element);
 
 			IRawElementProviderSimple proxy0 = peer.ProviderFromPeer_ (peer);
 			IRawElementProviderSimple proxy1 = peer.ProviderFromPeer_ (peer);
 
 			Assert.AreNotSame (proxy0, proxy1, "Should be different.");
 
 			Assert.IsNull(peer.ProviderFromPeer_(null), "#0");
 		}
 
 		[TestMethod]
 		public void PeerFromProvider ()
 		{
 			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();
 			ConcreteAutomationPeer peer = new ConcreteAutomationPeer (element);
 
 			IRawElementProviderSimple proxy0 = peer.ProviderFromPeer_ (peer);
 			IRawElementProviderSimple proxy1 = peer.ProviderFromPeer_ (peer);
 
 			AutomationPeer peer0 = peer.PeerFromProvider_ (proxy0);
 			AutomationPeer peer1 = peer.PeerFromProvider_ (proxy1);
 
 			Assert.AreSame (peer0, peer1, "Should not be different.");
 
 			Assert.Throws<NullReferenceException>(delegate {
 				peer.PeerFromProvider_ (null);
 			}, "null");
 		}

		[TestMethod]
 		public void InvalidatePeer ()
 		{
 			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
			peer.InvalidatePeer ();
			// FIXME: We need to add more tests here, but we also need the a11y enabled by FF 
			// to track changes
 		}
 
 		class ConcreteFrameworkElement : FrameworkElement {
 
 			protected override AutomationPeer OnCreateAutomationPeer ()
 			{
 				return new ConcreteAutomationPeer (this);
 			}
 		}
 
 		// We can't subclass AutomationPeer in SL2 because calling base()
 		// throws a NIE, so we are subclassing FrameworkElement and only
 		// testing non-abstract methods defined in AutomationPeer.
 		class ConcreteAutomationPeer : FrameworkElementAutomationPeer {
 
 			public ConcreteAutomationPeer (ConcreteFrameworkElement owner) : base (owner)
 			{
 			}
 
 			public AutomationPeer PeerFromProvider_ (IRawElementProviderSimple provider)
 			{
 				return PeerFromProvider (provider);
 			}
 
 			public IRawElementProviderSimple ProviderFromPeer_ (AutomationPeer peer)
 			{
 				return ProviderFromPeer (peer);
 			}
 
 			#region AutomationPeer Overriden methods.
 
 			protected override string GetAcceleratorKeyCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetAccessKeyCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override AutomationControlType GetAutomationControlTypeCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetAutomationIdCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override global::System.Windows.Rect GetBoundingRectangleCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetClassNameCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override global::System.Windows.Point GetClickablePointCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetHelpTextCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetItemStatusCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetItemTypeCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override AutomationPeer GetLabeledByCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetLocalizedControlTypeCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override string GetNameCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override AutomationOrientation GetOrientationCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			public override object GetPattern(PatternInterface patternInterface)
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool HasKeyboardFocusCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsContentElementCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsControlElementCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsEnabledCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsKeyboardFocusableCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsOffscreenCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsPasswordCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override bool IsRequiredForFormCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			protected override void SetFocusCore()
 			{
 				throw new NotImplementedException();
 			}
 
 			#endregion
 		}

	}
}
