//
// System.Windows.Automation.Peers.PopupAutomationPeer
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
using System.Collections.Generic;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {

	internal sealed class PopupAutomationPeer : FrameworkElementAutomationPeer {

		public PopupAutomationPeer (Popup owner)
			: base (owner)
		{
			owner.Opened += (o, e) => RaiseVisibilityEvents (false);
			owner.Closed += (o, e) => RaiseVisibilityEvents (true);
		}

		protected override string GetClassNameCore ()
		{
			return "Popup";
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Window;
		}

		protected override bool IsOffscreenCore ()
		{
			return base.IsOffscreenCore () || !((Popup) Owner).IsOpen;
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			Popup popup = (Popup) Owner;
			if (popup.Child == null)
				return null;
			return FrameworkElementAutomationPeer.GetChildrenRecursively (popup.Child);
		}

		private void RaiseVisibilityEvents (bool isOffscreen)
		{
			IAutomationCacheProperty cachedProperty
				= GetCachedProperty (AutomationElementIdentifiers.BoundingRectangleProperty);
			Rect newValue = GetBoundingRectangle ();
			RaisePropertyChangedEvent (AutomationElementIdentifiers.BoundingRectangleProperty,
			                           cachedProperty.OldValue,
			                           newValue);

			RaisePropertyChangedEvent (AutomationElementIdentifiers.IsOffscreenProperty,
			                           !isOffscreen,
			                           isOffscreen);
		}
	}
}

