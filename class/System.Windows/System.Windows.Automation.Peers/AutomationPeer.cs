//
// System.Windows.Automation.Peers.AutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008,2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation.Provider;

namespace System.Windows.Automation.Peers {

	[MonoTODO]
	public abstract class AutomationPeer : DependencyObject {

		protected AutomationPeer ()
		{
		}

		public AutomationPeer EventsSource {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}

		public void RaiseAutomationEvent (AutomationEvents events)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO ("always return false since this is used by MS controls")]
		public static bool ListenerExists (AutomationEvents events)
		{
			return false;
		}

		[MonoTODO ("right now it only wraps the peer into a IRawElementProviderSimple")]
		protected IRawElementProviderSimple ProviderFromPeer (AutomationPeer peer)
		{
			return new IRawElementProviderSimple (peer);
		}

		public AutomationPeer GetLabeledBy ()
		{
			return GetLabeledByCore ();
		}

		public string GetName ()
		{
			AutomationPeer labeledByPeer = GetLabeledBy ();
			if (labeledByPeer != null)
				return labeledByPeer.GetName () ?? string.Empty;
			else
				return GetNameCore ();
		}

		public string GetItemType ()
		{
			return GetItemTypeCore ();
		}

		public List<AutomationPeer> GetChildren ()
		{
			return GetChildrenCore ();
		}

		protected abstract AutomationPeer GetLabeledByCore ();
		protected abstract string GetNameCore ();
		protected abstract string GetItemTypeCore ();
		protected abstract List<AutomationPeer> GetChildrenCore ();
		protected abstract string GetAcceleratorKeyCore ();
		protected abstract string GetAccessKeyCore ();
		protected abstract AutomationControlType GetAutomationControlTypeCore ();
		protected abstract string GetAutomationIdCore ();
		protected abstract Rect GetBoundingRectangleCore ();
		protected abstract string GetClassNameCore ();
		protected abstract bool IsOffscreenCore ();	
		protected abstract bool IsPasswordCore ();
		protected abstract Point GetClickablePointCore ();
		protected abstract string GetItemStatusCore ();
		protected abstract string GetHelpTextCore ();
		protected abstract AutomationOrientation GetOrientationCore ();
		protected abstract bool HasKeyboardFocusCore ();
		protected abstract bool IsContentElementCore ();
		protected abstract bool IsControlElementCore ();
		protected abstract bool IsEnabledCore ();
		protected abstract bool IsKeyboardFocusableCore ();
		protected abstract bool IsRequiredForFormCore ();
		protected abstract string GetLocalizedControlTypeCore ();
		protected abstract void SetFocusCore ();

		public abstract object GetPattern (PatternInterface patternInterface);
		
		public string GetAcceleratorKey ()
		{
			return GetAcceleratorKeyCore ();
		}
		
		public string GetAccessKey ()
		{
			return GetAccessKeyCore ();
		}
	
		public AutomationControlType GetAutomationControlType ()
		{
			return GetAutomationControlTypeCore ();
		}
	
		public string GetAutomationId ()
		{
			return GetAutomationIdCore ();
		}
	
		public Rect GetBoundingRectangle ()
		{
			return GetBoundingRectangleCore ();
		}
	
		public string GetClassName ()
		{
			return GetClassNameCore ();
		}
	
		public Point GetClickablePoint ()
		{
			return GetClickablePointCore ();
		}
	
		public string GetHelpText ()
		{
			return GetHelpTextCore ();
		}
	
		public string GetItemStatus ()
		{
			return GetItemStatusCore ();
		}
	
		public string GetLocalizedControlType ()
		{
			return GetLocalizedControlTypeCore ();
		}
	
		public AutomationOrientation GetOrientation ()
		{
			return GetOrientationCore ();
		}
	
		public AutomationPeer GetParent ()
		{
			return GetParent ();
		}
	
		public bool HasKeyboardFocus ()
		{
			return HasKeyboardFocusCore ();
		}

		public void InvalidatePeer ()
		{
			throw new NotImplementedException ();
		}
		
		public bool IsContentElement ()
		{
			return IsContentElementCore ();
		}
	
		public bool IsControlElement ()
		{
			return IsControlElementCore ();
		}
	
		public bool IsEnabled ()
		{
			return IsEnabledCore ();
		}
	
		public bool IsKeyboardFocusable ()
		{
			return IsKeyboardFocusableCore ();
		}
	
		public bool IsOffscreen ()
		{
			return IsOffscreenCore ();
		}
	
		public bool IsPassword ()
		{
			return IsPasswordCore ();
		}
	
		public bool IsRequiredForForm ()
		{
			return IsRequiredForFormCore ();
		}
	
		[MonoTODO ("right now it only unwrap the peer from a IRawElementProviderSimple")]
		protected AutomationPeer PeerFromProvider (IRawElementProviderSimple provider)
		{
			// SL2 will NRE too if 'provider' is null
			return provider.AutomationPeer;
		}
	
		public void RaisePropertyChangedEvent (AutomationProperty property, object oldValue, object newValue)
		{
			throw new NotImplementedException ();
		}
	
		public void SetFocus ()
		{
			SetFocusCore ();
		}
	}
}
