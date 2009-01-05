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
using System.Security;
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

		[SecuritySafeCritical]
		public void RaiseAutomationEvent (AutomationEvents events)
		{
			throw new NotImplementedException ();
		}

		public static bool ListenerExists (AutomationEvents events)
		{
			throw new NotImplementedException ();
		}

		protected IRawElementProviderSimple ProviderFromPeer (AutomationPeer peer)
		{
			throw new NotImplementedException ();
		}

		public AutomationPeer GetLabeledBy ()
		{
			throw new NotImplementedException ();
		}

		public string GetName ()
		{
			return GetNameCore ();
		}

		public string GetItemType ()
		{
			throw new NotImplementedException ();
		}

		public List<AutomationPeer> GetChildren ()
		{
			throw new NotImplementedException ();
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
			throw new NotImplementedException ();
		}
		
		public string GetAccessKey ()
		{
			throw new NotImplementedException ();
		}
	
		public AutomationControlType GetAutomationControlType ()
		{
			return GetAutomationControlTypeCore ();
		}
	
		public string GetAutomationId ()
		{
			throw new NotImplementedException ();
		}
	
		public Rect GetBoundingRectangle ()
		{
			throw new NotImplementedException ();
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
			throw new NotImplementedException ();
		}
	
	
		public string GetItemStatus ()
		{
			throw new NotImplementedException ();
		}
	
		public string GetLocalizedControlType ()
		{
			throw new NotImplementedException ();
		}
	
		public AutomationOrientation GetOrientation ()
		{
			throw new NotImplementedException ();
		}
	
		public AutomationPeer GetParent ()
		{
			throw new NotImplementedException ();
		}
	
		public bool HasKeyboardFocus ()
		{
			throw new NotImplementedException ();
		}

		public void InvalidatePeer ()
		{
			throw new NotImplementedException ();
		}
		
		public bool IsContentElement ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsControlElement ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsEnabled ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsKeyboardFocusable ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsOffscreen ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsPassword ()
		{
			throw new NotImplementedException ();
		}
	
		public bool IsRequiredForForm ()
		{
			throw new NotImplementedException ();
		}
	
		protected AutomationPeer PeerFromProvider (IRawElementProviderSimple provider)
		{
			throw new NotImplementedException ();
		}
	
		[SecuritySafeCritical]
		public void RaisePropertyChangedEvent (AutomationProperty property, object oldValue, object newValue)
		{
			throw new NotImplementedException ();
		}
	
		public void SetFocus ()
		{
			throw new NotImplementedException ();
		}
	}
}
