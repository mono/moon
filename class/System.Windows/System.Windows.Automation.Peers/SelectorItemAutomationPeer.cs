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

using System;
using System.Windows;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {
	public abstract class SelectorItemAutomationPeer : ItemAutomationPeer, ISelectionItemProvider {

		protected SelectorItemAutomationPeer (UIElement uielement) : base (uielement)
		{
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			if (patternInterface == PatternInterface.SelectionItem)
				return this;

			return base.GetPattern (patternInterface);
		}

		#region ISelectionItemProvider realization

		void ISelectionItemProvider.AddToSelection ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException();

			Selector control = SelectorOwner;
			if (control != null)
				control.SelectedItem = Item;
		}

		void ISelectionItemProvider.RemoveFromSelection ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException ();
			if (!((ISelectionItemProvider) this).IsSelected)
				return;

			Selector control = SelectorOwner;
			if (control != null)
				control.SelectedIndex = -1;
		}

		void ISelectionItemProvider.Select ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException ();

			Selector control = SelectorOwner;
			if (control != null)
				control.SelectedItem = Item;
		}

		bool ISelectionItemProvider.IsSelected {
			get {
				Selector control = SelectorOwner;
				return control != null && Item.Equals (control.SelectedItem);
			}
		}

		IRawElementProviderSimple ISelectionItemProvider.SelectionContainer {
			get { return base.ProviderFromPeer (ItemsControlAutomationPeer); }
		}

		#endregion

		#region Private fields

		private Selector SelectorOwner {
			get {
				if (ItemsControlAutomationPeer != null)
					return ItemsControlAutomationPeer.Owner as Selector;
				else
					return null;
			}
		}

		#endregion
	}
}
