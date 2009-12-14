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
    /// AutomationPeer for DataGridCell
    /// </summary>
    sealed public class DataGridCellAutomationPeer : FrameworkElementAutomationPeer,
        IGridItemProvider, IInvokeProvider, IScrollItemProvider, ISelectionItemProvider, ITableItemProvider
    {
        #region Constructors

        /// <summary>
        /// AutomationPeer for DataGridCell
        /// </summary>
        /// <param name="owner">DataGridCell</param>
        public DataGridCellAutomationPeer(DataGridCell owner)
            : base(owner)
        {
        }

        #endregion
        
        #region Properties

        private DataGridCell OwningCell
        {
            get
            {
                return (DataGridCell)Owner;
            }
        }

        private IRawElementProviderSimple ContainingGrid
        {
            get
            {
                AutomationPeer peer = CreatePeerForElement(this.OwningCell.OwningGrid);
                if (peer != null)
                {
                    return ProviderFromPeer(peer);
                }
                return null;
            }
        }

        #endregion

        #region AutomationPeer Overrides

        /// <summary>
        /// Gets the control type for the element that is associated with the UI Automation peer.
        /// </summary>
        /// <returns>The control type.</returns>
        protected override AutomationControlType GetAutomationControlTypeCore()
        {
            return AutomationControlType.Custom;
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
        /// Gets the name of the element.
        /// </summary>
        /// <returns>The string that contains the name.</returns>
        protected override string GetNameCore()
        {
            TextBlock textBlock = this.OwningCell.Content as TextBlock;
            if (textBlock != null)
            {
                return textBlock.Text;
            }
            TextBox textBox = this.OwningCell.Content as TextBox;
            if (textBox != null)
            {
                return textBox.Text;
            }
            return base.GetNameCore();
        }

        /// <summary>
        /// Gets the control pattern that is associated with the specified System.Windows.Automation.Peers.PatternInterface.
        /// </summary>
        /// <param name="patternInterface">A value from the System.Windows.Automation.Peers.PatternInterface enumeration.</param>
        /// <returns>The object that supports the specified pattern, or null if unsupported.</returns>
        public override object GetPattern(PatternInterface patternInterface)
        {
            switch (patternInterface)
            {
                case PatternInterface.Invoke:
                    {
                        if (!this.OwningCell.OwningGrid.IsReadOnly &&
                            this.OwningCell.OwningColumn != null &&
                            !this.OwningCell.OwningColumn.IsReadOnly)
                        {
                            return this;
                        }
                        break;
                    }
                case PatternInterface.ScrollItem:
                    {
                        if (this.OwningCell.OwningGrid.HorizontalScrollBar != null &&
                            this.OwningCell.OwningGrid.HorizontalScrollBar.Maximum > 0)
                        {
                            return this;
                        }
                        break;
                    }
                case PatternInterface.GridItem:
                case PatternInterface.SelectionItem:
                case PatternInterface.TableItem:
                    return this;

            }
            return base.GetPattern(patternInterface);
        }

        /// <summary>
        /// Gets a value that indicates whether the element can accept keyboard focus.
        /// </summary>
        /// <returns>true if the element can accept keyboard focus; otherwise, false</returns>
        protected override bool IsKeyboardFocusableCore()
        {
            return true;
        }

        #endregion

        #region IGridItemProvider

        int IGridItemProvider.Column
        {
            get
            {
                return this.OwningCell.ColumnIndex;
            }
        }

        int IGridItemProvider.ColumnSpan
        {
            get
            {
                return 1;
            }
        }

        IRawElementProviderSimple IGridItemProvider.ContainingGrid
        {
            get
            {
                return this.ContainingGrid;
            }
        }

        int IGridItemProvider.Row
        {
            get
            {
                return this.OwningCell.RowIndex;
            }
        }

        int IGridItemProvider.RowSpan
        {
            get
            {
                return 1;
            }
        }

        #endregion

        #region IInvokeProvider

        void IInvokeProvider.Invoke()
        {
            EnsureEnabled();

            bool success = false;
            if (this.OwningCell.OwningGrid != null)
            {
                if (success = this.OwningCell.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/) &&
                    this.OwningCell.OwningGrid.EditingRowIndex != this.OwningCell.RowIndex &&
                    this.OwningCell.OwningGrid.EditingColumnIndex != this.OwningCell.ColumnIndex)
                {
                    this.OwningCell.OwningGrid.ClearRowSelection(this.OwningCell.RowIndex, false /*setAnchorRowIndex*/);
                    this.OwningCell.OwningGrid.SetCurrentCellCore(this.OwningCell.ColumnIndex, this.OwningCell.RowIndex);
                    success = this.OwningCell.OwningGrid.BeginEdit();
                }
            }
            if (!success)
            {
                // 

                return;
            }
        }

        #endregion

        #region IScrollItemProvider

        void IScrollItemProvider.ScrollIntoView()
        {
            if (this.OwningCell.OwningGrid != null)
            {
                this.OwningCell.OwningGrid.ScrollIntoView(this.OwningCell.DataContext, this.OwningCell.OwningColumn);
            }
            else
            {
                // 

                return;
            }
        }

        #endregion

        #region ISelectionItemProvider

        bool ISelectionItemProvider.IsSelected
        {
            get
            {
                if (this.OwningCell.OwningGrid != null)
                {
                    return (this.OwningCell.OwningGrid.CurrentRowIndex == this.OwningCell.RowIndex &&
                        this.OwningCell.OwningGrid.CurrentColumnIndex == this.OwningCell.ColumnIndex);
                }
                // 

                return false;
            }
        }

        IRawElementProviderSimple ISelectionItemProvider.SelectionContainer
        {
            get
            {
                AutomationPeer peer = CreatePeerForElement(this.OwningCell.OwningRow);
                if (peer != null)
                {
                    return ProviderFromPeer(peer);
                }
                return null;
            }
        }

        void ISelectionItemProvider.AddToSelection()
        {
            EnsureEnabled();

            // 







        }

        void ISelectionItemProvider.RemoveFromSelection()
        {
            EnsureEnabled();

            // 







        }

        void ISelectionItemProvider.Select()
        {
            EnsureEnabled();

            bool success = false;
            if (this.OwningCell.OwningGrid != null)
            {
                if (this.OwningCell.OwningGrid.CurrentColumnIndex == this.OwningCell.ColumnIndex &&
                    this.OwningCell.OwningGrid.CurrentRowIndex == this.OwningCell.RowIndex)
                {
                    success = true;
                }
                else if (this.OwningCell.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                {
                    this.OwningCell.OwningGrid.ClearRowSelection(this.OwningCell.RowIndex, false /*setAnchorRowIndex*/);
                    success = this.OwningCell.OwningGrid.SetCurrentCellCore(this.OwningCell.ColumnIndex, this.OwningCell.RowIndex);
                }
            }
            if (!success)
            {
                // 

                return;
            }
        }

        #endregion

        #region ITableItemProvider

        IRawElementProviderSimple[] ITableItemProvider.GetColumnHeaderItems()
        {
            if (this.OwningCell.OwningGrid != null &&
                this.OwningCell.OwningGrid.AreColumnHeadersVisible &&
                this.OwningCell.OwningColumn.HeaderCell != null)
            {
                AutomationPeer peer = CreatePeerForElement(this.OwningCell.OwningColumn.HeaderCell);
                if (peer != null)
                {
                    List<IRawElementProviderSimple> providers = new List<IRawElementProviderSimple>(1);
                    providers.Add(ProviderFromPeer(peer));
                    return providers.ToArray();
                }
            }
            return null;
        }

        IRawElementProviderSimple[] ITableItemProvider.GetRowHeaderItems()
        {
            if (this.OwningCell.OwningGrid != null && 
                this.OwningCell.OwningGrid.AreRowHeadersVisible &&
                this.OwningCell.OwningRow.HeaderCell != null)
            {
                AutomationPeer peer = CreatePeerForElement(this.OwningCell.OwningRow.HeaderCell);
                if (peer != null)
                {
                    List<IRawElementProviderSimple> providers = new List<IRawElementProviderSimple>(1);
                    providers.Add(ProviderFromPeer(peer));
                    return providers.ToArray();
                }
            }
            return null;
        }

        #endregion

        #region Private Methods

        private void EnsureEnabled()
        {
            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }
        }

        #endregion
    }
}
