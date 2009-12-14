// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Diagnostics;

namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for TabItem
    /// </summary>
    public sealed class TabItemAutomationPeer : ItemAutomationPeer, ISelectionItemProvider
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the TabItemAutomationPeer class.
        /// </summary>
        /// <param name="owner">Owning TabItem for this AutomationPeer</param>
        public TabItemAutomationPeer(object owner) : base(owner as UIElement)
        {
        }

        #endregion

        #region AutomationPeer Overrides

        /// <summary>
        /// Gets the control type for the element that is associated with the UI Automation peer.
        /// </summary>
        /// <returns>The control type.</returns>
        protected override AutomationControlType GetAutomationControlTypeCore()
        {
            return AutomationControlType.TabItem;
        }

        /// <summary>
        /// Returns the ChildrenCore
        /// </summary>
        /// <returns>List of AutomationPeers for the Children</returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification="Requires a return list of AutomationPeers")]
        protected override List<AutomationPeer> GetChildrenCore()
        {
            List<AutomationPeer> childrenCore = base.GetChildrenCore();
            TabItem wrapper = Item as TabItem;
            if ((wrapper != null) && wrapper.IsSelected)
            {
                TabControl control = TabOwner;
                if (control == null)
                {
                    return childrenCore;
                }
                ContentPresenter selectedContentPresenter = control.GetContentHost(control.TabStripPlacement);
                if (selectedContentPresenter == null)
                {
                    return childrenCore;
                }
                List<AutomationPeer> children = new FrameworkElementAutomationPeer(selectedContentPresenter).GetChildren();
                if (children == null)
                {
                    return childrenCore;
                }
                if (childrenCore == null)
                {
                    return children;
                }
                childrenCore.AddRange(children);
            }
            return childrenCore;
        }

        /// <summary>
        /// Called by GetClassName that gets a human readable name that, in addition to AutomationControlType, 
        /// differentiates the control represented by this AutomationPeer.
        /// </summary>
        /// <returns>The string that contains the name.</returns>
        protected override string GetClassNameCore()
        {
            return Owner.GetType().Name;
        }

        /// <summary>
        /// Gets the control pattern that is associated with the specified System.Windows.Automation.Peers.PatternInterface.
        /// </summary>
        /// <param name="patternInterface">A value from the System.Windows.Automation.Peers.PatternInterface enumeration.</param>
        /// <returns>The object that supports the specified pattern, or null if unsupported.</returns>
        public override object GetPattern(PatternInterface patternInterface)
        {
            if (patternInterface == PatternInterface.SelectionItem)
            {
                return this;
            }
            return base.GetPattern(patternInterface);
        }

        /// <summary>
        /// Gets the text label of the TabItem that is associated with 
        /// this TabItemAutomationPeer. Called by GetName. 
        /// </summary>
        /// <returns>
        /// The string that contains the label. If set, 
        /// this method returns the value of the Name property; 
        /// otherwise this method will return the value of the Header property. 
        /// </returns>
        protected override string GetNameCore()
        {
            TabItem wrapper = this.Owner as TabItem;
            Debug.Assert(wrapper != null);
            return (wrapper.Header as string) ?? String.Empty;
        }

        #endregion

        #region ISelectionItemProvider Members

        /// <summary>
        /// Select the item
        /// </summary>
        void ISelectionItemProvider.AddToSelection()
        {
            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }

            TabControl control = TabOwner;
            if (control == null)
            {
                // 

                return;
            }
            control.SelectedItem = Item;
        }

        /// <summary>
        /// Whether the item is selected
        /// </summary>
        bool ISelectionItemProvider.IsSelected
        {
            get
            {
                TabControl control = TabOwner;
                return control != null && Item.Equals(control.SelectedItem);
            }
        }

        /// <summary>
        /// If the item is selected, make sure it is no longer selected
        /// </summary>
        void ISelectionItemProvider.RemoveFromSelection()
        {
            // Even though we can set the SelectedItem to null
            // programmatically through code, we cannot do this
            // through normal interaction with the TabControl, and
            // so we will not allow Automation to RemoveFromSelection
            // either.

            // 

            return;
        }

        /// <summary>
        /// Select the item
        /// </summary>
        void ISelectionItemProvider.Select()
        {
            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }

            TabControl control = TabOwner;
            if (control == null)
            {
                // 

                return;
            }
            control.SelectedItem = Item;
        }

        /// <summary>
        /// Return the SelectionContainer
        /// </summary>
        IRawElementProviderSimple ISelectionItemProvider.SelectionContainer
        {
            get { return base.ProviderFromPeer(ItemsControlAutomationPeer); }
        }

        #endregion

        #region Private Helpers

        private TabControl TabOwner
        {
            get
            {
                if (ItemsControlAutomationPeer != null)
                {
                    return ItemsControlAutomationPeer.Owner as TabControl;
                }
                else
                {
                    return null;
                }
            }
        }

        #endregion

        #region Events

        /// <summary>
        /// Raise the event for when the IsSelectedProperty changes
        /// </summary>
        /// <param name="isSelected"></param>
        internal void RaiseAutomationIsSelectedChanged(bool isSelected)
        {
            RaisePropertyChangedEvent(
                SelectionItemPatternIdentifiers.IsSelectedProperty,
                !isSelected,
                isSelected);
        }

        #endregion Events
    }
}
