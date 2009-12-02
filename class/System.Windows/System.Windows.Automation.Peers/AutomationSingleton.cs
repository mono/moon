//
// System.Windows.Automation.Peers.AutomationSingleton
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
using System.Reflection;
using System.Collections.Generic;
using System.Windows.Automation;
using System.Windows.Automation.Provider;

namespace System.Windows.Automation.Peers {

	internal sealed class AutomationSingleton {

		public static readonly AutomationSingleton Instance = new AutomationSingleton ();

		private AutomationSingleton ()
		{
		}

		public event EventHandler<AutomationPropertyChangedEventArgs> AutomationPropertyChanged;

		public event EventHandler<AutomationEventEventArgs> AutomationEventRaised;

		public bool AccessibilityEnabled {
			get {
				if (forceAccessibilityEnabled)
					return true;

				return Mono.A11yHelper.AccessibilityEnabled;
			}
		}

		// XXX: This should only be used for automated testing to force
		// events to be fired without the Bridge.
		internal void ForceAccessibilityEnabled ()
		{
			forceAccessibilityEnabled = true;
		}
		
		#region Methods used to raise Automation Events

		public void RaiseAutomationEvent (AutomationPeer peer, AutomationEvents eventId)
		{
			if (!AccessibilityEnabled || peer == null)
				return;

			if (AutomationEventRaised != null)
				AutomationEventRaised (this, new AutomationEventEventArgs (peer, eventId));
		}

		public bool ListenerExists (AutomationEvents events)
		{
			return AccessibilityEnabled;
		}

		public void RaisePropertyChangedEvent (AutomationPeer peer, 
		                                       AutomationProperty property, 
		                                       object oldValue, 
						       object newValue)
		{
			if (!AccessibilityEnabled || peer == null)
				return;

			if (object.Equals (newValue, oldValue))
				return;

			// We are going to raise changes only when the value ACTUALLY CHANGES
			IAutomationCacheProperty cachedProperty = peer.GetCachedProperty (property);
			if (cachedProperty != null) {
				if (object.Equals (newValue, cachedProperty.OldValue))
					return;
				cachedProperty.OldValue = newValue;
			}

			if (AutomationPropertyChanged != null)
				AutomationPropertyChanged (this, 
				                           new AutomationPropertyChangedEventArgs (peer, 
							                                           property, 
												   oldValue, 
												   newValue));
		}

		public void InvalidatePeer (AutomationPeer peer)
		{
			if (!AccessibilityEnabled || peer == null || peer.CacheProperties == null)
				return;

			foreach (IAutomationCacheProperty property in peer.CacheProperties) {
				object oldValue;
				if (property.CompareValues (out oldValue)) 
					peer.RaisePropertyChangedEvent (property.Property, oldValue, property.OldValue);
			}
		}

		#endregion

		private bool forceAccessibilityEnabled;
	}

	internal class AutomationPropertyChangedEventArgs : EventArgs {

		public AutomationPropertyChangedEventArgs (AutomationPeer peer, 
		                                           AutomationProperty property, 
							   object oldValue, 
							   object newValue)
		{
			Peer = peer;
			Property = property;
			OldValue = oldValue;
			NewValue = newValue;
		}

		public AutomationPeer Peer {
			get;
			private set;
		}

		public AutomationProperty Property {
			get;
			private set;
		}

		public object OldValue {
			get;
			private set;
		}

		public object NewValue {
			get;
			private set;
		}
	}

	internal class AutomationEventEventArgs : EventArgs {
		
		public AutomationEventEventArgs (AutomationPeer peer, AutomationEvents eventId)
		{
			Peer = peer;
			Event = eventId;
		}

		public AutomationPeer Peer {
			get;
			private set;
		}

		public AutomationEvents Event {
			get;
			private set;
		}
	}
}

