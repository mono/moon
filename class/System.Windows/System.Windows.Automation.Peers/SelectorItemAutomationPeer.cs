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
			ListBoxItem listboxitem = uielement as ListBoxItem;
			if (listboxitem != null) {
				// SelectionItem Pattern Automation Events
				// - SelectionContainerProperty
				listboxitem.ParentSelectorChanged += (o, e) => {
					AutomationPeer oldSelectorPeer = null;
					AutomationPeer newSelectorPeer = null;

					if (selector != null)
						oldSelectorPeer 
							= FrameworkElementAutomationPeer.CreatePeerForElement (selector);
					if (listboxitem.ParentSelector != null)
						newSelectorPeer 
							= FrameworkElementAutomationPeer.CreatePeerForElement (listboxitem.ParentSelector);

					RaisePropertyChangedEvent (SelectionItemPatternIdentifiers.SelectionContainerProperty,
					                           ProviderFromPeer (oldSelectorPeer),
								   ProviderFromPeer (newSelectorPeer));
					selector = listboxitem.ParentSelector;
				};
				selector = listboxitem.ParentSelector;
			}
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
				throw new ElementNotEnabledException ();

			ISelectionProvider selectionProvider = SelectionProvider;
			if (selectionProvider == null)
				return;

			ISelectionItemProvider selectionItemProvider = (ISelectionItemProvider) this;
			if (!selectionProvider.CanSelectMultiple) {
				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				if (selection == null || selection.Length == 0)
					selectionItemProvider.Select ();
				else
					throw new InvalidOperationException ();
			} else
				selectionItemProvider.Select ();
		}

		void ISelectionItemProvider.RemoveFromSelection ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException ();
			if (!((ISelectionItemProvider) this).IsSelected)
				return;

			if (selector != null) {
				SetFocusItemsControl ();
				selector.SelectedIndex = -1;
			}
		}

		void ISelectionItemProvider.Select ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException ();

			if (selector != null) {
				object item = Item;
				if (item != null) {
					DependencyObject container
						= selector.ItemContainerGenerator.ContainerFromItem (item);
					if (container != null) {
						SetFocusItem ();
						selector.SelectedIndex
							= selector.ItemContainerGenerator.IndexFromContainer (container);
					}
				}
			}
		}

		bool ISelectionItemProvider.IsSelected {
			get {
				ListBoxItem item = Owner as ListBoxItem;
				return selector != null 
				       && item != null
				       && item.IsSelected;
			}
		}

		IRawElementProviderSimple ISelectionItemProvider.SelectionContainer {
			get { return ProviderFromPeer (ItemsControlAutomationPeer); }
		}

		#endregion

		#region Private fields

		private ISelectionProvider SelectionProvider {
			get {
				if (selector == null)
					return null;
				AutomationPeer peer 
					= FrameworkElementAutomationPeer.CreatePeerForElement (selector);
				if (peer == null)
					return null;
				return peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
			}
		}

		#endregion

		private Selector selector;
	}
}
