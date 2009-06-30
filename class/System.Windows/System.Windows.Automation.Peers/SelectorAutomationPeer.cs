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
// Copyright (c) 2009 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System.Windows;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using System.Collections.Generic;

namespace System.Windows.Automation.Peers {

	public abstract class SelectorAutomationPeer : ItemsControlAutomationPeer, ISelectionProvider {

		protected SelectorAutomationPeer (Selector owner) : base (owner)
		{
			// Since Selector only supports one item selected we are only raising SelectionItemPatternOnElementSelected
			owner.SelectionChanged += (o, e) => {
				if (owner.SelectedIndex == -1) 
					return;
				
				UIElement selectedItem = GetChildAtIndex (owner.SelectedIndex);
				if (selectedItem != null) {
					AutomationPeer peer 
						= FrameworkElementAutomationPeer.CreatePeerForElement (selectedItem);
					if (peer != null)
						peer.RaiseAutomationEvent (AutomationEvents.SelectionItemPatternOnElementSelected);
				}
					
			};
		}

		public override object GetPattern (PatternInterface pattern)
		{
			if (pattern == PatternInterface.Selection)
				return this;

			return base.GetPattern (pattern);
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return base.GetAutomationControlTypeCore ();
		}

		#region ISelectionProvider realization

		IRawElementProviderSimple[] ISelectionProvider.GetSelection ()
		{
			Selector selector = Owner as Selector;

			if (selector.SelectedIndex == -1)
				return null;

			UIElement uielement = GetChildAtIndex (selector.SelectedIndex);
			if (uielement == null)
				return null;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (uielement);
			if (peer != null)
				return new IRawElementProviderSimple [] { ProviderFromPeer (peer) };
			
			return null;
		}

		bool ISelectionProvider.CanSelectMultiple {
			get { return false; }
		}

		bool ISelectionProvider.IsSelectionRequired {
			get { return false; }
		}

		#endregion

	}
}
