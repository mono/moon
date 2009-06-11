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
				return new IRawElementProviderSimple [0];
			else if (VisualTreeHelper.GetChildrenCount (selector) >= selector.SelectedIndex)
				return new IRawElementProviderSimple [0];

			UIElement uielement
				= VisualTreeHelper.GetChild (selector, selector.SelectedIndex) as UIElement;
			if (uielement == null)
				return new IRawElementProviderSimple [0];

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (uielement);
			if (peer != null)
				return new IRawElementProviderSimple [] { ProviderFromPeer (peer) };
			else
				return new IRawElementProviderSimple [0];
		}


		[MonoTODO("Confirm returned value with tests")]
		bool ISelectionProvider.CanSelectMultiple {
			get { return false; }
		}

		[MonoTODO("Confirm returned value with tests")]
		bool ISelectionProvider.IsSelectionRequired {
			get { return false; }
		}

		#endregion

		internal override List<AutomationPeer> GetChildrenInternal ()
		{
			Selector selector = Owner as Selector;
			if (selector.Items.Count > 0)
				return FrameworkElementAutomationPeer.GetChildrenRecursively (selector);
			
			return null;
		}

	}
}
