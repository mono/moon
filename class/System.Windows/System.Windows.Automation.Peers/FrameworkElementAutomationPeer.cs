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
// Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers {

	public class FrameworkElementAutomationPeer : AutomationPeer {

		private UIElement owner;

		public FrameworkElementAutomationPeer (FrameworkElement owner)
		{
			if (owner == null)
				throw new NullReferenceException ("owner");
			this.owner = owner;
		}

		public UIElement Owner {
			get { return owner; }
		}

		protected override string GetNameCore ()
		{
			return owner.GetValue (AutomationProperties.NameProperty) as string ?? string.Empty;
		}

		protected override string GetItemTypeCore ()
		{
			return owner.GetValue (AutomationProperties.ItemTypeProperty) as string ?? string.Empty;
		}

		protected override AutomationPeer GetLabeledByCore ()
		{
			UIElement labeledBy = owner.GetValue (AutomationProperties.LabeledByProperty) as UIElement;
			if (labeledBy != null)
				return FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);
			else
				return null;
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			return null;
		}

		public override object GetPattern (PatternInterface pattern)
		{
			return null;
		}
		
		public static AutomationPeer FromElement (UIElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			return element.AutomationPeer;
		}
		
		protected override string GetAcceleratorKeyCore ()
		{
			return owner.GetValue (AutomationProperties.AcceleratorKeyProperty) as string ?? string.Empty;
		}
		
		protected override string GetAccessKeyCore ()
		{
			return owner.GetValue (AutomationProperties.AccessKeyProperty) as string ?? string.Empty;
		}
		
		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Custom;
		}
		
		protected override string GetAutomationIdCore ()
		{
			return owner.GetValue (AutomationProperties.AutomationIdProperty) as string ?? string.Empty;
		}
		
		protected override Rect GetBoundingRectangleCore ()
		{
			return new Rect (0, 0, 0, 0);
		}
		
		protected override string GetClassNameCore ()
		{
			return owner.GetValue (AutomationProperties.NameProperty) as string ?? string.Empty;
		}
		
		protected override Point GetClickablePointCore ()
		{
			return new Point (0, 0);
		}
		
		protected override string GetHelpTextCore ()
		{
			return owner.GetValue (AutomationProperties.HelpTextProperty) as string ?? string.Empty;
		}
		
		protected override string GetItemStatusCore ()
		{
			return owner.GetValue (AutomationProperties.ItemStatusProperty) as string ?? string.Empty;
		}
		
		protected override string GetLocalizedControlTypeCore ()
		{
			// LAMESPEC: http://msdn.microsoft.com/en-us/library/ms743581.aspx
			// "CamelCase" literal values should be "camel case", not "camelcase"
			return GetAutomationControlType ().ToString ().ToLower ();
		}
		
		protected override AutomationOrientation GetOrientationCore ()
		{
			return AutomationOrientation.None;
		}
		
		protected override bool HasKeyboardFocusCore ()
		{
			return false;
		}
		
		protected override bool IsContentElementCore ()
		{
			return true;
		}
		
		protected override bool IsControlElementCore ()
		{
			return true;
		}
		
		protected override bool IsEnabledCore ()
		{
			return true;
		}
		
		protected override bool IsKeyboardFocusableCore ()
		{
			return false;
		}
		
		protected override bool IsOffscreenCore ()
		{
			return false;
		}
		
		protected override bool IsPasswordCore ()
		{
			return false;
		}
		
		protected override bool IsRequiredForFormCore ()
		{
			bool? isRequired = (bool?) owner.GetValue (AutomationProperties.IsRequiredForFormProperty);
			return isRequired.HasValue ? isRequired.Value : false;
		}
		
		protected override void SetFocusCore ()
		{
			Control ownerAsControl = owner as Control;
			if (ownerAsControl != null)
				ownerAsControl.Focus ();
		}
		
		public static AutomationPeer CreatePeerForElement (UIElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			if (element.AutomationPeer == null) {
				element.AutomationPeer = element.CreateAutomationPeer ();
				// We need to cache old values to raise PropertyChanged events
				// when calling AutomationPeer.InvalidatePeer()
				if (element.AutomationPeer != null)
					element.AutomationPeer.CacheMainProperties ();
			}
			return element.AutomationPeer;
		}

		internal override AutomationPeer GetParentCore ()
		{
			return GetParentPeer (Owner as FrameworkElement);
		}

		private AutomationPeer GetParentPeer (FrameworkElement element)
		{
			// We are returning parents of children already instantiated.
			if (element == null)
				return null;

			FrameworkElement parent = element.Parent as FrameworkElement;
			if (parent == null)
				return null;

			// Some parents don't return an Automation Peer (for example: Border or Panel subclasses)
			AutomationPeer peer = FrameworkElementAutomationPeer.FromElement (parent);
			if (peer == null)
				peer = GetParentPeer (parent);
			
			return peer;
		}
	}
}
