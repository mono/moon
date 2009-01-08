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
using System.Windows;
using System.Security;
using System.Collections.Generic;

namespace System.Windows.Automation.Peers {

	[MonoTODO]
	public class FrameworkElementAutomationPeer : AutomationPeer {

		private UIElement owner;

		[SecuritySafeCritical]
		public FrameworkElementAutomationPeer (FrameworkElement owner)
		{
			this.owner = owner;
		}

		public UIElement Owner {
			get { return owner; }
		}

		protected override string GetNameCore ()
		{
			return String.Empty;
		}

		protected override string GetItemTypeCore ()
		{
			throw new NotImplementedException ();
		}

		protected override AutomationPeer GetLabeledByCore ()
		{
			return null;
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			throw new NotImplementedException ();
		}

		public override object GetPattern (PatternInterface pattern)
		{
			return null;
		}
		
		public static AutomationPeer FromElement (UIElement element)
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetAcceleratorKeyCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetAccessKeyCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetAutomationIdCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override Rect GetBoundingRectangleCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetClassNameCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override Point GetClickablePointCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetHelpTextCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetItemStatusCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetLocalizedControlTypeCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override AutomationOrientation GetOrientationCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override bool HasKeyboardFocusCore ()
		{
			throw new NotImplementedException ();
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
			throw new NotImplementedException ();
		}
		
		protected override bool IsKeyboardFocusableCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override bool IsOffscreenCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override bool IsPasswordCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override bool IsRequiredForFormCore ()
		{
			throw new NotImplementedException ();
		}
		
		protected override void SetFocusCore ()
		{
			throw new NotImplementedException ();
		}
		
		public static AutomationPeer CreatePeerForElement (UIElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			return element.GetAutomationPeer ();
		}
	}
}
