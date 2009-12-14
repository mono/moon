// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Controls;
 
namespace System.Windows.Controlsb1
{ 
    public partial class DataGrid
    {
        #region Internal Properties 

        internal bool AreRowBottomGridlinesRequired
        { 
            get 
            {
                return (this.GridlinesVisibility == DataGridGridlines.Horizontal || this.GridlinesVisibility == DataGridGridlines.All) && this.HorizontalGridlinesBrush != null /*&& this.HorizontalGridlinesThickness > 0*/; 
            }
        }
 
        internal int FirstRowIndex
        {
            get 
            { 
                return (this._rowCount > 0) ? 0 : -1;
            } 
        }

        internal int LastRowIndex 
        {
            get
            { 
                return (this._rowCount > 0) ? this._rowCount - 1 : -1; 
            }
        } 

        #endregion Internal Properties
 
        #region Private Properties

        // Returns the height of the empty room available below the row 
        // at index this._lastDisplayedRowIndex 
        private double AvailableRowRoom
        { 
            get
            {
                if (this._lastDisplayedRowIndex > -1) 
                {
                    DataGridRow lastDisplayedScrollingRow = GetDisplayedRow(this._lastDisplayedRowIndex);
                    if (lastDisplayedScrollingRow != null) 
                    { 
                        double bottomEdge = 0;
                        if (this._layoutSuspended == 0) 
                        {
                            bottomEdge = (double)lastDisplayedScrollingRow.GetValue(Canvas.TopProperty) + GetEdgedRowHeight(this._lastDisplayedRowIndex);
                        } 
                        else
                        {
                            Debug.Assert(this._firstDisplayedRowIndex > -1); 
                            int rowIndex = this._firstDisplayedRowIndex; 
                            bottomEdge = -this._negVerticalOffset;
                            while (bottomEdge < this.CellsHeight && rowIndex <= this._lastDisplayedRowIndex) 
                            {
                                bottomEdge += GetEdgedRowHeight(rowIndex);
                                rowIndex++; 
                            }
                        }
                        return this.CellsHeight - bottomEdge; 
                    } 
                }
                return this.CellsHeight; 
            }
        }
 
        // Cumulated height of all known rows, including the gridlines and details section.
        // This property returns an approximation of the actual total row heights.
        private double EdgedRowsHeight 
        { 
            get
            { 
                double horizontalGridlinesThickness = this.AreRowBottomGridlinesRequired ? DataGrid.HorizontalGridlinesThickness : 0;
                double edgedRowsHeight = 0;
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++) 
                {
                    DataGridRow dataGridRow = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex);
                    edgedRowsHeight += dataGridRow.DisplayHeight + horizontalGridlinesThickness; 
                } 
                if (this._rowCount > this.DisplayedRowCount)
                { 
                    if (this._verticalOffset > 0)
                    {
                        // Take advantage of the good approximation of the height of the scrolled 
                        // off rows, given by this._verticalOffset - this._negVerticalOffset.
                        Debug.Assert(DoubleUtil.GreaterThanOrClose(this._negVerticalOffset, 0));
                        Debug.Assert(DoubleUtil.LessThanOrClose(this._negVerticalOffset, this._verticalOffset)); 
                        edgedRowsHeight += this._verticalOffset - this._negVerticalOffset; 

                        // Take advantage of the known height of the prefetched rows that appear below 
                        // the last displayed row.
                        Debug.Assert(this._lastDisplayedRowIndex >= 0);
                        int unknownHeights = this._rowCount - this._lastDisplayedRowIndex - 1; 
                        foreach (DataGridRow dataGridRow in this._prefetchedRows)
                        {
                            if (dataGridRow.Index > this._lastDisplayedRowIndex) 
                            { 
                                // Prefetched row appears below the last displayed row. Use its exact height.
                                unknownHeights--; 
                                Debug.Assert(unknownHeights >= 0);
                                edgedRowsHeight += dataGridRow.DisplayHeight + horizontalGridlinesThickness;
                            } 
                        }

                        // Finally there are other rows below the last displayed row that haven't been prefetched. 
                        // Their height is assumed to be the default row height. 
                        Debug.Assert(unknownHeights >= 0);
                        // 
                        edgedRowsHeight += unknownHeights * (this.RowHeight + horizontalGridlinesThickness);
                    }
                    else 
                    {
                        // Take advantage of the known height of the prefetched rows that appear above
                        // the first displayed row or below the last displayed row. 
                        int unknownHeights = this._rowCount - this.DisplayedRowCount; 
                        foreach (DataGridRow dataGridRow in this._prefetchedRows)
                        { 
                            if ((dataGridRow.Index < this._firstDisplayedRowIndex && this._firstDisplayedRowIndex != -1) ||
                                (dataGridRow.Index > this._lastDisplayedRowIndex && this._lastDisplayedRowIndex != -1))
                            { 
                                unknownHeights--;
                                Debug.Assert(unknownHeights >= 0);
                                edgedRowsHeight += dataGridRow.DisplayHeight + horizontalGridlinesThickness; 
                            } 
                        }
 
                        // Finally there are other non-displayed rows that haven't been prefetched.
                        // Their height is assumed to be the default row height.
                        Debug.Assert(unknownHeights >= 0); 
                        //
                        edgedRowsHeight += unknownHeights * (this.RowHeight + horizontalGridlinesThickness);
                    } 
                } 
                return edgedRowsHeight;
            } 
        }

        #endregion Private Properties 

        #region Public Methods
 
        /// <summary> 
        /// Returns the DetailsVisibility of the DataGridRow that contains the given element
        /// </summary> 
        /// <param name="element">FrameworkElement contained in the row</param>
        /// <returns>If the element is valid then Visible or Collapsed based on the DetailsVisibility of the DataGridRow
        /// containing the element; otherwise, throws an ArgumentException 
        /// </returns>
        public Visibility GetRowDetailsVisibility(FrameworkElement element)
        { 
            // Walk up the tree to find the DataGridRow that contains the element 
            object parent = element;
            DataGridRow row = parent as DataGridRow; 
            while ((parent != null) && (row == null))
            {
                parent = element.Parent; 
                row = parent as DataGridRow;
            }
            if (row == null || row.Index == -1) 
            { 
                throw DataGridError.DataGrid.InvalidRowElement("element");
            } 
            else
            {
                return GetRowDetailsVisibility(row.Index); 
            }
        }
 
        // 

 


 


 
 

 


 


 
 

 


        #endregion Public Methods 

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
            if (this.SelectedItemsInternal.Count > 0)
            { 
                this._noSelectionChangeCount++;
                try
                { 
                    // Individually deselecting displayed rows to view potential transitions
                    for (int rowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                         rowIndex > -1 && rowIndex <= this.DisplayData.LastDisplayedScrollingRow; 
                         rowIndex++) 
                    {
                        if (this.SelectedItemsInternal.Contains(rowIndex)) 
                        {
                            SelectRow(rowIndex, false);
                        } 
                    }
                    this.SelectedItemsInternal.ClearRows();
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
                if (this.SelectedItemsInternal.Count > 0) 
                {
                    // Individually deselecting displayed rows to view potential transitions 
                    for (int rowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                         rowIndex > -1 && rowIndex <= this.DisplayData.LastDisplayedScrollingRow;
                         rowIndex++) 
                    {
                        if (rowIndexException != rowIndex && this.SelectedItemsInternal.Contains(rowIndex))
                        { 
                            SelectRow(rowIndex, false); 
                            this._selectionChanged = true;
                        } 
                    }
                    exceptionAlreadySelected = this.SelectedItemsInternal.Contains(rowIndexException);
                    int selectedCount = this.SelectedItemsInternal.Count; 
                    if (selectedCount > 0)
                    {
                        if (selectedCount > 1) 
                        { 
                            this._selectionChanged = true;
                        } 
                        else
                        {
                            System.Collections.IEnumerator selectedRowIndexesEnumerator = this.SelectedItemsInternal.GetIndexesEnumerator(); 
                            selectedRowIndexesEnumerator.MoveNext();
                            int currentlySelectedRowIndex = (int)selectedRowIndexesEnumerator.Current;
                            if (currentlySelectedRowIndex != rowIndexException) 
                            { 
                                this._selectionChanged = true;
                            } 
                        }
                        this.SelectedItemsInternal.ClearRows();
                    } 
                }
                if (exceptionAlreadySelected)
                { 
                    // Exception row was already selected. It just needs to be marked as selected again. 
                    // No transition involved.
                    this.SelectedItemsInternal.SelectRow(rowIndexException, true /*select*/); 
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
            return GetRowDetailsVisibility(rowIndex, this.RowDetailsVisibility);
        } 

        internal Visibility GetRowDetailsVisibility(int rowIndex, DataGridRowDetailsVisibility gridLevelRowDetailsVisibility)
        { 
            Debug.Assert(rowIndex != -1); 
            if (_showDetailsTable.Contains(rowIndex))
            { 
                // The user explicity set AreDetailsVisible on a row so we should respect that
                return _showDetailsTable.GetValueAt(rowIndex);
            } 
            else
            {
                if (gridLevelRowDetailsVisibility == DataGridRowDetailsVisibility.Visible || 
                    (gridLevelRowDetailsVisibility == DataGridRowDetailsVisibility.VisibleWhenSelected && 
                     this.SelectedItemsInternal.Contains(rowIndex)))
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
        /// <param name="index">backend data item</param>
        /// <returns>null if the datasource is null or provided item in not in the source. The associated UI row otherwise</returns>
        internal DataGridRow GetRowFromItem(object dataItem) 
        { 
            int rowIndex = this.DataConnection.IndexOf(dataItem);
            if (rowIndex == -1) 
            {
                return null;
            } 
            if (IsRowDisplayed(rowIndex))
            {
                return GetDisplayedRow(rowIndex); 
            } 
            return GenerateRow(rowIndex);
        } 

        internal bool GetRowSelection(int rowIndex)
        { 
            Debug.Assert(rowIndex != -1);
            return this.SelectedItemsInternal.Contains(rowIndex);
        } 
 
        internal void InsertRows(int rowIndex, int rowCount)
        { 
            Debug.Assert(this.ColumnsItemsInternal.Count > 0);
            Debug.Assert(rowIndex >= 0 && rowIndex <= this._rowCount);
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
                this._rowCount++; 
                OnInsertedRow_Phase1(rowIndex, false /*generateRow*/, 1); 
                OnInsertedRow_Phase2(rowIndex, newCurrentCellCoordinates, true /*lastInsertion*/,
                    this._vScrollBar == null || this._vScrollBar.Visibility == Visibility.Visible /*updateVerticalScrollBarOnly*/); 
                OnRowsChanged(true /*rowsGrew*/);
            }
        } 

        internal bool IsRowDisplayed(int rowIndex)
        { 
            return rowIndex >= this._firstDisplayedRowIndex && rowIndex <= this._lastDisplayedRowIndex; 
        }
 
        // detailsElement is the FrameworkElement created by the DetailsTemplate
        internal void OnCleaningRowDetails(FrameworkElement detailsElement)
        { 
            OnCleaningRowDetails(new DataGridRowDetailsEventArgs(detailsElement));
        }
 
        // detailsElement is the FrameworkElement created by the DetailsTemplate 
        internal void OnPreparingRowDetails(FrameworkElement detailsElement)
        { 
            OnPreparingRowDetails(new DataGridRowDetailsEventArgs(detailsElement));
        }
 
        internal void OnRowDetailsChanged()
        {
            PerformLayout(); 
        } 

        internal void RefreshRows() 
        {
            ClearRows();
 
            if (this.DataConnection != null && this.ColumnsItemsInternal.Count > 0)
            {
                AddRows(0, this.DataConnection.Count); 
            } 
        }
 
        internal void RemoveRowAt(int rowIndex)
        {
            Debug.Assert(rowIndex >= 0 && rowIndex < this._rowCount); 
            //

            DataGridCellCoordinates newCurrentCellCoordinates = OnRemovingRow(rowIndex); 
            this._rowCount--; 
            OnRemovedRow(rowIndex, newCurrentCellCoordinates);
            OnRowsChanged(false /*rowsGrew*/); 
        }

        internal void SetRowDetailsVisibility(int rowIndex, Visibility visibility) 
        {
            Debug.Assert(rowIndex >= 0);//
            _showDetailsTable.AddValue(rowIndex, visibility); 
        } 

        internal void SetRowSelection(int rowIndex, bool isSelected, bool setAnchorRowIndex) 
        {
            Debug.Assert(!(!isSelected && setAnchorRowIndex));
            Debug.Assert(rowIndex >= 0 && rowIndex < this._rowCount); 
            this._noSelectionChangeCount++;
            try
            { 
                if (this.SelectionMode == DataGridSelectionMode.SingleFullRow && isSelected) 
                {
                    Debug.Assert(this.SelectedItemsInternal.Count <= 1); 
                    if (this.SelectedItemsInternal.Count > 0)
                    {
                        System.Collections.IEnumerator selectedRowIndexesEnumerator = this.SelectedItemsInternal.GetIndexesEnumerator(); 
                        selectedRowIndexesEnumerator.MoveNext();
                        int currentlySelectedRowIndex = (int)selectedRowIndexesEnumerator.Current;
                        if (currentlySelectedRowIndex != rowIndex) 
                        { 
                            SelectRow(currentlySelectedRowIndex, false);
                            this._selectionChanged = true; 
                        }
                    }
                } 
                if (this.SelectedItemsInternal.Contains(rowIndex) != isSelected)
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
            Debug.Assert(startRowIndex >= 0 && startRowIndex < this._rowCount);
            Debug.Assert(endRowIndex >= 0 && endRowIndex < this._rowCount); 
            Debug.Assert(startRowIndex <= endRowIndex); 
            //
 
            this._noSelectionChangeCount++;
            try
            { 
                //

 
 

 

                if (/*isSelected &&*/ !this.SelectedItemsInternal.ContainsAll(startRowIndex, endRowIndex))
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
            Debug.Assert(!dataGridRow.HasHeaderCell || dataGridRow.HeaderCell.OwningRow == dataGridRow);
            Debug.Assert(rowIndex == this._rowCount); 

            OnAddedRow_Phase1(rowIndex, dataGridRow);
            this._rowCount++; 
            OnAddedRow_Phase2(false /*updateVerticalScrollBarOnly*/);
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
                this._rowCount += (rowCount - addedRows); 
                OnAddedRow_Phase2(this._vScrollBar == null || this._vScrollBar.Visibility == Visibility.Visible /*updateVerticalScrollBarOnly*/);
                OnRowsChanged(true /*rowsGrew*/);
            } 
        } 

        private void ApplyDisplayedRowsState(int startRowIndex, int endRowIndex) 
        {
            int firstRow = Math.Max(this._firstDisplayedRowIndex, startRowIndex);
            int lastRow = Math.Min(this._lastDisplayedRowIndex, endRowIndex); 

            if (firstRow >= 0)
            { 
                Debug.Assert(lastRow >= firstRow); 
                for (int rowIndex = firstRow; rowIndex <= lastRow; rowIndex++)
                { 
                    Debug.Assert(IsRowDisplayed(rowIndex));
                    GetDisplayedRow(rowIndex).ApplyBackgroundBrush(true /*animate*/);
                } 
            }
        }
 
        private void ClearRows() 
        {
            if (this._rowCount > 0) 
            {
                SetCurrentCellCore(-1, -1, false /*commitEdit*/);
                ClearRowSelection(true /*resetAnchorRowIndex*/); 
                ResetDisplayedRows();
                this._showDetailsTable.Clear();
                this._rowCount = 0; 
                ComputeDisplayedRows(this.CellsHeight); 
                this._verticalOffset = this._negVerticalOffset = 0;
                this._verticalScrollChangesIgnored++; 
                try
                {
                    if (this._vScrollBar != null && this._vScrollBar.Value != 0) 
                    {
                        this._vScrollBar.Value = 0;
                    } 
                } 
                finally
                { 
                    this._verticalScrollChangesIgnored--;
                }
                // 

                PerformLayout();
            } 
        } 

        private void ComputeDisplayedRows(double displayHeight) 
        {
            int firstDisplayedScrollingRow = this.DisplayData.FirstDisplayedScrollingRow;
            int lastDisplayedScrollingRow = -1; 
            double cy = -this._negVerticalOffset;
            int visibleScrollingRows = 0;
 
            if (DoubleUtil.LessThanOrClose(displayHeight, 0) || this._rowCount == 0 || this.ColumnsItemsInternal.Count == 0) 
            {
                this.DisplayData.NumDisplayedScrollingRows = this.DisplayData.NumTotallyDisplayedScrollingRows = 0; 
                this.DisplayData.FirstDisplayedScrollingRow = this.DisplayData.LastDisplayedScrollingRow = -1;
                return;
            } 

            if (firstDisplayedScrollingRow == -1)
            { 
                firstDisplayedScrollingRow = 0; 
            }
 
            for (int i = firstDisplayedScrollingRow; i < this._rowCount; i++)
            {
                cy += GetEdgedExactRowHeight(i); 
                visibleScrollingRows++;
                lastDisplayedScrollingRow = i;
                if (DoubleUtil.GreaterThanOrClose(cy,  displayHeight)) 
                { 
                    break;
                } 
            }

            if (DoubleUtil.LessThan(cy, displayHeight)) 
            {
                for (int i = firstDisplayedScrollingRow - 1; i >= 0; i--)
                { 
                    double height = GetEdgedExactRowHeight(i); 
                    if (cy + height > displayHeight)
                    { 
                        break;
                    }
                    cy += height; 
                    firstDisplayedScrollingRow = i;
                    visibleScrollingRows++;
                } 
            } 

            this.DisplayData.NumDisplayedScrollingRows = visibleScrollingRows; 
            if (DoubleUtil.GreaterThan(cy, displayHeight) || (DoubleUtil.AreClose(cy, displayHeight) && DoubleUtil.GreaterThan(this._negVerticalOffset, 0)))
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

            Debug.Assert(lastDisplayedScrollingRow < this._rowCount, "lastDisplayedScrollingRow larger than number of rows"); 
 
            this.DisplayData.FirstDisplayedScrollingRow = firstDisplayedScrollingRow;
            this.DisplayData.LastDisplayedScrollingRow = lastDisplayedScrollingRow; 

            Debug.Assert(this.DisplayData.NumDisplayedScrollingRows >= 0, "the number of visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0, "the number of totally visible scrolling rows can't be negative"); 
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow < this._rowCount, "firstDisplayedScrollingRow larger than number of rows");
        }
 
        // Similar to ComputeDisplayedRows except that it starts with the LastDisplayedScrollingRow 
        // and computes the FirstDisplayScrollingRow instead of doing it the other way around.  We use this
        // when scrolling down to a full row 
        private void ComputeDisplayedRowsFromBottom()
        {
            int lastDisplayedScrollingRow = this.DisplayData.LastDisplayedScrollingRow; 
            int firstDisplayedScrollingRow = -1;
            double displayHeight = this.CellsHeight;
            double cy = 0; 
            int visibleScrollingRows = 0; 

            if (DoubleUtil.LessThanOrClose(displayHeight, 0) || this._rowCount == 0 || this.ColumnsItemsInternal.Count == 0) 
            {
                this.DisplayData.NumDisplayedScrollingRows = this.DisplayData.NumTotallyDisplayedScrollingRows = 0;
                this.DisplayData.FirstDisplayedScrollingRow = this.DisplayData.LastDisplayedScrollingRow = -1; 
                return;
            }
 
            if (lastDisplayedScrollingRow == -1) 
            {
                lastDisplayedScrollingRow = 0; 
            }

            for (int i = lastDisplayedScrollingRow; i >= 0; i--) 
            {
                cy += GetEdgedExactRowHeight(i);
                visibleScrollingRows++; 
                firstDisplayedScrollingRow = i; 
                if (DoubleUtil.GreaterThanOrClose(cy, displayHeight))
                { 
                    break;
                }
            } 

            this.DisplayData.NumTotallyDisplayedScrollingRows = cy > displayHeight ? visibleScrollingRows - 1 : visibleScrollingRows;
            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0); 
 
            this._negVerticalOffset = Math.Max(0, cy - displayHeight);
 
            this.DisplayData.NumDisplayedScrollingRows = visibleScrollingRows;

            Debug.Assert(lastDisplayedScrollingRow < this._rowCount, "lastDisplayedScrollingRow larger than number of rows"); 

            this.DisplayData.FirstDisplayedScrollingRow = firstDisplayedScrollingRow;
            this.DisplayData.LastDisplayedScrollingRow = lastDisplayedScrollingRow; 
 
            Debug.Assert(this.DisplayData.NumDisplayedScrollingRows >= 0, "the number of visible scrolling rows can't be negative");
            Debug.Assert(this.DisplayData.NumTotallyDisplayedScrollingRows >= 0, "the number of totally visible scrolling rows can't be negative"); 
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow < this._rowCount, "firstDisplayedScrollingRow larger than number of rows");
        }
 
        /*

 
 

 


 


 
 

 


 


 
 

 


 


 
 

 

*/
 
        // Corrects row indexes for displayed rows and creates new rows as necessary.  Returns
        // true if a new row was created that has a height different from DataGrid.RowHeight.
        // This happens when the user changes the height of a Row in the prepare row event 
        private bool CorrectDisplayedRowsIndexes() 
        {
            // 
            if (this.DisplayData.FirstDisplayedScrollingRow > this._lastDisplayedRowIndex ||
                this.DisplayData.LastDisplayedScrollingRow < this._firstDisplayedRowIndex)
            { 
                //

                // Scrolling was so fast that we need to delete all displayed row 
                while (this._firstDisplayedRowIndex > -1) 
                {
                    RemoveDisplayedRow(this._firstDisplayedRowIndex); 
                }
            }
 
            bool uniqueRowHeight = false;
            while (this.DisplayData.FirstDisplayedScrollingRow != this._firstDisplayedRowIndex)
            { 
                if (this.DisplayData.FirstDisplayedScrollingRow == -1) 
                {
                    // 

                    // Need to delete first displayed row
                    Debug.Assert(this._firstDisplayedRowIndex >= 0); 
                    RemoveDisplayedRow(this._firstDisplayedRowIndex);
                }
                else if (this._firstDisplayedRowIndex == -1) 
                { 
                    //
 
                    // Need to add first row
                    Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= 0);
                    DataGridRow dataGridRow = GenerateRow(this.DisplayData.FirstDisplayedScrollingRow); 
                    uniqueRowHeight |= dataGridRow.Height != this.RowHeight;
                    Debug.Assert(dataGridRow != null);
                    InsertDisplayedRow(this.DisplayData.FirstDisplayedScrollingRow, dataGridRow); 
                    Debug.Assert(this._firstDisplayedRowIndex == this.DisplayData.FirstDisplayedScrollingRow); 
                    Debug.Assert(this._lastDisplayedRowIndex == this.DisplayData.FirstDisplayedScrollingRow);
                } 
                else if (this.DisplayData.FirstDisplayedScrollingRow < this._firstDisplayedRowIndex)
                {
                    // 

                    // Need to add rows on top of currently first displayed row
                    DataGridRow dataGridRow = GenerateRow(this._firstDisplayedRowIndex - 1); 
                    uniqueRowHeight |= dataGridRow.Height != this.RowHeight; 
                    Debug.Assert(dataGridRow != null);
                    InsertDisplayedRow(this._firstDisplayedRowIndex - 1, dataGridRow); 
                }
                else
                { 
                    //
                    Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow > this._firstDisplayedRowIndex);
                    // Need to remove rows on top of this.DisplayData.FirstDisplayedScrollingRow 
                    RemoveDisplayedRow(this._firstDisplayedRowIndex); 
                }
            } 

            while (this.DisplayData.LastDisplayedScrollingRow != this._lastDisplayedRowIndex)
            { 
                if (this.DisplayData.LastDisplayedScrollingRow < this._lastDisplayedRowIndex)
                {
                    // 
 
                    // Need to remove rows below this.DisplayData.LastDisplayedScrollingRow
                    RemoveDisplayedRow(this._lastDisplayedRowIndex); 
                }
                else if (this.DisplayData.LastDisplayedScrollingRow > this._lastDisplayedRowIndex)
                { 
                    //

                    // Need to add rows below currently last displayed row 
                    DataGridRow dataGridRow = GenerateRow(this._lastDisplayedRowIndex + 1); 
                    uniqueRowHeight |= dataGridRow.Height != this.RowHeight;
                    Debug.Assert(dataGridRow != null); 
                    InsertDisplayedRow(this._lastDisplayedRowIndex + 1, dataGridRow);
                }
            } 

            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow == this._firstDisplayedRowIndex);
            Debug.Assert(this.DisplayData.LastDisplayedScrollingRow == this._lastDisplayedRowIndex); 
 
            return uniqueRowHeight;
        } 

        /// <summary>
        /// Adjusts the index of all displayed, prepared & edited rows after a row was deleted. 
        /// Removes the deleted row from the list of prepared rows if present.
        /// </summary>
        private void CorrectRowIndexesAfterDeletion(int rowIndexDeleted) 
        { 
            Debug.Assert(rowIndexDeleted >= 0);
 
            // Take care of the non-displayed prepared rows
            for (int index = 0; index < this._preparedRows.Count; )
            { 
                DataGridRow dataGridRow = this._preparedRows[index];
                if (IsRowDisplayed(dataGridRow.Index))
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
                        this._preparedRows.RemoveAt(index); 
                    }
                    else
                    { 
                        index++;
                    }
                } 
            } 

            // Take care of the non-displayed edited row 
            if (this._editingRow != null &&
                !IsRowDisplayed(this._editingRow.Index) &&
                this._editingRow.Index > rowIndexDeleted) 
            {
                this._editingRow.Index--;
            } 
 
            // Take care of the displayed rows
            if (this._cells != null) 
            {
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++)
                { 
                    DataGridRow dataGridRow = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex);
                    if (dataGridRow.Index > rowIndexDeleted)
                    { 
                        dataGridRow.Index--; 
                        dataGridRow.EnsureBackground();
                    } 
                }
            }
        } 

        /// <summary>
        /// Adjusts the index of all displayed, prepared & edited rows after rows were deleted. 
        /// </summary> 
        private void CorrectRowIndexesAfterInsertion(int rowIndexInserted, int insertionCount)
        { 
            Debug.Assert(rowIndexInserted >= 0);
            Debug.Assert(insertionCount > 0);
 
            // Take care of the non-displayed prepared rows
            foreach (DataGridRow dataGridRow in this._preparedRows)
            { 
                if (!IsRowDisplayed(dataGridRow.Index) && dataGridRow.Index >= rowIndexInserted) 
                {
                    dataGridRow.Index += insertionCount; 
                }
            }
 
            // Take care of the non-displayed edited row
            if (this._editingRow != null &&
                !IsRowDisplayed(this._editingRow.Index) && 
                this._editingRow.Index >= rowIndexInserted) 
            {
                this._editingRow.Index += insertionCount; 
            }

            // Take care of the displayed rows 
            if (this._cells != null)
            {
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++) 
                { 
                    DataGridRow dataGridRow = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex);
                    if (dataGridRow.Index >= rowIndexInserted) 
                    {
                        dataGridRow.Index += insertionCount;
                        dataGridRow.EnsureBackground(); 
                    }
                }
            } 
        } 

        private void EnsureRowDetailsVisibility(DataGridRow row, bool animate) 
        {
            // Show or hide RowDetails based on DataGrid settings
            row.SetAreRowDetailsVisibleInternal(GetRowDetailsVisibility(row.Index) == Visibility.Visible, animate); 
        }

        private DataGridRow GenerateRow(int rowIndex) 
        { 
            return GenerateRow(rowIndex, false /*prefetch*/);
        } 

        /// <summary>
        /// Returns a row for the provided index. The row gets first prepared through the PreparingRow event. 
        /// </summary>
        private DataGridRow GenerateRow(int rowIndex, bool prefetch)
        { 
            Debug.Assert(rowIndex > -1); 
            DataGridRow dataGridRow = GetGeneratedRow(rowIndex);
            if (dataGridRow == null) 
            {
                if (this._recyclableRows.Count > 0)
                { 
                    //
                    dataGridRow = this._recyclableRows[0];
                    this._recyclableRows.RemoveAt(0); 
 
                    //
                    foreach (DataGridColumnBase column in this.ColumnsInternal) 
                    {
                        if (!(column is DataGridTextBoxColumn))
                        { 
                            PopulateCellContent(true /* forceTemplating */, false /* isCellEdited */, column, dataGridRow, dataGridRow.Cells[column.Index]);
                        }
                    } 
                } 
                else
                { 
                    //
                    dataGridRow = new DataGridRow();
                } 
                dataGridRow.Index = rowIndex;
                dataGridRow.OwningGrid = this;
                dataGridRow.DataContext = this.DataConnection.GetDataItem(rowIndex); 
                CompleteCellsCollection(dataGridRow); 

                OnPreparingRow(new DataGridRowEventArgs(dataGridRow)); 
                if (prefetch)
                {
                    this._prefetchedRows.Add(dataGridRow); 
                }
            }
            return dataGridRow; 
        } 

        private DataGridRow GetDisplayedRow(int rowIndex) 
        {
            Debug.Assert(rowIndex >= this._firstDisplayedRowIndex);
            Debug.Assert(rowIndex <= this._lastDisplayedRowIndex); 

            if (this._cells != null)
            { 
                int childIndex = rowIndex - this._firstDisplayedRowIndex; 
                if (this._editingRowLocation == DataGridEditingRowLocation.Top)
                { 
                    childIndex++;
                }
                return this._cells.Children[childIndex] as DataGridRow; 
            }
            return null;
        } 
 
        /// <summary>
        /// Returns the exact row height, whether it is currently displayed or not. 
        /// The row is generated in case it is not already displayed.
        /// The horizontal gridlines thickness are added.
        /// </summary> 
        private double GetEdgedExactRowHeight(int rowIndex)
        {
            Debug.Assert((rowIndex >= 0 /**/) && rowIndex < this._rowCount); 
            double rowHeight; 
            if (IsRowDisplayed(rowIndex))
            { 
                rowHeight = GetDisplayedRow(rowIndex).DisplayHeight;
            }
            else 
            {
                rowHeight = GenerateRow(rowIndex, true /*prefetch*/).DisplayHeight;
            } 
            if (this.AreRowBottomGridlinesRequired) 
            {
                rowHeight += DataGrid.HorizontalGridlinesThickness; 
            }
            return rowHeight;
        } 

        //
 
 

 


 

        //    double rowsHeight = 0;
        //    for (int rowIndex = fromRowIndex; rowIndex <= toRowIndex; rowIndex++) 
        //    { 
        //        rowsHeight += GetEdgedExactRowHeight(rowIndex);
        //    } 
        //    return rowsHeight;
        //}
 
        /// <summary>
        /// If the provided row index is displayed, returns the exact row height.
        /// If the row is not displayed, returns the default row height. 
        /// In both cases, the horizontal gridlines thickness are added. 
        /// </summary>
        private double GetEdgedRowHeight(int rowIndex) 
        {
            Debug.Assert((rowIndex >= 0 /**/) && rowIndex < this._rowCount);
            double rowHeight; 
            if (IsRowDisplayed(rowIndex))
            {
                rowHeight = GetDisplayedRow(rowIndex).DisplayHeight; 
            } 
            else
            { 
                DataGridRow dataGridRow = GetGeneratedRow(rowIndex);
                if (dataGridRow == null)
                { 
                    //

 
                    rowHeight = this.RowHeight; 
                }
                else 
                {
                    rowHeight = dataGridRow.DisplayHeight;
                } 
            }
            if (this.AreRowBottomGridlinesRequired)
            { 
                rowHeight += DataGrid.HorizontalGridlinesThickness; 
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
        /// in either the prepared rows, prefetched rows, or editing row. 
        /// The displayed rows are *not* searched. Returns null if the row does not belong to those 3 categories.
        /// </summary>
        private DataGridRow GetGeneratedRow(int rowIndex) 
        { 
            // Check the list of rows being prepared via the PreparingRow event.
            DataGridRow dataGridRow = GetPreparedRow(rowIndex); 
            if (dataGridRow != null)
            {
                return dataGridRow; 
            }

            // Check the list of prefetched rows. 
            dataGridRow = GetPrefetchedRow(rowIndex); 
            if (dataGridRow != null)
            { 
                return dataGridRow;
            }
 
            // Check the potential editing row.
            if (this._editingRow != null && rowIndex == this.EditingRowIndex)
            { 
                return this._editingRow; 
            }
 
            return null;
        }
 
        private int GetNextRow(int rowIndex)
        {
            int result = rowIndex + 1; 
            return (result < this._rowCount) ? result : -1; 
        }
 
        private DataGridRow GetPrefetchedRow(int rowIndex)
        {
            foreach (DataGridRow dataGridRow in this._prefetchedRows) 
            {
                if (dataGridRow.Index == rowIndex)
                { 
                    return dataGridRow; 
                }
            } 
            return null;
        }
 
        private DataGridRow GetPreparedRow(int rowIndex)
        {
            foreach (DataGridRow dataGridRow in this._preparedRows) 
            { 
                if (dataGridRow.Index == rowIndex)
                { 
                    return dataGridRow;
                }
            } 
            return null;
        }
 
        private static int GetPreviousRow(int rowIndex) 
        {
            int result = rowIndex - 1; 
            return (result >= 0) ? result : -1;
        }
 
        /*

 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


*/ 
 
        private void InsertDisplayedRow(int rowIndex, DataGridRow dataGridRow)
        { 
            Debug.Assert(dataGridRow != null);

            if (dataGridRow == this._editingRow) 
            {
                this._editingRowLocation = DataGridEditingRowLocation.Inline;
                this._editingRow.Visibility = Visibility.Visible; 
                if (this.AreRowHeadersVisible && this._rowHeaders != null && this._editingRow.HasHeaderCell) 
                {
                    this._editingRow.HeaderCell.Visibility = Visibility.Visible; 
                }
                Debug.Assert(dataGridRow.Index == rowIndex);
            } 
            else
            {
                dataGridRow.Index = rowIndex; 
 
                int childIndex;
                if (this._firstDisplayedRowIndex == -1) 
                {
                    childIndex = 0;
                } 
                else if (rowIndex > this._firstDisplayedRowIndex)
                {
                    childIndex = rowIndex - this._firstDisplayedRowIndex; 
                } 
                else
                { 
                    Debug.Assert(rowIndex == this._firstDisplayedRowIndex - 1 || rowIndex == this._firstDisplayedRowIndex);
                    childIndex = 0;
                } 
                if (this._editingRowLocation == DataGridEditingRowLocation.Top)
                {
                    childIndex++; 
                } 

                // Add potential row header cell 
                InsertDisplayedRowHeader(dataGridRow, childIndex);

                if (this._cells != null) 
                {
                    if (this.AreRowBottomGridlinesRequired)
                    { 
                        dataGridRow.EnsureGridline(); 
                    }
                    foreach (DataGridCell dataGridCell in dataGridRow.Cells) 
                    {
                        if (ColumnRequiresRightGridline(this.ColumnsItemsInternal[dataGridCell.ColumnIndex], true /*includeLastRightGridlineWhenPresent*/))
                        { 
                            dataGridCell.EnsureGridline();
                        }
                    } 
                    Debug.Assert(!this._cells.Children.Contains(dataGridRow)); 
                    this._cells.Children.Insert(childIndex, dataGridRow);
                } 
            }

            if (this._firstDisplayedRowIndex == -1) 
            {
                this._firstDisplayedRowIndex = rowIndex;
                Debug.Assert(this._lastDisplayedRowIndex == -1); 
                this._lastDisplayedRowIndex = rowIndex; 
            }
            else if (rowIndex < this._firstDisplayedRowIndex) 
            {
                this._firstDisplayedRowIndex = rowIndex;
            } 
            else
            {
                this._lastDisplayedRowIndex++; 
            } 

            PrepareRowVisualsForDisplay(dataGridRow); 
        }

        private void InsertDisplayedRowHeader(DataGridRow dataGridRow, int childIndex) 
        {
            Debug.Assert(dataGridRow != null);
 
            if (this.AreRowHeadersVisible && this._rowHeaders != null) 
            {
                dataGridRow.HeaderCell.Visibility = Visibility.Visible; 
                Debug.Assert(!this._rowHeaders.Children.Contains(dataGridRow.HeaderCell));
                this._rowHeaders.Children.Insert(childIndex, dataGridRow.HeaderCell);
            } 
        }

        private void InsertRow(int rowIndex) 
        { 
            Debug.Assert(rowIndex >= 0 && rowIndex <= this._rowCount);
 
            DataGridCellCoordinates newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);

            OnInsertingRow(rowIndex, ref newCurrentCellCoordinates, true /*firstInsertion*/, 1 /*insertionCount*/);   // will throw an exception if the insertion is illegal 

            OnInsertedRow_Phase1(rowIndex, true /*generateRow*/, 1);
            this._rowCount++; 
            OnInsertedRow_Phase2(rowIndex, newCurrentCellCoordinates, true /*lastInsertion*/, false /*updateVerticalScrollBarOnly*/); 
            OnRowsChanged(true /*rowsGrew*/);
        } 

        private void OnAddedRow_Phase1(int rowIndex, DataGridRow dataGridRow)
        { 
            Debug.Assert(rowIndex >= 0);
            // Row needs to be potentially added to the displayed rows
            if (RowRequiresDisplay(rowIndex)) 
            { 
                InsertDisplayedRow(rowIndex, dataGridRow);
            } 
        }

        private void OnAddedRow_Phase2(bool updateVerticalScrollBarOnly) 
        {
            if (updateVerticalScrollBarOnly)
            { 
                UpdateVerticalScrollBar(); 
            }
            else 
            {
                PerformLayout();
            } 
        }

        private void OnInsertedRow_Phase1(int rowIndex, bool generateRow, int insertionCount) 
        { 
            Debug.Assert(rowIndex >= 0);
            Debug.Assert(insertionCount > 0); 

            // Fix the OldFirstDisplayedScrollingRow
            this.DisplayData.CorrectRowIndexAfterInsertion(rowIndex, insertionCount); 

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
                Debug.Assert(!dataGridRow.HasHeaderCell || dataGridRow.HeaderCell.OwningRow == dataGridRow);

                dataGridRow.OwningGrid = this; 
                OnAddedRow_Phase1(rowIndex, dataGridRow); 
            }
            else if (rowIndex < this._firstDisplayedRowIndex) 
            {
                this._firstDisplayedRowIndex++;
                this._lastDisplayedRowIndex++; 
            }
        }
 
        private void OnInsertedRow_Phase2(int rowIndex, DataGridCellCoordinates newCurrentCellCoordinates, 
                                          bool lastInsertion, bool updateVerticalScrollBarOnly)
        { 
            Debug.Assert(rowIndex >= 0);

            // Same effect as adding a row 
            OnAddedRow_Phase2(updateVerticalScrollBarOnly);

            // Update current cell if needed 
            if (lastInsertion && newCurrentCellCoordinates.RowIndex != -1) 
            {
                Debug.Assert(this.CurrentColumnIndex == -1); 
                bool success = SetAndSelectCurrentCell(newCurrentCellCoordinates.ColumnIndex,
                                                       newCurrentCellCoordinates.RowIndex,
                                                       this._rowCount == 1 /*forceCurrentCellSelection*/); 
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
            this.SelectedItemsInternal.InsertIndexes(rowIndexInserted, insertionCount);
            // Same for details visibility settings 
            this._showDetailsTable.InsertIndexes(rowIndexInserted, insertionCount);
        }
 
        private void OnRemovedRow(int rowIndexDeleted, DataGridCellCoordinates newCurrentCellCoordinates) 
        {
            // Fix the OldFirstDisplayedScrollingRow 
            this.DisplayData.CorrectRowIndexAfterDeletion(rowIndexDeleted);

            CorrectRowIndexesAfterDeletion(rowIndexDeleted); 

            // Row needs to be potentially removed from the displayed rows
            if (rowIndexDeleted >= this.DisplayData.FirstDisplayedScrollingRow && 
                rowIndexDeleted <= this.DisplayData.LastDisplayedScrollingRow) 
            {
                // Displayed row is removed 
                RemoveDisplayedRow(rowIndexDeleted);
                if (this._firstDisplayedRowIndex > rowIndexDeleted)
                { 
                    this._firstDisplayedRowIndex--;
                    this._lastDisplayedRowIndex--;
                } 
            } 
            else if (rowIndexDeleted < this.DisplayData.FirstDisplayedScrollingRow)
            { 
                // Scrolled off row above first displayed row is removed
                RemoveDisplayedRow(this.DisplayData.FirstDisplayedScrollingRow);
                this._firstDisplayedRowIndex--; 
                this._lastDisplayedRowIndex--;
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

            if (rowIndexDeleted > this.DisplayData.LastDisplayedScrollingRow)
            { 
                UpdateVerticalScrollBar();
            }
            else 
            { 
                PerformLayout();
            } 
        }

        private DataGridCellCoordinates OnRemovingRow(int rowIndexDeleted) 
        {
            // Note that the row needs to be deleted no matter what. The underlying data row was already deleted.
 
            DataGridCellCoordinates newCurrentCellCoordinates; 
            Debug.Assert(rowIndexDeleted >= 0 && rowIndexDeleted < this._rowCount);
 
            this._temporarilyResetCurrentCell = false;
            newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);
 
            // Reset the current cell's address if it's on the deleted row, or after it.
            if (this.CurrentRowIndex != -1 && rowIndexDeleted <= this.CurrentRowIndex)
            { 
                int newCurrentRowIndex; 
                if (rowIndexDeleted == this.CurrentRowIndex)
                { 
                    int rowIndexPrevious = rowIndexDeleted - 1;
                    int rowIndexNext = (rowIndexDeleted == this._rowCount - 1) ? -1 : rowIndexDeleted + 1;
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
            if (this.SelectedItemsInternal.Contains(rowIndexDeleted)) 
            {
                this._selectionChanged = true; 
            }
            this.SelectedItemsInternal.RemoveIndex(rowIndexDeleted);
 
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
        private void PrepareRowVisualsForDisplay(DataGridRow row) 
        {
            // If the row has been recycled, reapply the BackgroundBrush
            if (row.IsRecycled) 
            {
                row.EnsureBackground();
                row.ApplyBackgroundBrush(false /*animate*/); 
                row.ApplyCellsState(false /*animate*/); 

                // 
                row.ReAttachHandlers();

                if (this.AreRowHeadersVisible) 
                {
                    row.HeaderCell.ApplyRowStatus(false /*animate*/);
                } 
            } 
            else if (row == this._editingRow)
            { 
                //

                row.ApplyCellsState(false /*animate*/); 
            }

            // Set the Row's Style if we one's defined at the DataGrid level and the user didn't 
            // set one at the row level 
            EnsureRowStyle(row, null, this.RowStyle);
            if (this.RowHeaderStyle != null && this.AreRowHeadersVisible) 
            {
                EnsureRowHeaderCellStyle(row, null, this.RowHeaderStyle);
            } 

            // Check to see if the row contains the CurrentCell, apply its state.
            if (this.CurrentColumnIndex != -1 && 
                this.CurrentRowIndex != -1 && 
                row.Index == this.CurrentRowIndex)
            { 
                row.Cells[this.CurrentColumnIndex].ApplyCellState(false /*animate*/);
            }
 
            // Show or hide RowDetails based on DataGrid settings
            EnsureRowDetailsVisibility(row, false);
        } 
 
        /// <summary>
        /// Detaches the provided row from the grid and adds it to the recyclable list. 
        /// </summary>
        private void ReleaseRow(DataGridRow dataGridRow)
        { 
            Debug.Assert(dataGridRow != null);

            if (this._prefetchedRows.Contains(dataGridRow) || 
                this._preparedRows.Contains(dataGridRow)) 
            {
                // 
                return; // The row is still referenced, we can't release it.
            }
 
            // Not trying to recycle the current row to avoid incorrect display of
            // current cell cues.
            bool recycleRow = this.CurrentRowIndex != dataGridRow.Index; 
 
#if DEBUG
            // 


 

#endif
 
            // Don't recycle if the row has a custom Style set 
            recycleRow &= (dataGridRow.Style == null || dataGridRow.Style == this.RowStyle);
 
#if DEBUG
            //
 


 
 

 

#endif
 
            if (recycleRow)
            {
                DataGridRowCancelEventArgs dataGridRowCancelEventArgs = new DataGridRowCancelEventArgs(dataGridRow); 
                OnCleaningRow(dataGridRowCancelEventArgs); 
                if (!dataGridRowCancelEventArgs.Cancel)
                { 
                    dataGridRow.Recycle();

                    // Add this row to the recyclable list of rows for future use. 
                    Debug.Assert(!this._recyclableRows.Contains(dataGridRow));
                    this._recyclableRows.Add(dataGridRow);
                } 
            } 

            dataGridRow.Index = -1; 
            dataGridRow.DataContext = null;
            dataGridRow.OwningGrid = null;
        } 

        private void RemoveDisplayedRow(int rowIndexDeleted)
        { 
            Debug.Assert(rowIndexDeleted >= this._firstDisplayedRowIndex && 
                         rowIndexDeleted <= this._lastDisplayedRowIndex);
            if (this._cells != null) 
            {
                DataGridRow dataGridRow = GetDisplayedRow(rowIndexDeleted);
                Debug.Assert(dataGridRow != null); 

                if (dataGridRow == this._editingRow)
                { 
                    if (rowIndexDeleted == this._firstDisplayedRowIndex) 
                    {
                        this._editingRowLocation = DataGridEditingRowLocation.Top; 
                    }
                    else
                    { 
                        Debug.Assert(rowIndexDeleted == this._lastDisplayedRowIndex);
                        this._editingRowLocation = DataGridEditingRowLocation.Bottom;
                    } 
                    this._editingRow.Visibility = Visibility.Collapsed; 
                    if (this.AreRowHeadersVisible && this._rowHeaders != null && this._editingRow.HasHeaderCell)
                    { 
                        this._editingRow.HeaderCell.Visibility = Visibility.Collapsed;
                    }
                    if (this._editingRow.HasBottomGridline) 
                    {
                        this._editingRow.BottomGridline.Visibility = Visibility.Collapsed;
                    } 
                } 
                else
                { 
                    int childIndex = rowIndexDeleted - this._firstDisplayedRowIndex;
                    if (this._editingRowLocation == DataGridEditingRowLocation.Top)
                    { 
                        childIndex++;
                    }
                    RemoveRow(dataGridRow, childIndex); 
                } 
                if (rowIndexDeleted == this._firstDisplayedRowIndex)
                { 
                    this._firstDisplayedRowIndex++;
                }
                else 
                {
                    this._lastDisplayedRowIndex--;
                } 
                if (this._lastDisplayedRowIndex < this._firstDisplayedRowIndex) 
                {
                    this._firstDisplayedRowIndex = this._lastDisplayedRowIndex = -1; 
                }
            }
        } 

        private void RemoveRow(DataGridRow dataGridRow, int childIndex)
        { 
            Debug.Assert(this._cells != null); 
            if (this.AreRowHeadersVisible && this._rowHeaders != null && dataGridRow.HasHeaderCell)
            { 
                RemoveDisplayedRowHeader(childIndex);
            }
            RemoveDisplayedHorizontalGridlines(dataGridRow); 
            RemoveDisplayedVerticalGridlines(dataGridRow);
            RemoveDisplayedRowAt(childIndex);
            ReleaseRow(dataGridRow); 
        } 

        private void ResetDisplayedRows() 
        {
            if (this._cells != null)
            { 
                Debug.Assert(this._editingRowLocation == DataGridEditingRowLocation.Inline);
                Debug.Assert(this._editingRow == null);
                SuspendLayout(); 
                while (this.DisplayedRowCount > 0) 
                {
                    DataGridRow dataGridRow = this._cells.Children[0] as DataGridRow; 
                    Debug.Assert(dataGridRow != null);
                    if (this.AreRowHeadersVisible && this._rowHeaders != null && dataGridRow.HasHeaderCell)
                    { 
                        RemoveDisplayedRowHeader(0);
                    }
                    RemoveDisplayedHorizontalGridlines(dataGridRow); 
                    RemoveDisplayedVerticalGridlines(dataGridRow); 
                    RemoveDisplayedRowAt(0);
                    ReleaseRow(dataGridRow); 
                    this._firstDisplayedRowIndex++;
                }
                this._firstDisplayedRowIndex = this._lastDisplayedRowIndex = -1; 
                ResumeLayout(false);
            }
        } 
 
        /// <summary>
        /// Determines whether the row at the provided index must be displayed or not. 
        /// </summary>
        private bool RowRequiresDisplay(int rowIndex)
        { 
            Debug.Assert(rowIndex >= 0);
            //
 
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
                // Additional row is the first displayed row unless layout has been suspended
                Debug.Assert((this._cells == null || this._cells.Children.Count == 0) || _layoutSuspended != 0); 
                Debug.Assert(rowIndex == 0 || _layoutSuspended != 0);
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
                Debug.Assert(this.AvailableRowRoom <= 0);
            } 
            return false; 
        }
 
        private bool ScrollRowIntoView(int rowIndex)
        {
            Debug.Assert(!IsRowOutOfBounds(rowIndex)); 
            Debug.Assert(this._editingRow == null || this.EditingRowIndex == rowIndex);

            // 
 

 


 
            if (this.DisplayData.FirstDisplayedScrollingRow < rowIndex && this.DisplayData.LastDisplayedScrollingRow > rowIndex)
            {
                // The row is already displayed in its entirety 
                return true; 
            }
            else if (this.DisplayData.FirstDisplayedScrollingRow == rowIndex && rowIndex != -1) 
            {
                if (!DoubleUtil.IsZero(this._negVerticalOffset))
                { 
                    // First displayed row is partially scrolled of. Let's scroll it so that this._negVerticalOffset becomes 0.
                    // UpdateVerticalOffset is a helper for calling ScrollRowsByHeight which also calls PerformLayout
                    ScrollRowsByHeight(-this._negVerticalOffset, this._verticalOffset - this._negVerticalOffset); 
                    CorrectDisplayedRowsIndexes(); 
                    PerformLayout();
                } 
                return true;
            }
 
            double deltaY = 0;
            int firstFullRow;
            if (this.DisplayData.FirstDisplayedScrollingRow > rowIndex) 
            { 
                // Scroll up to the new row so it becomes the first displayed row
                firstFullRow = this.DisplayData.FirstDisplayedScrollingRow - 1; 
                if (DoubleUtil.GreaterThan(this._negVerticalOffset, 0))
                {
                    deltaY = -this._negVerticalOffset; 
                }
                deltaY -= GetEdgedRowsHeight(rowIndex, firstFullRow);
                this.DisplayData.FirstDisplayedScrollingRow = rowIndex; 
                this._negVerticalOffset = 0; 
                ComputeDisplayedRows(this.CellsHeight);
            } 
            else if (this.DisplayData.LastDisplayedScrollingRow <= rowIndex)
            {
                // Scroll down to the new row so it's entirely displayed.  If the height of the row 
                // is greater than the height of the DataGrid, then show the top of the row at the top
                // of the grid
                firstFullRow = this.DisplayData.LastDisplayedScrollingRow; 
                // Figure out how much of the last row is cut off 
                double rowHeight = GetEdgedExactRowHeight(this.DisplayData.LastDisplayedScrollingRow);
                DataGridRow row = GetDisplayedRow(this.DisplayData.LastDisplayedScrollingRow); 
                Debug.Assert(row != null);
                double availableHeight = this.CellsHeight - ((double)row.GetValue(Canvas.TopProperty));
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
                if (DoubleUtil.GreaterThanOrClose(GetEdgedExactRowHeight(rowIndex), this.CellsHeight))
                { 
                    // The entire row won't fit in the DataGrid so we start showing it from the top
                    this.DisplayData.FirstDisplayedScrollingRow = rowIndex;
                    this._negVerticalOffset = 0; 
                    ComputeDisplayedRows(this.CellsHeight);
                }
                else 
                { 
                    this.DisplayData.LastDisplayedScrollingRow = rowIndex;
                    ComputeDisplayedRowsFromBottom(); 
                }
            }
 
            this._verticalOffset += deltaY;
            if (this._verticalOffset < 0 || this.DisplayData.FirstDisplayedScrollingRow == 0)
            { 
                // We scrolled too far because a row's height was larger than its approximation 
                this._verticalOffset = this._negVerticalOffset;
            } 

            Debug.Assert(DoubleUtil.LessThanOrClose(this._negVerticalOffset, this._verticalOffset));
            this._verticalScrollChangesIgnored++; 
            try
            {
                if (this._vScrollBar != null && !DoubleUtil.AreClose(this._vScrollBar.Value, this._verticalOffset)) 
                { 
                    this._vScrollBar.Value = this._verticalOffset;
                } 
            }
            finally
            { 
                this._verticalScrollChangesIgnored--;
            }
 
            CorrectDisplayedRowsIndexes(); 
            PerformLayout();
 
            return true;
        }
 
        //

 
 

        //   
        //    
        //    
        //    
        //    
        //    
        // 
        // 
        // 
 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        // 
        //        
        //        
        //        
        //        
        //        
        //        
        //        
        //         
        //    
        //    
        //     
        //    
        //    
        //    
        //    
        //    
        //    
        //            
        //        
        //        
        //        
        //        
        //        
        //        
        //        
        //     

        //    
        //     
        //       
        //       
        //         
        //         
        //        
        //    

        //    
        // 
 
        // 
        // 

        private void ScrollRowsByHeight(double height, double newVerticalOffset)
        { 
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= 0);
            Debug.Assert(!DoubleUtil.IsZero(height));
            double deltaY = 0; 
            int newFirstVisibleScrollingRow = this.DisplayData.FirstDisplayedScrollingRow; 
            if (height > 0)
            { 
                // Scrolling Down
                deltaY = GetEdgedRowHeight(newFirstVisibleScrollingRow) - _negVerticalOffset;
                if (DoubleUtil.LessThan(height, deltaY)) 
                {
                    // We've merely covered up more of the same row we're on
                    _negVerticalOffset += height; 
                } 
                else
                { 
                    // Figure out what row we've scrolled down to and update the value for _negVerticalOffset
                    _negVerticalOffset = 0;
                    if (height > 2 * this.CellsHeight) 
                    {
                        if (this._vScrollBar != null && DoubleUtil.AreClose(this._vScrollBar.Maximum, newVerticalOffset))
                        { 
                            this.DisplayData.LastDisplayedScrollingRow = this._rowCount - 1; 
                            ComputeDisplayedRowsFromBottom();
                            newFirstVisibleScrollingRow = this.DisplayData.FirstDisplayedScrollingRow; 
                        }
                        else
                        { 
                            // Very large scroll occurred. Instead of determining the exact number of scrolled off rows,
                            // let's estimate the number based on this.RowHeight.
                            // 
                            double horizontalGridlinesThickness = this.AreRowBottomGridlinesRequired ? DataGrid.HorizontalGridlinesThickness : 0; 
                            newFirstVisibleScrollingRow = Math.Min(newFirstVisibleScrollingRow + (int)(height / (this.RowHeight + horizontalGridlinesThickness)), this._rowCount - 1);
                        } 
                    }
                    else
                    { 
                        while (DoubleUtil.LessThanOrClose(deltaY, height))
                        {
                            if (newFirstVisibleScrollingRow < _rowCount - 1) 
                            { 
                                newFirstVisibleScrollingRow++;
                            } 
                            else
                            {
                                // We're being told to scroll beyond the last row, ignore the extra 
                                _negVerticalOffset = 0;
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
                                _negVerticalOffset = remainingHeight; 
                                break;
                            } 
                        }
                    }
                } 
            }
            else
            { 
                if (DoubleUtil.GreaterThanOrClose(height + _negVerticalOffset, 0)) 
                {
                    // We've merely exposing more of the row we're on 
                    _negVerticalOffset += height;
                }
                else 
                {
                    // Figure out what row we've scrolled up to and update the value for _negVerticalOffset
                    deltaY = -_negVerticalOffset; 
                    this._negVerticalOffset = 0; 
                    if (height < -2 * this.CellsHeight)
                    { 
                        // Very large scroll occurred. Instead of determining the exact number of scrolled off rows,
                        // let's estimate the number based on this.RowHeight.
                        if (newVerticalOffset == 0) 
                        {
                            newFirstVisibleScrollingRow = 0;
                        } 
                        else 
                        {
                            // 
                            double horizontalGridlinesThickness = this.AreRowBottomGridlinesRequired ? DataGrid.HorizontalGridlinesThickness : 0;
                            newFirstVisibleScrollingRow = Math.Max(newFirstVisibleScrollingRow + (int)(height / (this.RowHeight + horizontalGridlinesThickness)), 0);
                        } 
                    }
                    else
                    { 
                        while (DoubleUtil.GreaterThan(deltaY, height)) 
                        {
                            if (newFirstVisibleScrollingRow > 0) 
                            {
                                newFirstVisibleScrollingRow--;
                            } 
                            else
                            {
                                _negVerticalOffset = 0; 
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
                                _negVerticalOffset = rowHeight + remainingHeight;
                                break; 
                            }
                        }
                    } 
                }
            }
 
            // Tentative target value for this.DisplayData.FirstDisplayedScrollingRow. 
            this.DisplayData.FirstDisplayedScrollingRow = newFirstVisibleScrollingRow;
            // This sets the actual value of this.DisplayData.FirstDisplayedScrollingRow 
            ComputeDisplayedRows(this.CellsHeight);

            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= 0); 
            if (this.DisplayData.FirstDisplayedScrollingRow != newFirstVisibleScrollingRow)
            {
                double firstDisplayedScrollingRowHeight = GetEdgedExactRowHeight(this.DisplayData.FirstDisplayedScrollingRow); 
                if (firstDisplayedScrollingRowHeight < this._negVerticalOffset) 
                {
                    this._negVerticalOffset = Math.Min(1, firstDisplayedScrollingRowHeight/2); 
                }
            }
 
            Debug.Assert(GetEdgedExactRowHeight(this.DisplayData.FirstDisplayedScrollingRow) > this._negVerticalOffset);

            if (this.DisplayData.FirstDisplayedScrollingRow == 0) 
            { 
                this._verticalOffset = this._negVerticalOffset;
            } 
            else if (DoubleUtil.GreaterThan(this._negVerticalOffset, newVerticalOffset))
            {
                // The scrolled-in row was larger than anticipated. 
                // Assuming the remaining scrolled off rows have the default RowHeight.
                this._verticalOffset = GetEdgedRowsHeight(0, this.DisplayData.FirstDisplayedScrollingRow - 1) + this._negVerticalOffset;
            } 
            else 
            {
                this._verticalOffset = newVerticalOffset; 
            }

            Debug.Assert(!(this._verticalOffset == 0 && this._negVerticalOffset == 0 && this.DisplayData.FirstDisplayedScrollingRow > 0)); 

            if (this._vScrollBar != null && !DoubleUtil.AreClose(this._vScrollBar.Value, this._verticalOffset))
            { 
                try 
                {
                    this._verticalScrollChangesIgnored++; 
                    this._vScrollBar.Value = this._verticalOffset;
                }
                finally 
                {
                    this._verticalScrollChangesIgnored--;
                } 
            } 

            Debug.Assert(DoubleUtil.GreaterThanOrClose(_negVerticalOffset, 0)); 
            Debug.Assert(DoubleUtil.GreaterThanOrClose(this._verticalOffset,  this._negVerticalOffset));
            this.DisplayData.Dirty = false;
        } 

        private void SelectRow(int rowIndex, bool isSelected)
        { 
            this.SelectedItemsInternal.SelectRow(rowIndex, isSelected); 
            if (IsRowDisplayed(rowIndex))
            { 
                DataGridRow row = GetDisplayedRow(rowIndex);
                Debug.Assert(row != null);
                row.ApplyBackgroundBrush(true /*animate*/); 
                EnsureRowDetailsVisibility(row, true /*animate*/);
            }
        } 
 
        private void SelectRows(int startRowIndex, int endRowIndex, bool isSelected)
        { 
            this.SelectedItemsInternal.SelectRows(startRowIndex, endRowIndex - startRowIndex + 1 /*rowCount*/, isSelected);

            // Apply the correct row state for display rows and also expand or collapse detail accordingly 
            int firstRow = Math.Max(this._firstDisplayedRowIndex, startRowIndex);
            int lastRow = Math.Min(this._lastDisplayedRowIndex, endRowIndex);
 
            for (int rowIndex = firstRow; rowIndex <= lastRow; rowIndex++) 
            {
                Debug.Assert(IsRowDisplayed(rowIndex)); 
                DataGridRow row = GetDisplayedRow(rowIndex);
                Debug.Assert(row != null);
                row.ApplyBackgroundBrush(true /*animate*/); 
                EnsureRowDetailsVisibility(row, true /*animate*/);
            }
        } 
 
        #endregion Private Methods
 
        #region Debugging Methods

#if DEBUG 
        [SuppressMessage("Microsoft.Naming", "CA1707:IdentifiersShouldNotContainUnderscores", Justification="This is a debug method")]
        public void Debug_SetRowSelection(int rowIndex, bool isSelected, bool setAnchorRowIndex)
        { 
            SetRowSelection(rowIndex, isSelected, setAnchorRowIndex); 
        }
 
        [SuppressMessage("Microsoft.Naming", "CA1707:IdentifiersShouldNotContainUnderscores", Justification="This is a debug method")]
        public void Debug_SwitchIsRowDetailsVisible(/*int rowIndex,*/ DataTemplate detailsTemplate, bool overrideDetailsScrolling)
        { 
            //
            this.OverrideRowDetailsScrolling = overrideDetailsScrolling;
            // Apply Details template at DataGrid level 
            if (this.RowDetailsTemplate == null) 
            {
                this.RowDetailsTemplate = detailsTemplate; 
            }

            this.RowDetailsVisibility = this.RowDetailsVisibility == DataGridRowDetailsVisibility.Visible ? 
                DataGridRowDetailsVisibility.Collapsed : DataGridRowDetailsVisibility.Visible;

            // 
 

 


 
        }

        [SuppressMessage("Microsoft.Naming", "CA1707:IdentifiersShouldNotContainUnderscores", Justification="This is a debug method")] 
        public void Debug_SetRowStyle(int rowIndex, Style style) 
        {
            DataGridRow row = this.GetDisplayedRow(rowIndex); 
            row.Style = style;
        }
#endif 

        #endregion Debugging Methods
    } 
} 
