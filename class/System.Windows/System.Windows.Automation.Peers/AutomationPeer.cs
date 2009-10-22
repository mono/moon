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
using System.Linq;
using System.Collections.Generic;
using System.Windows.Automation.Provider;

namespace System.Windows.Automation.Peers {

	public abstract class AutomationPeer : DependencyObject {

		protected AutomationPeer ()
		{
		}

		public AutomationPeer EventsSource {
			get;
			set;
		}

		public void RaiseAutomationEvent (AutomationEvents events)
		{
			AutomationSingleton.Instance.RaiseAutomationEvent (this, events);
		}

		public static bool ListenerExists (AutomationEvents events)
		{
			return AutomationSingleton.Instance.ListenerExists (events);
		}

		protected IRawElementProviderSimple ProviderFromPeer (AutomationPeer peer)
		{
			if (peer == null)
				return null;
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
			return GetParentCore ();
		}
	
		public bool HasKeyboardFocus ()
		{
			return HasKeyboardFocusCore ();
		}

		public void InvalidatePeer ()
		{
			AutomationSingleton.Instance.InvalidatePeer (this);
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
	
		protected AutomationPeer PeerFromProvider (IRawElementProviderSimple provider)
		{
			// SL2 will NRE too if 'provider' is null
			return provider.AutomationPeer;
		}
	
		public void RaisePropertyChangedEvent (AutomationProperty property, object oldValue, object newValue)
		{
			AutomationSingleton.Instance.RaisePropertyChangedEvent (this, property, oldValue, newValue);
		}
	
		public void SetFocus ()
		{
			SetFocusCore ();
		}

		// Overriden by FrameworkElementAutomationPeer to return Parent peer using recursion
		internal virtual AutomationPeer GetParentCore ()
		{
			return null;
		}

		// Method used to cache main properties to RaisePropertyChanged when calling 
		// InvalidatePeer. This method is also called by FrameworkElementAutomationPeer.CreatePeerForElement
		internal void CacheMainProperties ()
		{
			// We are keeping a list of cached properties to raise events depending on the 
			// accessibility status, because the bridge is loaded by request, ie,
			// when an AT requests a11y information is loaded

			if (cacheProperties == null) {
				// Main properties defined in AutomationElementIdentifiers static fields
				cacheProperties = new IAutomationCacheProperty[] {
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.AcceleratorKeyProperty, 
					                                         OldValue = GetAcceleratorKey (), 
									         Delegate =  GetAcceleratorKey },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.AccessKeyProperty, 
					                                         OldValue = GetAccessKey (), 
									         Delegate = GetAccessKey },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.AutomationIdProperty, 
					                                         OldValue = GetAutomationId (), 
									         Delegate = GetAutomationId },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.ClassNameProperty, 
					                                         OldValue = GetClassName (), 
									         Delegate = GetClassName },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.HelpTextProperty, 
					                                         OldValue = GetHelpText (), 
									         Delegate = GetHelpText },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.ItemStatusProperty, 
					                                         OldValue = GetItemStatus (), 
									         Delegate = GetItemStatus },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.ItemTypeProperty,
					                                         OldValue = GetItemType (), 
									         Delegate = GetItemType },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.NameProperty,
					                                         OldValue = GetName (), 
									         Delegate = GetName },
					new AutomationCacheProperty<string> () { Property = AutomationElementIdentifiers.LocalizedControlTypeProperty,
					                                         OldValue = GetLocalizedControlType (), 
									         Delegate = GetLocalizedControlType },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.HasKeyboardFocusProperty,
					                                       OldValue = HasKeyboardFocus (), 
									       Delegate = HasKeyboardFocus },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsOffscreenProperty,
					                                       OldValue = IsOffscreen (), 
									       Delegate = IsOffscreen },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsContentElementProperty,
					                                       OldValue = IsContentElement (), 
									       Delegate = IsContentElement },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsControlElementProperty,
					                                       OldValue = IsControlElement (), 
									       Delegate = IsControlElement },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsEnabledProperty,
					                                       OldValue = IsEnabled (), 
									       Delegate = IsEnabled },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsPasswordProperty,
					                                       OldValue = IsPassword (), 
									       Delegate = IsPassword },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsRequiredForFormProperty,
					                                       OldValue = IsRequiredForForm (), 
									       Delegate = IsRequiredForForm },
					new AutomationCacheProperty<bool> () { Property = AutomationElementIdentifiers.IsKeyboardFocusableProperty,
					                                       OldValue = IsKeyboardFocusable (), 
									       Delegate = IsKeyboardFocusable },
					new AutomationCacheProperty<Rect> () { Property = AutomationElementIdentifiers.BoundingRectangleProperty,
					                                       OldValue = GetBoundingRectangle (), 
									       Delegate = GetBoundingRectangle },
					new AutomationCacheProperty<Point> () { Property = AutomationElementIdentifiers.ClickablePointProperty,
					                                        OldValue = GetClickablePoint (), 
									        Delegate = GetClickablePoint },
					new AutomationCachePeerProperty () { Property = AutomationElementIdentifiers.LabeledByProperty,
					                                     OldValue = GetLabeledBy (), 
									     Delegate = GetLabeledBy },
					new AutomationCacheProperty<AutomationOrientation> () { Property = AutomationElementIdentifiers.OrientationProperty,
					                                                        OldValue = GetOrientation (), 
										                Delegate = GetOrientation },
					new AutomationCacheProperty<AutomationControlType> () { Property = AutomationElementIdentifiers.ControlTypeProperty,
					                                                        OldValue = GetAutomationControlType (), 
										                Delegate = GetAutomationControlType }
				};
			}
		}

		internal IAutomationCacheProperty GetCachedProperty (AutomationProperty property)
		{
			CacheMainProperties ();

			return (from p in cacheProperties where p.Property == property select p).FirstOrDefault();
		}

		internal IEnumerable<IAutomationCacheProperty> CacheProperties {
			get { return cacheProperties; }
		}

		internal virtual void RaiseNameChanged ()
		{
		}
		
		private IAutomationCacheProperty []cacheProperties;
	}
}


