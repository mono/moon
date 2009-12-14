// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for TabControl
    /// </summary>
    public sealed class TabControlAutomationPeer : ItemsControlAutomationPeer, ISelectionProvider
    {
        #region Constructors

        /// <summary>
        /// AutomationPeer for TabControl
        /// </summary>
        /// <param name="owner">TabControl</param>
        public TabControlAutomationPeer(TabControl owner)
            : base(owner)
        {
        }

        #endregion

        #region AutomationPeer Overrides

        /// <summary>
        /// Creates a new System.Windows.Automation.Peers.TabItemAutomationPeer.
        /// </summary>
        /// <param name="item">The System.Windows.Controls.TabItem that is associated with the new System.Windows.Automation.Peers.TabItemAutomationPeer</param>
        /// <returns>The TabItemAutomationPeer that is created</returns>
        private static ItemAutomationPeer CreateItemAutomationPeer(object item)
        {
            return new TabItemAutomationPeer(item);
        }

        /// <summary>
        /// Gets the control type for the element that is associated with the UI Automation peer.
        /// </summary>
        /// <returns>The control type.</returns>
        protected override AutomationControlType GetAutomationControlTypeCore()
        {
            return AutomationControlType.Tab;
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
        /// This method is called by System.Windows.Automation.Peers.AutomationPeer.GetClickablePoint()
        /// </summary>
        /// <returns>
        /// A System.Windows.Point containing System.Double.NaN, System.Double.NaN; the
        /// only clickable points in a System.Windows.Controls.TabControl are the child
        /// System.Windows.Controls.TabItem elements.
        /// </returns>
        protected override Point GetClickablePointCore()
        {
            return new Point(double.NaN, double.NaN);
        }

        /// <summary>
        /// Gets the control pattern that is associated with the specified System.Windows.Automation.Peers.PatternInterface.
        /// </summary>
        /// <param name="patternInterface">A value from the System.Windows.Automation.Peers.PatternInterface enumeration.</param>
        /// <returns>The object that supports the specified pattern, or null if unsupported.</returns>
        public override object GetPattern(PatternInterface patternInterface)
        {
            if (patternInterface == PatternInterface.Selection)
            {
                return this;
            }
            return base.GetPattern(patternInterface);
        }

        #endregion

        #region ISelectionProvider Members

        /// <summary>
        /// Whether or not this control can select multiple items
        /// </summary>
        bool ISelectionProvider.CanSelectMultiple
        {
            get { return false; }
        }

        /// <summary>
        /// Gets the AutomationPeer for the SelectedItem
        /// </summary>
        /// <returns></returns>
        IRawElementProviderSimple[] ISelectionProvider.GetSelection()
        {
            TabControl tabControl = base.Owner as TabControl;
            if (tabControl.SelectedItem == null)
            {
                return null;
            }
            List<IRawElementProviderSimple> list = new List<IRawElementProviderSimple>(1);

            ItemAutomationPeer peer = CreateItemAutomationPeer(tabControl.SelectedItem);

            if (peer != null)
            {
                list.Add(base.ProviderFromPeer(peer));
            }
            return list.ToArray();

        }

        /// <summary>
        /// Whether or not selection is required for this control
        /// </summary>
        bool ISelectionProvider.IsSelectionRequired
        {
            get { return true; }
        }

        #endregion
    }
}
