// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Linq;
using System.Windows.Automation.Peers;
using System.Windows.Media;

namespace System.Windows.Controls
{
    public partial class DataGrid
    {
        #region Internal Properties

        internal bool AreRowBottomGridLinesRequired
        {
            get
            {
                return (this.GridLinesVisibility == DataGridGridLinesVisibility.Horizontal || this.GridLinesVisibility == DataGridGridLinesVisibility.All) && this.HorizontalGridLinesBrush != null;
            }
        }

        internal int FirstRowIndex
        {
            get
            {
                return (this.RowCount > 0) ? 0 : -1;
            }
        }

        internal int LastRowIndex
        {
            get
            {
                return (this.RowCount > 0) ? this.RowCount - 1 : -1;
            }
        }

        #endregion Internal Properties

        #region Private Properties

        // Cumulated height of all known rows, including the gridlines and details section.
        // This property returns an approximation of the actual total row heights and also
        // updates the RowHeightEstimate
        private double EdgedRowsHeightCalculated
        {
            get
            {
                // If we're not displaying any rows or if we have infinite space the, relative height of our rows is 0
                if (this.DisplayData.LastDisplayedScrollingRow == -1 || double.IsPositiveInfinity(this.AvailableRowRoom))
                {
                    return 0;
                }
                Debug.Assert(this.DisplayData.LastDisplayedScrollingRow >= 0);
                Debug.Assert(_verticalOffset >= 0);
                Debug.Assert(this.NegVerticalOffset >= 0);

                // Height of all rows above the viewport
                double totalRowsHeight = _verticalOffset - this.NegVerticalOffset;

                // Add the height of all the rows currently displayed, AvailableRowRoom
                // is not always up to date enough for this
                foreach (DataGridRow row in this.DisplayData.GetScrollingRows())
                {
                    totalRowsHeight += row.TargetHeight;
                }

                // Details up to and including viewport
                int detailsCount = GetDetailsCountExclusive(-1, this.DisplayData.LastDisplayedScrollingRow + 1);

                // Subtract details that were accounted for from the totalRowsHeight
                totalRowsHeight -= detailsCount * this.RowDetailsHeightEstimate;

                // Update the RowHeightEstimate if we have more row information
                if (this.DisplayData.LastDisplayedScrollingRow >= _lastEstimatedRow)
                {
                    _lastEstimatedRow = this.DisplayData.LastDisplayedScrollingRow;
                    this.RowHeightEstimate = totalRowsHeight / (_lastEstimatedRow + 1);
                }

                // Calculate estimates for what's beyond the viewport
                if (this.RowCount > this.DisplayData.NumDisplayedScrollingRows)
                {
                    int remainingRowCount = (this.RowCount - this.DisplayData.LastDisplayedScrollingRow - 1);

                    // Add estimation for the cell heights of all rows beyond our viewport
                    totalRowsHeight += this.RowHeightEstimate * remainingRowCount;

                    // Add the rest of the details beyond the viewport
                    detailsCount += GetDetailsCountExclusive(this.DisplayData.LastDisplayedScrollingRow, this.RowCount);
                }

                // 
                double totalDetailsHeight = detailsCount * this.RowDetailsHeightEstimate;

                return totalRowsHeight + totalDetailsHeight;
            }
        }

        #endregion Private Properties

        #region Public Methods

        // 






















        #endregion Public Methods

        #region Protected Methods

        protected internal virtual void OnRowDetailsVisibilityChanged(DataGridRowDetailsEventArgs e)
        {
            EventHandler<DataGridRowDetailsEventArgs> handler = this.RowDetailsVisibilityChanged;

            if (handler != null)
            {
                handler(this, e);
            }
        }

        #endregion Protected Methods

        #region Internal Methods

        /// <summary>
        /// Clears the entire selection. Displayed rows are deselected explicitly to visualize
        /// potential transition effects
        /// </summary>
        internal void ClearRowSelection(bool resetAnchorRowIndex)
        {
            if (resetAnchorRowIndex)
            {
                this.AnchorRowIndex = -1;
            }
            if (_selectedItems.Count > 0)
            {
                this._noSelectionChangeCount++;
                try
                {
                    // Individually deselecting displayed rows to view potential transitions
                    for (int rowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                         rowIndex > -1 && rowIndex <= this.DisplayData.LastDisplayedScrollingRow;
                         rowIndex++)
                    {
                        if (_selectedItems.Contains(rowIndex))
                        {
                            SelectRow(rowIndex, false);
                        }
                    }
                    _selectedItems.ClearRows();
                    this._selectionChanged = true;
                }
                finally
                {
                    this.NoSelectionChangeCount--;
                }
            }
        }

        /// <summary>
        /// Clears the entire selection except the indicated row. Displayed rows are deselected explicitly to 
        /// visualize potential transition effects. The row indicated is selected if it is not already.
        /// </summary>
        internal void ClearRowSelection(int rowIndexException, bool setAnchorRowIndex)
        {
            this._noSelectionChangeCount++;
            try
            {
                bool exceptionAlreadySelected = false;
                if (_selectedItems.Count > 0)
                {
                    // Individually deselecting displayed rows to view potential transitions
                    for (int rowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                         rowIndex > -1 && rowIndex <= this.DisplayData.LastDisplayedScrollingRow;
                         rowIndex++)
                    {
                        if (rowIndexException != rowIndex && _selectedItems.Contains(rowIndex))
                        {
                            SelectRow(rowIndex, false);
                            this._selectionChanged = true;
                        }
                    }
                    exceptionAlreadySelected = _selectedItems.Contains(rowIndexException);
                    int selectedCount = _selectedItems.Count;
                    if (selectedCount > 0)
                    {
                        if (selectedCount > 1)
                        {
                            this._selectionChanged = true;
                        }
                        else
                        {
                            int currentlySelectedRowIndex = _selectedItems.GetIndexes().First();
                            if (currentlySelectedRowIndex != rowIndexException)
                            {
                                this._selectionChanged = true;
                            }
                        }
                        _selectedItems.ClearRows();
                    }
                }
                if (exceptionAlreadySelected)
                {
                    // Exception row was already selected. It just needs to be marked as selected again.
                    // No transition involved.
                    _selectedItems.SelectRow(rowIndexException, true /*select*/);
                    if (setAnchorRowIndex)
                    {
                        this.AnchorRowIndex = rowIndexException;
                    }
                }
                else
                {
                    // Exception row was not selected. It needs to be selected with potential transition
                    SetRowSelection(rowIndexException, true /*isSelected*/, setAnchorRowIndex);
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
        }

        internal Visibility GetRowDetailsVisibility(int rowIndex)
        {
            return GetRowDetailsVisibility(rowIndex, this.RowDetailsVisibilityMode);   
        }

        internal Visibility GetRowDetailsVisibility(int rowIndex, DataGridRowDetailsVisibilityMode gridLevelRowDetailsVisibility)
        {
            Debug.Assert(rowIndex != -1);
            if (_showDetailsTable.Contains(rowIndex))
            {
                // The user explicity set DetailsVisibility on a row so we should respect that
                return _showDetailsTable.GetValueAt(rowIndex);
            }
            else
            {
                if (gridLevelRowDetailsVisibility == DataGridRowDetailsVisibilityMode.Visible ||
                    (gridLevelRowDetailsVisibility == DataGridRowDetailsVisibilityMode.VisibleWhenSelected &&
                     _selectedItems.Contains(rowIndex)))
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
        }

        /// <summary>
        /// Returns the row associated to the provided backend data item.
        /// </summary>
        /// <param name="dataItem">backend data item</param>
        /// <returns>null if the datasource is null, the provided item in not in the source, or the item is not displayed; otherwise, the associated Row</returns>
        internal DataGridRow GetRowFromItem(object dataItem)
        {
            int rowIndex = this.DataConnection.IndexOf(dataItem);
            if (rowIndex < 0 || !this.DisplayData.IsRowDisplayed(rowIndex))
            {
                return null;
            }
            return this.DisplayData.GetDisplayedRow(rowIndex);
        }

        internal bool GetRowSelection(int rowIndex)
        {
            Debug.Assert(rowIndex != -1);
            return _selectedItems.Contains(rowIndex);
        }

        internal void InsertRows(int rowIndex, int rowCount)
        {
            Debug.Assert(this.ColumnsItemsInternal.Count > 0);
            Debug.Assert(rowIndex >= 0 && rowIndex <= this.RowCount);
            Debug.Assert(rowCount == 1);

            if (RowRequiresDisplay(rowIndex))
            {
                // Row at that index needs to be displayed
                InsertRow(rowIndex);
            }
            else
            {
                // Row at that index is off screen
                DataGridCellCoordinates newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);
                OnInsertingRow(rowIndex, ref newCurrentCellCoordinates, true /*firstInsertion*/, 1 /*insertionCount*/);   // will throw an exception if the insertion is illegal
                this.RowCount++;
                OnInsertedRow_Phase1(rowIndex, false /*generateRow*/, 1);
                OnInsertedRow_Phase2(rowIndex, newCurrentCellCoordinates, true /*lastInsertion*/, 
                    this._vScrollBar == null || this._vScrollBar.Visibility == Visibility.Visible /*updateVerticalScrollBarOnly*/);
                OnRowsChanged(true /*rowsGrew*/);
            }
        }

        internal bool IsColumnDisplayed(int columnIndex)
        {
            return columnIndex >= this.FirstDisplayedColumnIndex && columnIndex <= this.DisplayData.LastTotallyDisplayedScrollingCol;
        }

        internal bool IsRowRecyclable(DataGridRow row)
        {
            return (row != this._editingRow && row != this._focusedRow);
        }

        // detailsElement is the FrameworkElement created by the DetailsTemplate
        internal void OnUnloadingRowDetails(DataGridRow row, FrameworkElement detailsElement)
        {
            OnUnloadingRowDetails(new DataGridRowDetailsEventArgs(row, detailsElement));
        }

        // detailsElement is the FrameworkElement created by the DetailsTemplate
        internal void OnLoadingRowDetails(DataGridRow row, FrameworkElement detailsElement)
        {
            OnLoadingRowDetails(new DataGridRowDetailsEventArgs(row, detailsElement));
        }

        internal void OnRowDetailsVisibilityPropertyChanged(int rowIndex, Visibility visibility)
        {
            Debug.Assert(rowIndex >= 0 && rowIndex < this.RowCount);

            _showDetailsTable.AddValue(rowIndex, visibility);
        }

        internal void OnRowsMeasure()
        {
            if (!DoubleUtil.IsZero(this.DisplayData.PendingVerticalScrollHeight))
            {
                ScrollRowsByHeight(this.DisplayData.PendingVerticalScrollHeight);
                this.DisplayData.PendingVerticalScrollHeight = 0;
            }
        }
        
        internal void RefreshRows(bool recycleRows)
        {
            ClearRows(recycleRows);

            if (this.DataConnection != null && this.ColumnsItemsInternal.Count > 0)
            {
                AddRows(0, this.DataConnection.Count);

                InvalidateMeasure();
            }
        }

        internal void RemoveRowAt(int rowIndex, object item)
        {
            Debug.Assert(rowIndex >= 0 && rowIndex < this.RowCount);
            // 

            DataGridCellCoordinates newCurrentCellCoordinates = OnRemovingRow(rowIndex, item);
            this.RowCount--;
            OnRemovedRow(rowIndex, newCurrentCellCoordinates);
            OnRowsChanged(false /*rowsGrew*/);
        }

        internal void SetRowSelection(int rowIndex, bool isSelected, bool setAnchorRowIndex)
        {
            Debug.Assert(!(!isSelected && setAnchorRowIndex));
            Debug.Assert(rowIndex >= 0 && rowIndex < this.RowCount);
            this._noSelectionChangeCount++;
            try
            {
                if (this.SelectionMode == DataGridSelectionMode.Single && isSelected)
                {
                    Debug.Assert(_selectedItems.Count <= 1);
                    if (_selectedItems.Count > 0)
                    {
                        int currentlySelectedRowIndex = _selectedItems.GetIndexes().First();
                        if (currentlySelectedRowIndex != rowIndex)
                        {
                            SelectRow(currentlySelectedRowIndex, false);
                            this._selectionChanged = true;
                        }
                    }
                }
                if (_selectedItems.Contains(rowIndex) != isSelected)
                {
                    SelectRow(rowIndex, isSelected);
                    this._selectionChanged = true;
                }
                if (setAnchorRowIndex)
                {
                    this.AnchorRowIndex = rowIndex;
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
        }

        // For now, all scenarios are for isSelected == true.
        internal void SetRowsSelection(int startRowIndex, int endRowIndex /*, bool isSelected*/)
        {
            Debug.Assert(startRowIndex >= 0 && startRowIndex < this.RowCount);
            Debug.Assert(endRowIndex >= 0 && endRowIndex < this.RowCount);
            Debug.Assert(startRowIndex <= endRowIndex);
            // 

            this._noSelectionChangeCount++;
            try
            {
                // 






                if (/*isSelected &&*/ !_selectedItems.ContainsAll(startRowIndex, endRowIndex))
                {
                    // At least one row gets selected
                    SelectRows(startRowIndex, endRowIndex, true);
                    this._selectionChanged = true;
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
        }

        #endregion Internal Methods

        #region Private Methods

        private void AddRow(int rowIndex, DataGridRow dataGridRow)
        {
            Debug.Assert(dataGridRow != null);
            Debug.Assert(dataGridRow.OwningGrid == this);
            // 
            Debug.Assert(dataGridRow.Cells.Count == this.ColumnsItemsInternal.Count);
            Debug.Assert(!dataGridRow.IsSelected);
#if DEBUG
            int columnIndex = 0;
            foreach (DataGridCell dataGridCell in dataGridRow.Cells)
            {
                Debug.Assert(dataGridCell.OwningRow == dataGridRow);
                Debug.Assert(dataGridCell.OwningColumn == this.ColumnsItemsInternal[columnIndex]);
                columnIndex++;
            }
#endif
            Debug.Assert(rowIndex == this.RowCount);

            OnAddedRow_Phase1(rowIndex, dataGridRow);
            this.RowCount++;
            OnAddedRow_Phase2(rowIndex, false /*updateVerticalScrollBarOnly*/);
            OnRowsChanged(true /*rowsGrew*/);
        }

        private void AddRows(int rowIndex, int rowCount)
        {
            int addedRows = 0;
            while (addedRows < rowCount && this.AvailableRowRoom > 0)
            {
                // Additional row needs to be displayed
                AddRow(rowIndex + addedRows, GenerateRow(rowIndex + addedRows));
                addedRows++;
            }

            if (addedRows < rowCount)
            {
                // Additional rows are off screen
                this.RowCount += (rowCount - addedRows);
                OnAddedRow_Phase2(rowIndex, this._vScrollBar == null || this._vScrollBar.Visibility == Visibility.Visible /*updateVerticalScrollBarOnly*/);
                OnRowsChanged(true /*rowsGrew*/);
            }
        }

        private void ApplyDisplayedRowsState(int startRowIndex, int endRowIndex)
        {
            int firstRow = Math.Max(this.DisplayData.FirstDisplayedScrollingRow, startRowIndex);
            int lastRow = Math.Min(this.DisplayData.LastDisplayedScrollingRow, endRowIndex);

            if (firstRow >= 0)
            {
                Debug.Assert(lastRow >= firstRow);
                for (int rowIndex = firstRow; rowIndex <= lastRow; rowIndex++)
                {
                    Debug.Assert(this.DisplayData.IsRowDisplayed(rowIndex));
                    this.DisplayData.GetDisplayedRow(rowIndex).ApplyState(true /*animate*/);
                }
            }
        }

        private void ClearRows(bool recycleRows)
        {
            if (this.RowCount > 0)
            {
                SetCurrentCellCore(-1, -1, false /*commitEdit*/);
                ClearRowSelection(true /*resetAnchorRowIndex*/);
                this.DisplayData.ClearRows(recycleRows);

                if (!recycleRows && this._rowsPresenter != null)
                {
                    foreach (DataGridRow row in _rowsPresenter.Children)
                    {
                        // Raise UnloadingRow for any row that was visible
                        if (row.Visibility == Visibility.Visible)
                        {
                            OnUnloadingRow(new DataGridRowEventArgs(row));
                        }
                        row.DetachFromDataGrid(false /*recycle*/);
                    }
                    this._rowsPresenter.Children.Clear();
                }

                this._showDetailsTable.Clear();
                this.RowCount = 0;
                this.NegVerticalOffset = 0;
                SetVerticalOffset(0);
                // 

                ComputeScrollBarsLayout();

                // Update the AvailableRowRoom since we're displaying 0 rows now
                this.AvailableRowRoom = this.CellsHeight;
            }
        }

        /// <summary>
        /// Adjusts the index of all displayed, loaded and edited rows after a row was deleted.
        /// Removes the deleted row from the list of loaded rows if present.
        /// </summary>
        private void CorrectRowIndexesAfterDeletion(int rowIndexDeleted)
        {
            Debug.Assert(rowIndexDeleted >= 0);

            // Take care of the non-displayed loaded rows
            for (int index = 0; index < this._loadedRows.Count; )
            {
                DataGridRow dataGridRow = this._loadedRows[index];
                if (this.DisplayData.IsRowDisplayed(dataGridRow.Index))
                {
                    index++;
                }
                else
                {
                    if (dataGridRow.Index > rowIndexDeleted)
                    {
                        dataGridRow.Index--;
                        index++;
                    }
                    else if (dataGridRow.Index == rowIndexDeleted)
                    {
                        this._loadedRows.RemoveAt(index);
                    }
                    else
                    {
                        index++;
                    }
                }
            }

            // Take care of the non-displayed edited row
            if (this._editingRow != null &&
                !this.DisplayData.IsRowDisplayed(this._editingRow.Index) &&
                this._editingRow.Index > rowIndexDeleted)
            {
                this._editingRow.Index--;
            }

            // Take care of the non-displayed focused row
            if (this._focusedRow != null &&
                this._focusedRow != this._editingRow &&
                !this.DisplayData.IsRowDisplayed(this._focusedRow.Index) &&
                this._focusedRow.Index > rowIndexDeleted)
            {
                this._focusedRow.Index--;
            }

            // Take care of the displayed rows
            foreach (DataGridRow row in this.DisplayData.GetScrollingRows())
            {
                if (row.Index > rowIndexDeleted)
                {
                    row.Index--;
                    row.EnsureBackground();
                }
            }

            // Update which row we've calculated the RowHeightEstimate up to
            if (_lastEstimatedRow >= rowIndexDeleted)
            {
                _lastEstimatedRow--;
            }
        }

        /// <summary>
        /// Adjusts the index of all displayed, loaded and edited rows after rows were deleted.
        /// </summary>
        private void CorrectRowIndexesAfterInsertion(int rowIndexInserted, int insertionCount)
        {
            Debug.Assert(rowIndexInserted >= 0);
            Debug.Assert(insertionCount > 0);

            // Take care of the non-displayed loaded rows
            foreach (DataGridRow dataGridRow in this._loadedRows)
            {
                if (!this.DisplayData.IsRowDisplayed(dataGridRow.Index) && dataGridRow.Index >= rowIndexInserted)
                {
                    dataGridRow.Index += insertionCount;
                }
            }

            // Take care of the non-displayed edited row
            if (this._editingRow != null &&
                !this.DisplayData.IsRowDisplayed(this._editingRow.Index) &&
                this._editingRow.Index >= rowIndexInserted)
            {
                this._editingRow.Index += insertionCount;
            }

            // Take care of the non-displayed focused row
            if (this._focusedRow != null &&
                this._focusedRow != this._editingRow &&
                !this.DisplayData.IsRowDisplayed(this._focusedRow.Index) &&
                this._focusedRow.Index >= rowIndexInserted)
            {
                this._focusedRow.Index += insertionCount;
            }

            // Take care of the displayed rows
            foreach (DataGridRow row in this.DisplayData.GetScrollingRows())
            {
                if (row.Index >= rowIndexInserted)
                {
                    row.Index += insertionCount;
                    row.EnsureBackground();
                }
            }

            // Update which row we've calculated the RowHeightEstimate up to
            if (_lastEstimatedRow >= rowIndexInserted)
            {
                _lastEstimatedRow++;
            }
        }

        private void EnsureRowDetailsVisibility(DataGridRow row, bool raiseNotification, bool animate)
        {
            // Show or hide RowDetails based on DataGrid settings
            row.SetDetailsVisibilityInternal(GetRowDetailsVisibility(row.Index), raiseNotification, animate);
        }

        // Returns the number of rows with details visible between lowerBound and upperBound exclusive
        private int GetDetailsCountExclusive(int lowerBound, int upperBound)
        {
            int indexCount = upperBound - lowerBound - 1;
            if (indexCount <= 0)
            {
                return 0;
            }
            if (this.RowDetailsVisibilityMode == DataGridRowDetailsVisibilityMode.Visible)
            {
                // Total rows minus ones which explicity turned details off
                return indexCount - _showDetailsTable.GetIndexCount(lowerBound, upperBound, Visibility.Collapsed);
            }
            else if (this.RowDetailsVisibilityMode == DataGridRowDetailsVisibilityMode.Collapsed)
            {
                // Total rows with details explicitly turned on
                return _showDetailsTable.GetIndexCount(lowerBound, upperBound, Visibility.Visible);
            }
            else if (this.RowDetailsVisibilityMode == DataGridRowDetailsVisibilityMode.VisibleWhenSelected)
            {
                // Total number of remaining rows that are selected
                return _selectedItems.GetIndexCount(lowerBound, upperBound);
            }
            Debug.Assert(false); // Shouldn't ever happen
            return 0;
        }

        private void InvalidateRowHeightEstimate()
        {
            // Start from scratch and assume that we haven't estimated any rows
            _lastEstimatedRow = -1;
        }

        private void UpdateDisplayedRows(int newFirstDisplayedRowIndex, double displayHeight)
        {
            int firstDisplayedScrollingRow = newFirstDisplayedRowIndex;
            int lastDisplayedScrollingRow = -1;
            double deltaY = -this.NegVerticalOffset;
            int visibleScrollingRows = 0;

            if (DoubleUtil.LessThanOrClose(displayHeight, 0) || this.RowCount == 0 || this.ColumnsItemsInternal.Count == 0)
            {
                return;
            }

            if (firstDisplayedScrollingRow == -1)
            {
                firstDisplayedScrollingRow = 0;
            }

            for (int i = firstDisplayedScrollingRow; i < this.RowCount; i++)
            {
                deltaY += GetEdgedExactRowHeight(i);
                visibleScrollingRows++;
                lastDisplayedScrollingRow = i;
                if (DoubleUtil.GreaterThanOrClose(deltaY, displayHeight))
                {
                    break;
                }
            }

            if (DoubleUtil.LessThan(deltaY, displayHeight))
            {
                for (int i = firstDisplayedScrollingRow - 1; i >= 0; i--)
                {
                    double height = GetEdgedExactRowHeight(i);
                    if (deltaY + height > displayHeight)
                    {
                        break;
                    }
                    deltaY += height;
                    firstDisplayedScrollingRow = i;
                    visibleScrollingRows++;
                }
                // If we're up to the first row, and we still have room left, uncover as much of the first row as we can
                if (firstDisplayedScrollingRow == 0 && DoubleUtil.LessThan(deltaY, displayHeight))
                {
                    double newNegVerticalOffset = Math.Max(0, this.NegVerticalOffset - displayHeight + deltaY);
                    deltaY += this.NegVerticalOffset - newNegVerticalOffset;
                    this.NegVerticalOffset = newNegVerticalOffset;
                }
            }

            if (DoubleUtil.GreaterThan(deltaY, displayHeight) || (DoubleUtil.AreClose(deltaY, displayHeight) && DoubleUtil.GreaterThan(this.NegVerticalOffset, 0)))
            {
                this.DisplayData.NumTotallyDisplayedScrollingRows = visibleScrollingRows - 1;
            }
            else
            {
                this.DisplayData.NumTotallyDisplayedScrollingRows = visibleScrollingRows;
            }
            if (visibleScrollingRows == 0)
            {
                firstDisplayedScrollingRow = -1;
                Debug.Assert(lastDisplayedScrollingRow == -1);
            }

            Debug.Assert(lastDisplayedScrollingRow < this.RowCount, "lastDisplayedScrollingRow larger than number of rows");

            RemoveNonDisplayedRows(firstDisplayedScrollingRow, lastDisplayedScrollingRow);

            Debug.Assert(this.DisplayData.NumDisplayedScrollingRows >= 0, "the number of visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0, "the number of totally visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow < this.RowCount, "firstDisplayedScrollingRow larger than number of rows");
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow == firstDisplayedScrollingRow);
            Debug.Assert(this.DisplayData.LastDisplayedScrollingRow == lastDisplayedScrollingRow);
        }

        // Similar to UpdateDisplayedRows except that it starts with the LastDisplayedScrollingRow
        // and computes the FirstDisplayScrollingRow instead of doing it the other way around.  We use this
        // when scrolling down to a full row
        private void UpdateDisplayedRowsFromBottom(int newLastDisplayedScrollingRow)
        {
            int lastDisplayedScrollingRow = newLastDisplayedScrollingRow; // this.DisplayData.LastDisplayedScrollingRow;
            int firstDisplayedScrollingRow = -1;
            double displayHeight = this.CellsHeight;
            double deltaY = 0;
            int visibleScrollingRows = 0;

            if (DoubleUtil.LessThanOrClose(displayHeight, 0) || this.RowCount == 0 || this.ColumnsItemsInternal.Count == 0)
            {
                this.ResetDisplayedRows();
                return;
            }

            if (lastDisplayedScrollingRow == -1)
            {
                lastDisplayedScrollingRow = 0;
            }

            for (int i = lastDisplayedScrollingRow; i >= 0; i--)
            {
                deltaY += GetEdgedExactRowHeight(i);
                visibleScrollingRows++;
                firstDisplayedScrollingRow = i;
                if (DoubleUtil.GreaterThanOrClose(deltaY, displayHeight))
                {
                    break;
                }
            }

            this.DisplayData.NumTotallyDisplayedScrollingRows = deltaY > displayHeight ? visibleScrollingRows - 1 : visibleScrollingRows;

            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0);
            Debug.Assert(lastDisplayedScrollingRow < this.RowCount, "lastDisplayedScrollingRow larger than number of rows");

            this.NegVerticalOffset = Math.Max(0, deltaY - displayHeight);

            RemoveNonDisplayedRows(firstDisplayedScrollingRow, lastDisplayedScrollingRow);

            Debug.Assert(this.DisplayData.NumDisplayedScrollingRows >= 0, "the number of visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0, "the number of totally visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow < this.RowCount, "firstDisplayedScrollingRow larger than number of rows");
        }

        private void UpdateRowDetailsHeightEstimate()
        {
            if (_rowsPresenter != null && _measured && this.RowDetailsTemplate != null)
            {
                FrameworkElement detailsContent = this.RowDetailsTemplate.LoadContent() as FrameworkElement;
                if (detailsContent != null)
                {
                    _rowsPresenter.Children.Add(detailsContent);
                    if (this.RowCount > 0)
                    {
                        detailsContent.DataContext = this.DataConnection.GetDataItem(0);
                    }
                    detailsContent.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                    this.RowDetailsHeightEstimate = detailsContent.DesiredSize.Height;
                    _rowsPresenter.Children.Remove(detailsContent);
                }
            }
        }

        /// <summary>
        /// Returns a row for the provided index. The row gets first loaded through the LoadingRow event.
        /// </summary>
        private DataGridRow GenerateRow(int rowIndex)
        {
            Debug.Assert(rowIndex > -1);
            DataGridRow dataGridRow = GetGeneratedRow(rowIndex);
            if (dataGridRow == null)
            {
                dataGridRow = this.DisplayData.GetUsedRow() ?? new DataGridRow();
                dataGridRow.Index = rowIndex;
                dataGridRow.OwningGrid = this;
                dataGridRow.DataContext = this.DataConnection.GetDataItem(rowIndex);
                CompleteCellsCollection(dataGridRow);

                OnLoadingRow(new DataGridRowEventArgs(dataGridRow));

                DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
                if (peer != null)
                {
                    peer.UpdateRowPeerEventsSource(dataGridRow);
                }
            }
            return dataGridRow;
        }

        /// <summary>
        /// Returns the exact row height, whether it is currently displayed or not.
        /// The row is generated and added to the displayed rows in case it is not already displayed.
        /// The horizontal gridlines thickness are added.
        /// </summary>
        private double GetEdgedExactRowHeight(int rowIndex)
        {
            Debug.Assert((rowIndex >= 0 /**/) && rowIndex < this.RowCount);
            double rowHeight;
            if (this.DisplayData.IsRowDisplayed(rowIndex))
            {
                rowHeight = this.DisplayData.GetDisplayedRow(rowIndex).DesiredSize.Height;
            }
            else
            {
                // We can only support creating new rows that are adjacent to the currently visible rows
                // since they need to be added to the visual tree for us to Measure them.
                Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow == -1 || (this.DisplayData.FirstDisplayedScrollingRow - rowIndex == 1) || (rowIndex - this.DisplayData.LastDisplayedScrollingRow == 1));
                DataGridRow row = GenerateRow(rowIndex);
                InsertDisplayedRow(rowIndex, row, false/*wasNewlyAdded*/);
                rowHeight = row.DesiredSize.Height;
            }
            return rowHeight;
        }

        // 








        
        
       
        
        
           
        

        /// <summary>
        /// If the provided row index is displayed, returns the exact row height.
        /// If the row is not displayed, returns the default row height.
        /// In both cases, the horizontal gridlines thickness are added.
        /// </summary>
        private double GetEdgedRowHeight(int rowIndex)
        {
            Debug.Assert((rowIndex >= 0 /**/) && rowIndex < this.RowCount);
            double rowHeight;
            if (this.DisplayData.IsRowDisplayed(rowIndex))
            {
                rowHeight = this.DisplayData.GetDisplayedRow(rowIndex).DesiredSize.Height;
            }
            else
            {
                return this.RowHeightEstimate + (GetRowDetailsVisibility(rowIndex) == Visibility.Visible ? this.RowDetailsHeightEstimate : 0);
            }
            return rowHeight;
        }

        /// <summary>
        /// Cumulates the approximate height of the rows from fromRowIndex to toRowIndex included.
        /// Including the potential gridline thickness.
        /// </summary>
        private double GetEdgedRowsHeight(int fromRowIndex, int toRowIndex)
        {
            Debug.Assert(toRowIndex >= fromRowIndex);

            double rowsHeight = 0;
            for (int rowIndex = fromRowIndex; rowIndex <= toRowIndex; rowIndex++)
            {
                rowsHeight += GetEdgedRowHeight(rowIndex);
            }
            return rowsHeight;
        }

        /// <summary>
        /// Checks if the row for the provided index has been generated and is present
        /// in either the loaded rows, prefetched rows, or editing row. 
        /// The displayed rows are *not* searched. Returns null if the row does not belong to those 3 categories.
        /// </summary>
        private DataGridRow GetGeneratedRow(int rowIndex)
        {
            // Check the list of rows being loaded via the LoadingRow event.
            DataGridRow dataGridRow = GetLoadedRow(rowIndex);
            if (dataGridRow != null)
            {
                return dataGridRow;
            }

            // Check the potential editing row.
            if (this._editingRow != null && rowIndex == this.EditingRowIndex)
            {
                return this._editingRow;
            }

            // Check the potential focused row.
            if (this._focusedRow != null && rowIndex == this._focusedRow.Index)
            {
                return this._focusedRow;
            }

            return null;
        }

        private DataGridRow GetLoadedRow(int rowIndex)
        {
            foreach (DataGridRow dataGridRow in this._loadedRows)
            {
                if (dataGridRow.Index == rowIndex)
                {
                    return dataGridRow;
                }
            }
            return null;
        }

        private int GetNextRow(int rowIndex)
        {
            int result = rowIndex + 1;
            return (result < this.RowCount) ? result : -1;
        }

        private static int GetPreviousRow(int rowIndex)
        {
            int result = rowIndex - 1;
            return (result >= 0) ? result : -1;
        }

        private void InsertDisplayedRow(int rowIndex, DataGridRow dataGridRow, bool wasNewlyAdded)
        {
            Debug.Assert(dataGridRow != null);

            LoadRowVisualsForDisplay(dataGridRow);

            if (this._rowsPresenter != null)
            {
                
                if (IsRowRecyclable(dataGridRow))
                {
                    if (dataGridRow.IsRecycled)
                    {
                        dataGridRow.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        Debug.Assert(!this._rowsPresenter.Children.Contains(dataGridRow));
                        this._rowsPresenter.Children.Add(dataGridRow);
                    }
                }
                else
                {
                    dataGridRow.Clip = null;
                    Debug.Assert(dataGridRow.Index == rowIndex);
                }

                // Measure the row and update AvailableRowRoom
                dataGridRow.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                this.AvailableRowRoom -= dataGridRow.DesiredSize.Height;

                // 
                if (this.RowHeightEstimate == DataGrid.DATAGRID_defaultRowHeight && double.IsNaN(dataGridRow.Height))
                {
                    this.RowHeightEstimate = dataGridRow.DesiredSize.Height;
                }
            }

            if (wasNewlyAdded)
            {
                this.DisplayData.CorrectRowsAfterInsertion(rowIndex, dataGridRow);
            }
            else
            {
                this.DisplayData.LoadScrollingRow(rowIndex, dataGridRow);
            }
        }

        private void InsertRow(int rowIndex)
        {
            Debug.Assert(rowIndex >= 0 && rowIndex <= this.RowCount);

            DataGridCellCoordinates newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);

            OnInsertingRow(rowIndex, ref newCurrentCellCoordinates, true /*firstInsertion*/, 1 /*insertionCount*/);   // will throw an exception if the insertion is illegal

            OnInsertedRow_Phase1(rowIndex, true /*generateRow*/, 1);
            this.RowCount++;
            OnInsertedRow_Phase2(rowIndex, newCurrentCellCoordinates, true /*lastInsertion*/, false /*updateVerticalScrollBarOnly*/);
            OnRowsChanged(true /*rowsGrew*/);
        }

        private void OnAddedRow_Phase1(int rowIndex, DataGridRow dataGridRow)
        {
            Debug.Assert(rowIndex >= 0);
            // Row needs to be potentially added to the displayed rows
            if (RowRequiresDisplay(rowIndex))
            {
                InsertDisplayedRow(rowIndex, dataGridRow, true /*wasNewlyAdded*/);
            }
        }

        private void OnAddedRow_Phase2(int rowIndex, bool updateVerticalScrollBarOnly)
        {
            if (rowIndex < this.DisplayData.FirstDisplayedScrollingRow - 1)
            {
                // The row was added above our viewport so it pushes the VerticalOffset down
                SetVerticalOffset(_verticalOffset + this.RowHeightEstimate);
            }
            if (updateVerticalScrollBarOnly)
            {
                UpdateVerticalScrollBar();
            }
            else
            {
                ComputeScrollBarsLayout();
                // Reposition rows in case we use a recycled one
                InvalidateRowsArrange();
            }
        }

        private void OnInsertedRow_Phase1(int rowIndex, bool generateRow, int insertionCount)
        {
            Debug.Assert(rowIndex >= 0);
            Debug.Assert(insertionCount > 0);

            // Fix the Index of all following rows
            CorrectRowIndexesAfterInsertion(rowIndex, insertionCount);

            // Next, same effect as adding a row
            if (generateRow)
            {
                DataGridRow dataGridRow = GenerateRow(rowIndex);
                Debug.Assert(dataGridRow.Cells.Count == this.ColumnsItemsInternal.Count);
#if DEBUG
                int columnIndex = 0;
                foreach (DataGridCell dataGridCell in dataGridRow.Cells)
                {
                    Debug.Assert(dataGridCell.OwningRow == dataGridRow);
                    Debug.Assert(dataGridCell.OwningColumn == this.ColumnsItemsInternal[columnIndex]);
                    columnIndex++;
                }
#endif
                dataGridRow.OwningGrid = this;
                OnAddedRow_Phase1(rowIndex, dataGridRow);
            }
            else if (rowIndex <= this.DisplayData.FirstDisplayedScrollingRow)
            {
                this.DisplayData.CorrectRowsAfterInsertion(rowIndex, null /*row*/);
            }
        }

        private void OnInsertedRow_Phase2(int rowIndex, DataGridCellCoordinates newCurrentCellCoordinates,
                                          bool lastInsertion, bool updateVerticalScrollBarOnly)
        {
            Debug.Assert(rowIndex >= 0);

            // Same effect as adding a row
            OnAddedRow_Phase2(rowIndex, updateVerticalScrollBarOnly);

            // Update current cell if needed
            if (lastInsertion && newCurrentCellCoordinates.RowIndex != -1)
            {
                Debug.Assert(this.CurrentColumnIndex == -1);
                bool success = SetAndSelectCurrentCell(newCurrentCellCoordinates.ColumnIndex,
                                                       newCurrentCellCoordinates.RowIndex,
                                                       this.RowCount == 1 /*forceCurrentCellSelection*/);
                Debug.Assert(success);
            }
        }

        private void OnInsertingRow(int rowIndexInserted,
                                    ref DataGridCellCoordinates newCurrentCellCoordinates,
                                    bool firstInsertion,
                                    int insertionCount)
        {
            // Reset the current cell's address if it's after the inserted row.
            if (firstInsertion)
            {
                if (this.CurrentRowIndex != -1 && rowIndexInserted <= this.CurrentRowIndex)
                {
                    newCurrentCellCoordinates = new DataGridCellCoordinates(this.CurrentColumnIndex, this.CurrentRowIndex + insertionCount);
                    // The underlying data was already added, therefore we need to avoid accessing any back-end data since we might be off by 1 row.
                    this._temporarilyResetCurrentCell = true;
                    bool success = SetCurrentCellCore(-1, -1/* */);
                    Debug.Assert(success);
                }
                else
                {
                    newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);
                }
            }
            else
            {
                if (newCurrentCellCoordinates.RowIndex != -1)
                {
                    newCurrentCellCoordinates.RowIndex += insertionCount;
                }
            }

            // Update the ranges of selected rows to compensate for the insertion of this row
            _selectedItems.InsertIndexes(rowIndexInserted, insertionCount);
            // Same for details visibility settings
            this._showDetailsTable.InsertIndexes(rowIndexInserted, insertionCount);

            // If we've inserted rows before the current selected item, update its index
            if (rowIndexInserted <= this.SelectedIndex)
            {
                this.SetValueNoCallback(SelectedIndexProperty, this.SelectedIndex + insertionCount);
            }
        }

        private void OnRemovedRow(int rowIndexDeleted, DataGridCellCoordinates newCurrentCellCoordinates)
        {
            CorrectRowIndexesAfterDeletion(rowIndexDeleted);

            DataGridRow deletedRow = null;
            // Row needs to be potentially removed from the displayed rows
            if (rowIndexDeleted <= this.DisplayData.LastDisplayedScrollingRow)
            {
                if (rowIndexDeleted >= this.DisplayData.FirstDisplayedScrollingRow)
                {
                    // Displayed row is removed
                    deletedRow = RemoveDisplayedRow(rowIndexDeleted, true);
                }
                else
                {
                    // Removed row is above our viewport, just update the DisplayData
                    this.DisplayData.CorrectRowsAfterDeletion(rowIndexDeleted);
                }
            }

            // Update current cell if needed
            if (newCurrentCellCoordinates.RowIndex != -1)
            {
                Debug.Assert(this.CurrentColumnIndex == -1);
                bool success = SetAndSelectCurrentCell(newCurrentCellCoordinates.ColumnIndex,
                                                       newCurrentCellCoordinates.RowIndex,
                                                       false /*forceCurrentCellSelection*/);
                Debug.Assert(success);
            }

            // Raise SelectionChange event if needed
            FlushSelectionChanged();

            if (rowIndexDeleted-1 > this.DisplayData.LastDisplayedScrollingRow && deletedRow == null)
            {
                // Deleted Row is below our Viewport, we just need to adjust the scrollbar
                UpdateVerticalScrollBar();
            }
            else
            {
                if (deletedRow != null)
                {
                    // Deleted Row is within our Viewport, update the AvailableRowRoom
                    this.AvailableRowRoom += deletedRow.DesiredSize.Height;
                }
                else
                {
                    // Deleted Row is above our Viewport, update the vertical offset
                    SetVerticalOffset(Math.Max(0, _verticalOffset - this.RowHeightEstimate));
                }

                ComputeScrollBarsLayout();
                // Reposition rows in case we use a recycled one
                InvalidateRowsArrange();
            }
        }

        private DataGridCellCoordinates OnRemovingRow(int rowIndexDeleted, object itemDeleted)
        {
            // Note that the row needs to be deleted no matter what. The underlying data row was already deleted.

            DataGridCellCoordinates newCurrentCellCoordinates;
            Debug.Assert(rowIndexDeleted >= 0 && rowIndexDeleted < this.RowCount);

            this._temporarilyResetCurrentCell = false; 
            newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);

            // Reset the current cell's address if it's on the deleted row, or after it.
            if (this.CurrentRowIndex != -1 && rowIndexDeleted <= this.CurrentRowIndex)
            {
                int newCurrentRowIndex;
                if (rowIndexDeleted == this.CurrentRowIndex)
                {
                    int rowIndexPrevious = rowIndexDeleted - 1;
                    int rowIndexNext = (rowIndexDeleted == this.RowCount - 1) ? -1 : rowIndexDeleted + 1;
                    if (rowIndexNext > -1)
                    {
                        newCurrentRowIndex = rowIndexNext - 1;
                    }
                    else
                    {
                        newCurrentRowIndex = rowIndexPrevious;
                    }
                }
                else
                {
                    Debug.Assert(rowIndexDeleted < this.CurrentRowIndex);
                    newCurrentRowIndex = this.CurrentRowIndex - 1;
                }
                newCurrentCellCoordinates = new DataGridCellCoordinates(this.CurrentColumnIndex, newCurrentRowIndex);
                if (rowIndexDeleted == this.CurrentRowIndex)
                {
                    // No editing is committed since the underlying entity was already deleted.
                    bool success = SetCurrentCellCore(-1, -1, false /*commitEdit*/ /* */);
                    Debug.Assert(success);
                }
                else
                {
                    // Underlying data of deleted row is gone. It cannot be accessed anymore. Skip the commit of the editing.
                    this._temporarilyResetCurrentCell = true;
                    bool success = SetCurrentCellCore(-1, -1/* */);
                    Debug.Assert(success);
                }
            }

            // Update the ranges of selected rows
            if (_selectedItems.Contains(rowIndexDeleted))
            {
                this._selectionChanged = true;
            }
            _selectedItems.Delete(rowIndexDeleted, itemDeleted);

            // If a row was removed before the currently selected row, update its index
            if (rowIndexDeleted < this.SelectedIndex)
            {
                this.SetValueNoCallback(SelectedIndexProperty, this.SelectedIndex - 1);
            }

            return newCurrentCellCoordinates;
        }

        private void OnRowsChanged(bool rowsGrew)
        {
            if (rowsGrew &&
                this.Columns.Count > 0 &&
                this.CurrentColumnIndex == -1)
            {
                MakeFirstDisplayedCellCurrentCell();
            }
        }

        // Makes sure the row shows the proper visuals for selection, currency, details, etc.
        private void LoadRowVisualsForDisplay(DataGridRow row)
        {
            // If the row has been recycled, reapply the BackgroundBrush
            if (row.IsRecycled)
            {
                row.EnsureBackground();
                row.ApplyCellsState(false /*animate*/);

                // 
                row.ReAttachHandlers();
            }
            else if (row == this._editingRow)
            {
                // 

                row.ApplyCellsState(false /*animate*/);
            }

            // Set the Row's Style if we one's defined at the DataGrid level and the user didn't
            // set one at the row level
            EnsureRowStyle(row, null, this.RowStyle);
            row.EnsureHeaderStyleAndVisibility();

            // Check to see if the row contains the CurrentCell, apply its state.
            if (this.CurrentColumnIndex != -1 &&
                this.CurrentRowIndex != -1 &&
                row.Index == this.CurrentRowIndex)
            {
                row.Cells[this.CurrentColumnIndex].ApplyCellState(false /*animate*/);
            }

            if (row.IsSelected || row.IsRecycled)
            {
                row.ApplyState(false);
            }

            // Show or hide RowDetails based on DataGrid settings
            EnsureRowDetailsVisibility(row, false /*raiseNotification*/, false /*animate*/);
        }

        // Removes a row from display either because it was deleted or it was scrolled out of view
        private DataGridRow RemoveDisplayedRow(int rowIndex, bool wasDeleted)
        {
            Debug.Assert(rowIndex >= this.DisplayData.FirstDisplayedScrollingRow &&
                         rowIndex <= this.DisplayData.LastDisplayedScrollingRow);

            DataGridRow dataGridRow = this.DisplayData.GetDisplayedRow(rowIndex);
            Debug.Assert(dataGridRow != null);
            if (IsRowRecyclable(dataGridRow))
            {
                UnloadRow(dataGridRow);
            }
            else
            {
                dataGridRow.Clip = new RectangleGeometry();
            }

            // Update DisplayData
            if (wasDeleted)
            {
                this.DisplayData.CorrectRowsAfterDeletion(rowIndex);
            }
            else
            {
                this.DisplayData.UnloadScrollingRow(rowIndex);
            }
            return dataGridRow;
        }

        private void RemoveNonDisplayedRows(int newFirstDisplayedRowIndex, int newLastDisplayedRowIndex)
        {
            while (this.DisplayData.FirstDisplayedScrollingRow < newFirstDisplayedRowIndex)
            {
                // Need to add rows above the lastDisplayedScrollingRow
                RemoveDisplayedRow(this.DisplayData.FirstDisplayedScrollingRow, false /*wasDeleted*/);
            }
            while (this.DisplayData.LastDisplayedScrollingRow > newLastDisplayedRowIndex)
            {
                // Need to remove rows below the lastDisplayedScrollingRow
                RemoveDisplayedRow(this.DisplayData.LastDisplayedScrollingRow, false /*wasDeleted*/);
            }
        }

        private void ResetDisplayedRows()
        {
            // Raise Unloading Row for all the rows we're displaying
            if (this.UnloadingRow != null)
            {
                foreach (DataGridRow row in this.DisplayData.GetScrollingRows())
                {
                    if (IsRowRecyclable(row))
                    {
                        OnUnloadingRow(new DataGridRowEventArgs(row));
                    }
                }
            }
            this.DisplayData.ClearRows(true /*recycleRows*/);
        }

        /// <summary>
        /// Determines whether the row at the provided index must be displayed or not.
        /// </summary>
        private bool RowRequiresDisplay(int rowIndex)
        {
            Debug.Assert(rowIndex >= 0);

            if (rowIndex >= this.DisplayData.FirstDisplayedScrollingRow &&
                rowIndex <= this.DisplayData.LastDisplayedScrollingRow)
            {
                // Additional row takes the spot of a displayed row - it is necessarilly displayed
                return true;
            }
            else if (this.DisplayData.FirstDisplayedScrollingRow == -1 &&
                     this.CellsHeight > 0 &&
                     this.CellsWidth > 0)
            {
                return true;
            }
            else if (rowIndex == this.DisplayData.LastDisplayedScrollingRow + 1)
            {
                if (this.AvailableRowRoom > 0)
                {
                    // There is room for this additional row
                    return true;
                }
            }
            else
            {
                // 



            }
            return false;
        }

        private bool ScrollRowIntoView(int rowIndex)
        {
            Debug.Assert(!IsRowOutOfBounds(rowIndex));

            // 






            if (this.DisplayData.FirstDisplayedScrollingRow < rowIndex && this.DisplayData.LastDisplayedScrollingRow > rowIndex)
            {
                // The row is already displayed in its entirety
                return true;
            }
            else if (this.DisplayData.FirstDisplayedScrollingRow == rowIndex && rowIndex != -1)
            {
                if (!DoubleUtil.IsZero(this.NegVerticalOffset))
                {
                    // First displayed row is partially scrolled of. Let's scroll it so that this.NegVerticalOffset becomes 0.
                    this.DisplayData.PendingVerticalScrollHeight = -this.NegVerticalOffset;
                    InvalidateRowsMeasure(false /*invalidateIndividualRows*/);
                }
                return true;
            }

            double deltaY = 0;
            int firstFullRow;
            if (this.DisplayData.FirstDisplayedScrollingRow > rowIndex)
            {
                // Scroll up to the new row so it becomes the first displayed row
                firstFullRow = this.DisplayData.FirstDisplayedScrollingRow - 1;
                if (DoubleUtil.GreaterThan(this.NegVerticalOffset, 0))
                {
                    deltaY = -this.NegVerticalOffset;
                }
                deltaY -= GetEdgedRowsHeight(rowIndex, firstFullRow);
                if (this.DisplayData.FirstDisplayedScrollingRow - rowIndex > 1)
                {
                    // 

                    ResetDisplayedRows();
                }
                this.NegVerticalOffset = 0;
                UpdateDisplayedRows(rowIndex, this.CellsHeight);
            }
            else if (this.DisplayData.LastDisplayedScrollingRow <= rowIndex)
            {
                // Scroll down to the new row so it's entirely displayed.  If the height of the row
                // is greater than the height of the DataGrid, then show the top of the row at the top
                // of the grid
                firstFullRow = this.DisplayData.LastDisplayedScrollingRow;
                // Figure out how much of the last row is cut off
                double rowHeight = GetEdgedExactRowHeight(this.DisplayData.LastDisplayedScrollingRow);
                double availableHeight = this.AvailableRowRoom + rowHeight;
                if (DoubleUtil.AreClose(rowHeight, availableHeight))
                {
                    if (this.DisplayData.LastDisplayedScrollingRow == rowIndex)
                    {
                        // We're already at the very bottom so we don't need to scroll down further
                        return true;
                    }
                    else
                    {
                        // We're already showing the entire last row so don't count it as part of the delta
                        firstFullRow++;
                    }
                }
                else if (rowHeight > availableHeight)
                {
                    firstFullRow++;
                    deltaY += rowHeight - availableHeight;
                }
                // sum up the height of the rest of the full rows
                if (rowIndex >= firstFullRow)
                {
                    deltaY += GetEdgedRowsHeight(firstFullRow, rowIndex);
                }
                // If the first row we're displaying is no longer adjacent to the rows we have
                // simply discard the ones we have
                if (rowIndex - this.DisplayData.LastDisplayedScrollingRow > 1)
                {
                    ResetDisplayedRows();
                }
                if (DoubleUtil.GreaterThanOrClose(GetEdgedExactRowHeight(rowIndex), this.CellsHeight))
                {
                    // The entire row won't fit in the DataGrid so we start showing it from the top
                    this.NegVerticalOffset = 0;
                    UpdateDisplayedRows(rowIndex, this.CellsHeight);
                }
                else
                {
                    UpdateDisplayedRowsFromBottom(rowIndex);
                }
            }

            this._verticalOffset += deltaY;
            if (this._verticalOffset < 0 || this.DisplayData.FirstDisplayedScrollingRow == 0)
            {
                // We scrolled too far because a row's height was larger than its approximation
                this._verticalOffset = this.NegVerticalOffset;
            }

            // 
            Debug.Assert(DoubleUtil.LessThanOrClose(this.NegVerticalOffset, this._verticalOffset));

            SetVerticalOffset(_verticalOffset);

            InvalidateMeasure();
            InvalidateRowsMeasure(false /*invalidateIndividualRows*/);

            return true;
        }

        // Updates display information and displayed rows after scrolling the given number of pixels
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private void ScrollRowsByHeight(double height)
        {
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= 0);
            Debug.Assert(!DoubleUtil.IsZero(height));

            double deltaY = 0;
            int newFirstVisibleScrollingRow = this.DisplayData.FirstDisplayedScrollingRow;
            double newVerticalOffset = this._verticalOffset + height;
            if (height > 0)
            {
                // Scrolling Down
                if (this._vScrollBar != null && DoubleUtil.AreClose(this._vScrollBar.Maximum, newVerticalOffset))
                {
                    
                    
                    
                    ResetDisplayedRows();
                    UpdateDisplayedRowsFromBottom(this.RowCount - 1);
                    newFirstVisibleScrollingRow = this.DisplayData.FirstDisplayedScrollingRow;
                }
                else
                {
                    deltaY = GetEdgedRowHeight(newFirstVisibleScrollingRow) - this.NegVerticalOffset;
                    if (DoubleUtil.LessThan(height, deltaY))
                    {
                        // We've merely covered up more of the same row we're on
                        this.NegVerticalOffset += height;
                    }
                    else
                    {
                        // Figure out what row we've scrolled down to and update the value for this.NegVerticalOffset
                        this.NegVerticalOffset = 0;
                        // 

                        if (height > 2 * this.CellsHeight &&
                            (this.RowDetailsVisibilityMode != DataGridRowDetailsVisibilityMode.VisibleWhenSelected || this.RowDetailsTemplate == null))
                        {
                            // Very large scroll occurred. Instead of determining the exact number of scrolled off rows,
                            // let's estimate the number based on this.RowHeight.
                            ResetDisplayedRows();
                            double singleRowHeightEstimate = this.RowHeightEstimate + (this.RowDetailsVisibilityMode == DataGridRowDetailsVisibilityMode.Visible ? this.RowDetailsHeightEstimate : 0);
                            newFirstVisibleScrollingRow = Math.Min(newFirstVisibleScrollingRow + (int)(height / singleRowHeightEstimate), this.RowCount - 1);
                        }
                        else
                        {
                            while (DoubleUtil.LessThanOrClose(deltaY, height))
                            {
                                if (newFirstVisibleScrollingRow < RowCount - 1)
                                {
                                    if (this.DisplayData.IsRowDisplayed(newFirstVisibleScrollingRow))
                                    {
                                        // Make the top row available for reuse
                                        RemoveDisplayedRow(newFirstVisibleScrollingRow, false /*wasDeleted*/);
                                    }
                                    newFirstVisibleScrollingRow++;
                                }
                                else
                                {
                                    // We're being told to scroll beyond the last row, ignore the extra
                                    this.NegVerticalOffset = 0;
                                    break;
                                }

                                double rowHeight = GetEdgedExactRowHeight(newFirstVisibleScrollingRow);
                                double remainingHeight = height - deltaY;
                                if (DoubleUtil.LessThanOrClose(rowHeight, remainingHeight))
                                {
                                    deltaY += rowHeight;
                                }
                                else
                                {
                                    this.NegVerticalOffset = remainingHeight;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // Scrolling Up
                if (DoubleUtil.GreaterThanOrClose(height + this.NegVerticalOffset, 0))
                {
                    // We've merely exposing more of the row we're on
                    this.NegVerticalOffset += height;
                }
                else
                {
                    // Figure out what row we've scrolled up to and update the value for this.NegVerticalOffset
                    deltaY = -this.NegVerticalOffset;
                    this.NegVerticalOffset = 0;
                    // 

                    if (height < -2 * this.CellsHeight &&
                        (this.RowDetailsVisibilityMode != DataGridRowDetailsVisibilityMode.VisibleWhenSelected || this.RowDetailsTemplate == null))
                    {
                        // Very large scroll occurred. Instead of determining the exact number of scrolled off rows,
                        // let's estimate the number based on this.RowHeight.
                        if (newVerticalOffset == 0)
                        {
                            newFirstVisibleScrollingRow = 0;
                        }
                        else
                        {
                            double singleRowHeightEstimate = this.RowHeightEstimate + (this.RowDetailsVisibilityMode == DataGridRowDetailsVisibilityMode.Visible ? this.RowDetailsHeightEstimate : 0);
                            newFirstVisibleScrollingRow = Math.Max(0, newFirstVisibleScrollingRow + (int)(height / singleRowHeightEstimate));
                        }
                        ResetDisplayedRows();
                    }
                    else
                    {
                        int lastVisibleScrollingRow = this.DisplayData.LastDisplayedScrollingRow;
                        while (DoubleUtil.GreaterThan(deltaY, height))
                        {
                            if (newFirstVisibleScrollingRow > 0)
                            {
                                if (this.DisplayData.IsRowDisplayed(lastVisibleScrollingRow))
                                {
                                    // Make the bottom row available for reuse
                                    RemoveDisplayedRow(lastVisibleScrollingRow, false /*wasDeleted*/);
                                    lastVisibleScrollingRow--;
                                }
                                newFirstVisibleScrollingRow--;
                            }
                            else
                            {
                                this.NegVerticalOffset = 0;
                                break;
                            }
                            double rowHeight = GetEdgedExactRowHeight(newFirstVisibleScrollingRow);
                            double remainingHeight = height - deltaY;
                            if (DoubleUtil.LessThanOrClose(rowHeight + remainingHeight, 0))
                            {
                                deltaY -= rowHeight;
                            }
                            else
                            {
                                this.NegVerticalOffset = rowHeight + remainingHeight;
                                break;
                            }
                        }
                    }
                }
                if (DoubleUtil.GreaterThanOrClose(0, newVerticalOffset) && newFirstVisibleScrollingRow != 0)
                {
                    // We've scrolled to the top of the ScrollBar, automatically place the user at the very top
                    // of the DataGrid.  If this produces very odd behavior, evaluate the RowHeight estimate.  
                    // strategy. For most data, this should be unnoticeable.
                    ResetDisplayedRows();
                    this.NegVerticalOffset = 0;
                    UpdateDisplayedRows(0, this.CellsHeight);
                    newFirstVisibleScrollingRow = 0;
                }
            }

            double firstRowHeight = GetEdgedExactRowHeight(newFirstVisibleScrollingRow);
            if (DoubleUtil.LessThan(firstRowHeight, this.NegVerticalOffset))
            {
                // We've scrolled off more of the first row than what's possible.  This can happen
                // if the first row got shorter (Ex: Collpasing RowDetails) or if the user has a recycling
                // cleanup issue.  In this case, simply try to display the next row as the first row instead
                if (newFirstVisibleScrollingRow < this.RowCount - 1)
                {
                    newFirstVisibleScrollingRow++;
                }
                this.NegVerticalOffset = 0;
            }

            UpdateDisplayedRows(newFirstVisibleScrollingRow, this.CellsHeight);

            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= 0);
            Debug.Assert(GetEdgedExactRowHeight(this.DisplayData.FirstDisplayedScrollingRow) > this.NegVerticalOffset);

            if (this.DisplayData.FirstDisplayedScrollingRow == 0)
            {
                this._verticalOffset = this.NegVerticalOffset;
            }
            else if (DoubleUtil.GreaterThan(this.NegVerticalOffset, newVerticalOffset))
            {
                // The scrolled-in row was larger than anticipated. Adjust the DataGrid so the ScrollBar thumb
                // can stay in the same place
                this.NegVerticalOffset = newVerticalOffset;
                this._verticalOffset = newVerticalOffset;
            }
            else
            {
                this._verticalOffset = newVerticalOffset;
            }

            Debug.Assert(!(this._verticalOffset == 0 && this.NegVerticalOffset == 0 && this.DisplayData.FirstDisplayedScrollingRow > 0));

            SetVerticalOffset(_verticalOffset);

            this.DisplayData.FullyRecycleRows();

            Debug.Assert(DoubleUtil.GreaterThanOrClose(this.NegVerticalOffset, 0));
            Debug.Assert(DoubleUtil.GreaterThanOrClose(this._verticalOffset,  this.NegVerticalOffset));

            DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
            if (peer != null)
            {
                peer.RaiseAutomationScrollEvents();
            }
        }

        private void SelectRow(int rowIndex, bool isSelected)
        {
            _selectedItems.SelectRow(rowIndex, isSelected);
            if (this.DisplayData.IsRowDisplayed(rowIndex))
            {
                DataGridRow row = this.DisplayData.GetDisplayedRow(rowIndex);
                Debug.Assert(row != null);
                row.ApplyState(true /*animate*/);
                EnsureRowDetailsVisibility(row, true /*raiseNotification*/, true /*animate*/);
            }
        }

        private void SelectRows(int startRowIndex, int endRowIndex, bool isSelected)
        {
            _selectedItems.SelectRows(startRowIndex, endRowIndex - startRowIndex + 1 /*rowCount*/, isSelected);
            
            // Apply the correct row state for display rows and also expand or collapse detail accordingly
            int firstRow = Math.Max(this.DisplayData.FirstDisplayedScrollingRow, startRowIndex);
            int lastRow = Math.Min(this.DisplayData.LastDisplayedScrollingRow, endRowIndex);

            for (int rowIndex = firstRow; rowIndex <= lastRow; rowIndex++)
            {
                Debug.Assert(this.DisplayData.IsRowDisplayed(rowIndex));
                DataGridRow row = this.DisplayData.GetDisplayedRow(rowIndex);
                Debug.Assert(row != null);
                row.ApplyState(true /*animate*/);
                EnsureRowDetailsVisibility(row, true /*raiseNotification*/, true /*animate*/);
            }
        }

        private void UnloadRow(DataGridRow dataGridRow)
        {
            Debug.Assert(dataGridRow != null);
            Debug.Assert(this._rowsPresenter != null);
            Debug.Assert(this._rowsPresenter.Children.Contains(dataGridRow));

            if (this._loadedRows.Contains(dataGridRow))
            {
                // 
                return; // The row is still referenced, we can't release it.
            }

            // Raise UnloadingRow regardless of whether the row will be recycled
            OnUnloadingRow(new DataGridRowEventArgs(dataGridRow));

            // 
            bool recycleRow = this.CurrentRowIndex != dataGridRow.Index;

            // Don't recycle if the row has a custom Style set
            recycleRow &= (dataGridRow.Style == null || dataGridRow.Style == this.RowStyle);

            if (recycleRow)
            {
                this.DisplayData.AddRecylableRow(dataGridRow);
            }
            else
            {
                // 
                this._rowsPresenter.Children.Remove(dataGridRow);
                dataGridRow.DetachFromDataGrid(false);
            }
        }

        #endregion Private Methods
    }
}
