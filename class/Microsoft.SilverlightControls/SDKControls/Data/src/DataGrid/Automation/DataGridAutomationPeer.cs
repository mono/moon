// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for DataGrid
    /// </summary>
    sealed public class DataGridAutomationPeer : FrameworkElementAutomationPeer,
        IGridProvider, IScrollProvider, ISelectionProvider, ITableProvider
    {
        #region Data

        private Dictionary<object, DataGridItemAutomationPeer> _itemPeers = new Dictionary<object, DataGridItemAutomationPeer>();
        private bool _oldHorizontallyScrollable;
        private double _oldHorizontalScrollPercent;
        private double _oldHorizontalViewSize;
        private bool _oldVerticallyScrollable;
        private double _oldVerticalScrollPercent;
        private double _oldVerticalViewSize;

        #endregion

        #region Constructors

        /// <summary>
        /// AutomationPeer for DataGrid
        /// </summary>
        /// <param name="owner">DataGrid</param>
        public DataGridAutomationPeer(DataGrid owner)
            : base(owner)
        {
            if (this.HorizontallyScrollable)
            {
                _oldHorizontallyScrollable = true;
                _oldHorizontalScrollPercent = this.HorizontalScrollPercent;
                _oldHorizontalViewSize = this.HorizontalViewSize;
            }
            else
            {
                _oldHorizontallyScrollable = false;
                _oldHorizontalScrollPercent = ScrollPatternIdentifiers.NoScroll;
                _oldHorizontalViewSize = 100.0;
            }

            if (this.VerticallyScrollable)
            {
                _oldVerticallyScrollable = true;
                _oldVerticalScrollPercent = this.VerticalScrollPercent;
                _oldVerticalViewSize = this.VerticalViewSize;
            }
            else
            {
                _oldVerticallyScrollable = false;
                _oldVerticalScrollPercent = ScrollPatternIdentifiers.NoScroll;
                _oldVerticalViewSize = 100.0;
            }
        }

        #endregion

        #region Properties

        private bool HorizontallyScrollable
        {
            get
            {
                return (OwningDataGrid.HorizontalScrollBar != null && OwningDataGrid.HorizontalScrollBar.Maximum > 0);
            }
        }

        private double HorizontalScrollPercent
        {
            get
            {
                if (!this.HorizontallyScrollable)
                {
                    return ScrollPatternIdentifiers.NoScroll;
                }
                return (double)(this.OwningDataGrid.HorizontalScrollBar.Value * 100.0 / this.OwningDataGrid.HorizontalScrollBar.Maximum);
            }
        }

        private double HorizontalViewSize
        {
            get
            {
                if (!this.HorizontallyScrollable || DoubleUtil.IsZero(this.OwningDataGrid.HorizontalScrollBar.Maximum))
                {
                    return 100.0;
                }
                return (double)(this.OwningDataGrid.HorizontalScrollBar.ViewportSize * 100.0 /
                    (this.OwningDataGrid.HorizontalScrollBar.ViewportSize + this.OwningDataGrid.HorizontalScrollBar.Maximum));
            }
        }

        private DataGrid OwningDataGrid
        {
            get
            {
                return (DataGrid)Owner;
            }
        }

        private bool VerticallyScrollable
        {
            get
            {
                return (OwningDataGrid.VerticalScrollBar != null && OwningDataGrid.VerticalScrollBar.Maximum > 0);
            }
        }

        private double VerticalScrollPercent
        {
            get
            {
                if (!this.VerticallyScrollable)
                {
                    return ScrollPatternIdentifiers.NoScroll;
                }
                return (double)(this.OwningDataGrid.VerticalScrollBar.Value * 100.0 / this.OwningDataGrid.VerticalScrollBar.Maximum);
            }
        }

        private double VerticalViewSize
        {
            get
            {
                if (!this.VerticallyScrollable || DoubleUtil.IsZero(this.OwningDataGrid.VerticalScrollBar.Maximum))
                {
                    return 100.0;
                }
                return (double)(this.OwningDataGrid.VerticalScrollBar.ViewportSize * 100.0 /
                    (this.OwningDataGrid.VerticalScrollBar.ViewportSize + this.OwningDataGrid.VerticalScrollBar.Maximum));
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
            return AutomationControlType.DataGrid;
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
            switch (patternInterface)
            {
                case PatternInterface.Grid:
                case PatternInterface.Selection:
                case PatternInterface.Table:
                    return this;
                case PatternInterface.Scroll:
                    {
                        if (this.HorizontallyScrollable || this.VerticallyScrollable)
                        {
                            return this;
                        }
                        break;
                    }

            }
            return base.GetPattern(patternInterface);
        }

        #endregion

        #region IGridProvider

        int IGridProvider.ColumnCount
        {
            get
            {
                return this.OwningDataGrid.Columns.Count;
            }
        }

        int IGridProvider.RowCount
        {
            get
            {
                return this.OwningDataGrid.RowCount;
            }
        }

        IRawElementProviderSimple IGridProvider.GetItem(int row, int column)
        {
            if (this.OwningDataGrid != null &&
                this.OwningDataGrid.DataConnection != null &&
                row >= 0 && row < this.OwningDataGrid.RowCount &&
                column >= 0 && column < this.OwningDataGrid.Columns.Count)
            {
                object item = null;
                if (!this.OwningDataGrid.DisplayData.IsRowDisplayed(row))
                {
                    item = this.OwningDataGrid.DataConnection.GetDataItem(row);
                }
                this.OwningDataGrid.ScrollIntoView(item, this.OwningDataGrid.Columns[column]);

                DataGridRow dgr = this.OwningDataGrid.DisplayData.GetDisplayedRow(row);
                DataGridCell cell = dgr.Cells[column];
                AutomationPeer peer = CreatePeerForElement(cell);
                if (peer != null)
                {
                    return ProviderFromPeer(peer);
                }
            }
            return null;
        }

        #endregion

        #region IScrollProvider

        bool IScrollProvider.HorizontallyScrollable
        {
            get
            {
                return this.HorizontallyScrollable;
            }
        }

        double IScrollProvider.HorizontalScrollPercent
        {
            get
            {
                return this.HorizontalScrollPercent;
            }
        }

        double IScrollProvider.HorizontalViewSize
        {
            get
            {
                return this.HorizontalViewSize;
            }
        }

        bool IScrollProvider.VerticallyScrollable
        {
            get
            {
                return this.VerticallyScrollable;
            }
        }

        double IScrollProvider.VerticalScrollPercent
        {
            get
            {
                return this.VerticalScrollPercent;
            }
        }

        double IScrollProvider.VerticalViewSize
        {
            get
            {
                return this.VerticalViewSize;
            }
        }

        void IScrollProvider.Scroll(ScrollAmount horizontalAmount, ScrollAmount verticalAmount)
        {
            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }

            bool scrollHorizontally = (horizontalAmount != ScrollAmount.NoAmount);
            bool scrollVertically = (verticalAmount != ScrollAmount.NoAmount);

            if (scrollHorizontally && !this.HorizontallyScrollable || scrollVertically && !this.VerticallyScrollable)
            {
                // 

                return;
            }

            switch (horizontalAmount)
            {
                // In the small increment and decrement calls, ScrollHorizontally will adjust the
                // ScrollBar.Value itself, so we don't need to do it here
                case ScrollAmount.SmallIncrement:
                    this.OwningDataGrid.ProcessHorizontalScroll(ScrollEventType.SmallIncrement);
                    break;
                case ScrollAmount.LargeIncrement:
                    this.OwningDataGrid.HorizontalScrollBar.Value += this.OwningDataGrid.HorizontalScrollBar.LargeChange;
                    this.OwningDataGrid.ProcessHorizontalScroll(ScrollEventType.LargeIncrement);
                    break;
                case ScrollAmount.SmallDecrement:
                    this.OwningDataGrid.ProcessHorizontalScroll(ScrollEventType.SmallDecrement);
                    break;
                case ScrollAmount.LargeDecrement:
                    this.OwningDataGrid.HorizontalScrollBar.Value -= this.OwningDataGrid.HorizontalScrollBar.LargeChange;
                    this.OwningDataGrid.ProcessHorizontalScroll(ScrollEventType.LargeDecrement);
                    break;
                case ScrollAmount.NoAmount:
                    break;
                default:
                    // 

                    return;
            }

            switch (verticalAmount)
            {
                // In the small increment and decrement calls, ScrollVertically will adjust the
                // ScrollBar.Value itself, so we don't need to do it here
                case ScrollAmount.SmallIncrement:
                    this.OwningDataGrid.ProcessVerticalScroll(ScrollEventType.SmallIncrement);
                    break;
                case ScrollAmount.LargeIncrement:
                    this.OwningDataGrid.VerticalScrollBar.Value += this.OwningDataGrid.VerticalScrollBar.LargeChange;
                    this.OwningDataGrid.ProcessVerticalScroll(ScrollEventType.LargeIncrement);
                    break;
                case ScrollAmount.SmallDecrement:
                    this.OwningDataGrid.ProcessVerticalScroll(ScrollEventType.SmallDecrement);
                    break;
                case ScrollAmount.LargeDecrement:
                    this.OwningDataGrid.VerticalScrollBar.Value -= this.OwningDataGrid.VerticalScrollBar.LargeChange;
                    this.OwningDataGrid.ProcessVerticalScroll(ScrollEventType.LargeDecrement);
                    break;
                case ScrollAmount.NoAmount:
                    break;
                default:
                    // 

                    return;
            }
        }

        void IScrollProvider.SetScrollPercent(double horizontalPercent, double verticalPercent)
        {

            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }

            bool scrollHorizontally = (horizontalPercent != (double)ScrollPatternIdentifiers.NoScroll);
            bool scrollVertically = (verticalPercent != (double)ScrollPatternIdentifiers.NoScroll);

            if (scrollHorizontally && !this.HorizontallyScrollable || scrollVertically && !this.VerticallyScrollable)
            {
                // 

                return;
            }

            if (scrollHorizontally && (horizontalPercent < 0.0) || (horizontalPercent > 100.0))
            {
                // 

                return;
            }
            if (scrollVertically && (verticalPercent < 0.0) || (verticalPercent > 100.0))
            {
                // 

                return;
            }

            if (scrollHorizontally)
            {
                this.OwningDataGrid.HorizontalScrollBar.Value =
                    (double)(this.OwningDataGrid.HorizontalScrollBar.Maximum * (horizontalPercent / 100.0));
                this.OwningDataGrid.ProcessHorizontalScroll(ScrollEventType.ThumbPosition);
            }
            if (scrollVertically)
            {
                this.OwningDataGrid.VerticalScrollBar.Value =
                    (double)(this.OwningDataGrid.VerticalScrollBar.Maximum * (verticalPercent / 100.0));
                this.OwningDataGrid.ProcessVerticalScroll(ScrollEventType.ThumbPosition);
            }
        }

        #endregion

        #region ISelectionProvider

        IRawElementProviderSimple[] ISelectionProvider.GetSelection()
        {
            if (this.OwningDataGrid != null &&
                this.OwningDataGrid.SelectedItems != null)
            {
                List<IRawElementProviderSimple> selectedProviders = new List<IRawElementProviderSimple>();
                foreach (object item in this.OwningDataGrid.SelectedItems)
                {
                    DataGridItemAutomationPeer peer = GetOrCreateItemPeer(item);
                    selectedProviders.Add(ProviderFromPeer(peer));
                }
                return selectedProviders.ToArray();
            }
            return null;
        }

        bool ISelectionProvider.CanSelectMultiple
        {
            get
            {
                return (this.OwningDataGrid.SelectionMode == DataGridSelectionMode.Extended);
            }
        }

        bool ISelectionProvider.IsSelectionRequired
        {
            get
            {
                return false;
            }
        }

        #endregion

        #region ITableProvider

        RowOrColumnMajor ITableProvider.RowOrColumnMajor
        {
            get
            {
                return RowOrColumnMajor.RowMajor;
            }
        }

        IRawElementProviderSimple[] ITableProvider.GetColumnHeaders()
        {
            if (this.OwningDataGrid.AreColumnHeadersVisible)
            {
                List<IRawElementProviderSimple> providers = new List<IRawElementProviderSimple>();
                foreach (DataGridColumn column in this.OwningDataGrid.Columns)
                {
                    if (column.HeaderCell != null)
                    {
                        AutomationPeer peer = CreatePeerForElement(column.HeaderCell);
                        if (peer != null)
                        {
                            providers.Add(ProviderFromPeer(peer));
                        }
                    }
                }
                if (providers.Count > 0)
                {
                    return providers.ToArray();
                }
            }
            return null;
        }

        IRawElementProviderSimple[] ITableProvider.GetRowHeaders()
        {
            if (this.OwningDataGrid.AreRowHeadersVisible)
            {
                List<IRawElementProviderSimple> providers = new List<IRawElementProviderSimple>();
                foreach (DataGridRow row in this.OwningDataGrid.DisplayData.GetScrollingRows())
                {
                    if (row.HeaderCell != null)
                    {
                        AutomationPeer peer = CreatePeerForElement(row.HeaderCell);
                        if (peer != null)
                        {
                            providers.Add(ProviderFromPeer(peer));
                        }
                    }
                }
                if (providers.Count > 0)
                {
                    return providers.ToArray();
                }
            }
            return null;
        }

        #endregion

        #region Methods

        internal List<AutomationPeer> GetItemPeers()
        {
            List<AutomationPeer> peers = new List<AutomationPeer>();
            Dictionary<object, DataGridItemAutomationPeer> oldChildren = new Dictionary<object, DataGridItemAutomationPeer>(_itemPeers);
            _itemPeers.Clear();

            if (this.OwningDataGrid.ItemsSource != null)
            {
                foreach (object item in this.OwningDataGrid.ItemsSource)
                {
                    DataGridItemAutomationPeer peer = null;

                    if (oldChildren.ContainsKey(item))
                    {
                        peer = oldChildren[item];
                    }
                    else
                    {
                        peer = new DataGridItemAutomationPeer(item, this.OwningDataGrid);
                    }

                    if (peer != null)
                    {
                        DataGridRowAutomationPeer rowPeer = peer.OwningRowPeer;
                        if (rowPeer != null)
                        {
                            rowPeer.EventsSource = peer;
                        }
                    }

                    // This guards against the addition of duplicate items
                    if (!_itemPeers.ContainsKey(item))
                    {
                        peers.Add(peer);
                        _itemPeers.Add(item, peer);
                    }
                }
            }
            return peers;
        }

        private DataGridItemAutomationPeer GetOrCreateItemPeer(object item)
        {
            DataGridItemAutomationPeer peer = null;

            if (_itemPeers.ContainsKey(item))
            {
                peer = _itemPeers[item];
            }
            else
            {
                peer = new DataGridItemAutomationPeer(item, this.OwningDataGrid);
                _itemPeers.Add(item, peer);
            }

            DataGridRowAutomationPeer rowPeer = peer.OwningRowPeer;
            if (rowPeer != null)
            {
                rowPeer.EventsSource = peer;
            }

            return peer;
        }

        internal void RaiseAutomationCellSelectedEvent(int row, int column)
        {
            if (row >= 0 && row < this.OwningDataGrid.RowCount &&
                column >= 0 && column < this.OwningDataGrid.Columns.Count &&
                this.OwningDataGrid.DisplayData.IsRowDisplayed(row))
            {
                DataGridRow dgr = this.OwningDataGrid.DisplayData.GetDisplayedRow(row);
                DataGridCell cell = dgr.Cells[column];
                AutomationPeer peer = CreatePeerForElement(cell);
                if (peer != null)
                {
                    peer.RaiseAutomationEvent(AutomationEvents.SelectionItemPatternOnElementSelected);
                }
            }
        }

        internal void RaiseAutomationInvokeEvents(DataGridEditingUnit editingUnit, DataGridColumn column, DataGridRow row)
        {
            switch (editingUnit)
            {
                case DataGridEditingUnit.Cell:
                    {
                        DataGridCell cell = row.Cells[column.Index];
                        AutomationPeer peer = FromElement(cell);
                        if (peer != null)
                        {
                            peer.InvalidatePeer();
                        }
                        else
                        {
                            CreatePeerForElement(cell);
                        }

                        if (peer != null)
                        {
                            peer.RaiseAutomationEvent(AutomationEvents.InvokePatternOnInvoked);
                        }
                        break;
                    }
                case DataGridEditingUnit.Row:
                    {
                        DataGridItemAutomationPeer peer = GetOrCreateItemPeer(row.DataContext);
                        peer.RaiseAutomationEvent(AutomationEvents.InvokePatternOnInvoked);
                        break;
                    }
            }
        }

        internal void RaiseAutomationScrollEvents()
        {
            IScrollProvider isp = (IScrollProvider)this;

            bool newHorizontallyScrollable = isp.HorizontallyScrollable;
            double newHorizontalViewSize = isp.HorizontalViewSize;
            double newHorizontalScrollPercent = isp.HorizontalScrollPercent;

            bool newVerticallyScrollable = isp.VerticallyScrollable;
            double newVerticalViewSize = isp.VerticalViewSize;
            double newVerticalScrollPercent = isp.VerticalScrollPercent;

            if (_oldHorizontallyScrollable != newHorizontallyScrollable)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.HorizontallyScrollableProperty,
                    _oldHorizontallyScrollable,
                    newHorizontallyScrollable);
                _oldHorizontallyScrollable = newHorizontallyScrollable;
            }

            if (_oldHorizontalViewSize != newHorizontalViewSize)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.HorizontalViewSizeProperty,
                    _oldHorizontalViewSize,
                    newHorizontalViewSize);
                _oldHorizontalViewSize = newHorizontalViewSize;
            }

            if (_oldHorizontalScrollPercent != newHorizontalScrollPercent)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.HorizontalScrollPercentProperty,
                    _oldHorizontalScrollPercent,
                    newHorizontalScrollPercent);
                _oldHorizontalScrollPercent = newHorizontalScrollPercent;
            }

            if (_oldVerticallyScrollable != newVerticallyScrollable)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.VerticallyScrollableProperty,
                    _oldVerticallyScrollable,
                    newVerticallyScrollable);
                _oldVerticallyScrollable = newVerticallyScrollable;
            }

            if (_oldVerticalViewSize != newVerticalViewSize)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.VerticalViewSizeProperty,
                    _oldVerticalViewSize,
                    newVerticalViewSize);
                _oldVerticalViewSize = newVerticalViewSize;
            }

            if (_oldVerticalScrollPercent != newVerticalScrollPercent)
            {
                RaisePropertyChangedEvent(
                    ScrollPatternIdentifiers.VerticalScrollPercentProperty,
                    _oldVerticalScrollPercent,
                    newVerticalScrollPercent);
                _oldVerticalScrollPercent = newVerticalScrollPercent;
            }
        }

        internal void RaiseAutomationSelectionEvents(SelectionChangedEventArgs e)
        {
            // If the result of an AddToSelection or RemoveFromSelection is a single selected item,
            // then all we raise is the ElementSelectedEvent for single item
            if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementSelected) &&
                this.OwningDataGrid.SelectedItems.Count == 1)
            {
                if (_itemPeers.ContainsKey(this.OwningDataGrid.SelectedItem))
                {
                    DataGridItemAutomationPeer peer = _itemPeers[this.OwningDataGrid.SelectedItem];
                    peer.RaiseAutomationEvent(AutomationEvents.SelectionItemPatternOnElementSelected);
                }
            }
            else
            {
                int i;

                if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementAddedToSelection))
                {
                    for (i = 0; i < e.AddedItems.Count; i++)
                    {
                        if (_itemPeers.ContainsKey(e.AddedItems[i]))
                        {
                            DataGridItemAutomationPeer peer = _itemPeers[e.AddedItems[i]];
                            peer.RaiseAutomationEvent(AutomationEvents.SelectionItemPatternOnElementAddedToSelection);
                        }
                    }
                }

                if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection))
                {
                    for (i = 0; i < e.RemovedItems.Count; i++)
                    {
                        if (_itemPeers.ContainsKey(e.RemovedItems[i]))
                        {
                            DataGridItemAutomationPeer peer = _itemPeers[e.RemovedItems[i]];
                            peer.RaiseAutomationEvent(AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection);
                        }
                    }
                }
            }
        }

        internal void UpdateRowPeerEventsSource(DataGridRow row)
        {
            DataGridRowAutomationPeer peer = FromElement(row) as DataGridRowAutomationPeer;
            if (peer != null && _itemPeers.ContainsKey(row.DataContext))
            {
                peer.EventsSource = _itemPeers[row.DataContext];
            }
        }

        #endregion
    }
}
