// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Reflection; 
using System.Windows.Data;
using System.Windows.Controls;

namespace System.Windows.Controlsb1
{
    public partial class DataGrid
    { 
        #region Private Properties
        #endregion Private Properties
 
        #region Protected Methods 

        protected virtual void OnColumnDisplayIndexChanged(DataGridColumnEventArgs e) 
        {
            EventHandler<DataGridColumnEventArgs> handler = this.ColumnDisplayIndexChanged;
            if (handler != null) 
            {
                handler(this, e);
            } 
        } 

        #endregion Protected Methods 

        #region Internal Methods
 
        internal bool ColumnRequiresRightGridline(DataGridColumnBase dataGridColumn, bool includeLastRightGridlineWhenPresent)
        {
            return (this.GridlinesVisibility == DataGridGridlines.Vertical || this.GridlinesVisibility == DataGridGridlines.All) && this.VerticalGridlinesBrush != null /*&& this.VerticalGridlinesThickness > 0*/ && 
                   (dataGridColumn != this.ColumnsInternal.LastVisibleColumn || (includeLastRightGridlineWhenPresent && this.ColumnsInternal.FillerColumn.IsActive)); 
        }
 
        internal DataGridColumnCollection CreateColumnsInstance()
        {
            return new DataGridColumnCollection(this); 
        }

        internal bool GetColumnReadOnlyState(DataGridColumnBase dataGridColumn, bool isReadOnly) 
        { 
            Debug.Assert(dataGridColumn != null);
 
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase;
            if (dataGridBoundColumn != null && !string.IsNullOrEmpty(dataGridBoundColumn.DisplayMemberPath))
            { 
                if (dataGridBoundColumn.DisplayMemberBinding != null
                    && dataGridBoundColumn.DisplayMemberBinding.Converter != null
                    && dataGridBoundColumn.DisplayMemberBinding.Converter is DataGridValueConverter) 
                { 
                    if (!DataGridValueConverter.CanEdit(dataGridBoundColumn.Type))
                    { 
                        return true;
                    }
                } 

                return this.DataConnection.GetPropertyIsReadOnly(dataGridBoundColumn.DisplayMemberPath) || isReadOnly;
            } 
 
            return isReadOnly;
        } 

        // Returns the column's width + the potential thickness of the right gridline
        internal double GetEdgedColumnWidth(DataGridColumnBase dataGridColumn) 
        {
            Debug.Assert(dataGridColumn != null);
            return GetEdgedColumnWidth(dataGridColumn, true /*includeLastRightGridlineWhenPresent*/); 
        } 

        internal double GetEdgedColumnWidth(DataGridColumnBase dataGridColumn, bool includeLastRightGridlineWhenPresent) 
        {
            Debug.Assert(dataGridColumn != null);
            return dataGridColumn.Width + (ColumnRequiresRightGridline(dataGridColumn, includeLastRightGridlineWhenPresent) ? DataGrid.VerticalGridlinesThickness : 0); 
        }

        // 
 

 

        internal void OnClearingColumns()
        { 
            // Rows need to be cleared first. There cannot be rows without also having columns.
            ClearRows();
 
            // Removing all the column header cells 
            RemoveDisplayedColumnHeaders();
 
            //

            // Trash all recyclable rows 
            this._recyclableRows.Clear();

            /* 
 
*/
 
            //

 


 
 

 
            this._horizontalOffset = this._negHorizontalOffset = 0;
            if (this._hScrollBar != null && this._hScrollBar.Visibility == Visibility.Visible) //
            { 
                this._hScrollBar.Value = 0;
            }
        } 
 
        //
 


 
        internal void OnColumnCellStyleChanged(DataGridColumnBase column)
        {
            // Set HeaderCell.Style for displayed rows if HeaderCell.Style is not already set 
            for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++) 
            {
                DataGridRow row = _cells.Children[childIndex] as DataGridRow; 
                if (row != null)
                {
                    row.Cells[column.Index].Style = column.CellStyle; 
                }
            }
            // Set HeaderCell.Style for prefetched rows if HeaderCell.Style is not already set 
            foreach (DataGridRow row in _prefetchedRows) 
            {
                row.Cells[column.Index].Style = column.CellStyle; 
            }
            // Simply discard the Recyclable rows.  This operation happens less enough that discarding
            // the Recyclable rows here shoudln't be too bad 
            _recyclableRows.Clear();
        }
 
        // 

 


        internal void OnColumnCollectionChanged_PostNotification(bool columnsGrew) 
        {
            UpdateColumnHeadersSeparatorVisibility();
            if (columnsGrew && 
                this.CurrentColumnIndex == -1) 
            {
                MakeFirstDisplayedCellCurrentCell(); 
            }
        }
 
        internal void OnColumnCollectionChanged_PreNotification(bool columnsGrew)
        {
            // dataGridColumn==null means the collection was refreshed. 
 
            //
 
            if (columnsGrew && this.ColumnsItemsInternal.Count == 1)
            {
                RefreshRows(); 
            }
            PerformLayout();
        } 
 
        internal void OnColumnDisplayIndexChanged(DataGridColumnBase dataGridColumn)
        { 
            Debug.Assert(dataGridColumn != null);
            DataGridColumnEventArgs e = new DataGridColumnEventArgs(dataGridColumn);
 
            // Call protected method to raise event
            OnColumnDisplayIndexChanged(e);
        } 
 
        internal void OnColumnDisplayIndexChanged_PostNotification()
        { 
            UpdateColumnHeadersSeparatorVisibility();
            // Notifications for adjusted display indexes.
            FlushDisplayIndexChanged(true /*raiseEvent*/); 
            PerformLayout();
        }
 
        internal void OnColumnDisplayIndexChanged_PreNotification() 
        {
            Debug.Assert(InDisplayIndexAdjustments); 

            // column.DisplayIndex changed - this may require a complete re-layout of the control
            this.ColumnsInternal.InvalidateCachedColumnsOrder(); 
        }

        internal void OnColumnDisplayIndexChanging(DataGridColumnBase targetColumn, int newDisplayIndex) 
        { 
            Debug.Assert(targetColumn != null);
            Debug.Assert(newDisplayIndex != targetColumn.DisplayIndex); 

            if (InDisplayIndexAdjustments)
            { 
                // We are within columns display indexes adjustments. We do not allow changing display indexes while adjusting them.
                throw DataGridError.DataGrid.CannotChangeColumnCollectionWhileAdjustingDisplayIndexes();
            } 
 
            // Throws an exception if the requested move is illegal
            CorrectColumnFrozenStatesForMove(targetColumn, newDisplayIndex); 

            try
            { 

                InDisplayIndexAdjustments = true;
 
                // Move is legal - let's adjust the affected display indexes. 
                if (newDisplayIndex < targetColumn.DisplayIndex)
                { 
                    // DisplayIndex decreases. All columns with newDisplayIndex <= DisplayIndex < targetColumn.DisplayIndex
                    // get their DisplayIndex incremented.
                    foreach (DataGridColumnBase column in this.Columns) 
                    {
                        if (newDisplayIndex <= column.DisplayIndex && column.DisplayIndex < targetColumn.DisplayIndex)
                        { 
                            column.DisplayIndexInternal = column.DisplayIndex + 1; 
                            column.DisplayIndexHasChanged = true; // OnColumnDisplayIndexChanged needs to be raised later on
                        } 
                    }
                }
                else 
                {
                    // DisplayIndex increases. All columns with targetColumn.DisplayIndex < DisplayIndex <= newDisplayIndex
                    // get their DisplayIndex incremented. 
                    foreach (DataGridColumnBase column in this.Columns) 
                    {
                        if (targetColumn.DisplayIndex < column.DisplayIndex && column.DisplayIndex <= newDisplayIndex) 
                        {
                            column.DisplayIndexInternal = column.DisplayIndex - 1;
                            column.DisplayIndexHasChanged = true; // OnColumnDisplayIndexChanged needs to be raised later on 
                        }
                    }
                } 
            } 
            finally
            { 
                InDisplayIndexAdjustments = false;
            }
 
            // Note that displayIndex of moved column is updated by caller.
        }
 
        internal void OnColumnDisplayMemberBindingChanged(DataGridBoundColumnBase column) 
        {
            // Update Binding in Displayed rows by regenerating the affected elements 
            for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++)
            {
                DataGridRow row = _cells.Children[childIndex] as DataGridRow; 
                if (row != null)
                {
                    PopulateCellContent(true /* forceTemplating */, false, column, row, row.Cells[column.Index]); 
                } 
            }
 
            // Simply discard the Prefetched and Recyclable rows.  This operation happens less enough that discarding
            // the Recyclable rows here shoudln't be too bad
            _prefetchedRows.Clear(); 
            _recyclableRows.Clear();
        }
 
        internal void OnColumnElementStyleChanged(DataGridBoundColumnBase column) 
        {
            // Update Element Style in Displayed rows 
            for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++)
            {
                DataGridRow row = _cells.Children[childIndex] as DataGridRow; 
                if (row != null)
                {
                    FrameworkElement element = column.GetElement(row); 
                    // 
                    if (element != null && element.Style == null)
                    { 
                        element.Style = column.ElementStyle;
                    }
                } 
            }
            // Set Element Style for prefetched rows if HeaderCell.Style is not already set
            foreach (DataGridRow row in _prefetchedRows) 
            { 
                FrameworkElement element = column.GetElement(row);
                // 
                if (element != null && element.Style == null)
                {
                    element.Style = column.ElementStyle; 
                }
            }
 
            // Simply discard the Recyclable rows.  This operation happens less enough that discarding 
            // the Recyclable rows here shoudln't be too bad
            _recyclableRows.Clear(); 
        }

        internal void OnColumnFrozenStateChanged(DataGridColumnBase updatedColumn) 
        {
            Debug.Assert(updatedColumn != null);
            if (updatedColumn.Visibility == Visibility.Visible) 
            { 
                if (updatedColumn.IsFrozen)
                { 
                    // visible column became frozen
                    if (this._horizontalOffset >= GetEdgedColumnWidth(updatedColumn))
                    { 
                        Debug.Assert(this.ColumnsInternal.DisplayInOrder(updatedColumn.Index, this.DisplayData.FirstDisplayedScrollingCol));
                        this._horizontalOffset -= GetEdgedColumnWidth(updatedColumn);
                    } 
                    else 
                    {
                        this._horizontalOffset = this._negHorizontalOffset = 0; 
                    }
                }
                else 
                {
                    // column was unfrozen - make it the first visible scrolling column if there is room
                    this._horizontalOffset = this._negHorizontalOffset = 0; 
                } 
                //
 


 

                PerformLayout();
            } 
        } 

        internal void OnColumnFrozenStateChanging(DataGridColumnBase targetColumn) 
        {
            Debug.Assert(targetColumn != null);
            CorrectColumnFrozenStates(targetColumn, true); 
        }

        // 
 

 


        internal void OnColumnReadOnlyStateChanging(DataGridColumnBase dataGridColumn, bool isReadOnly) 
        {
            Debug.Assert(dataGridColumn != null);
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase; 
            if (!isReadOnly && !string.IsNullOrEmpty(dataGridBoundColumn.DisplayMemberPath)) 
            {
                if (this.DataConnection.GetPropertyIsReadOnly(dataGridBoundColumn.DisplayMemberPath)) 
                {
                    // Cannot set a column to read/write when the backend does not allow it.
                    throw DataGridError.DataGrid.UnderlyingPropertyIsReadOnly(); 
                }
            }
            if (isReadOnly && this.CurrentColumnIndex == dataGridColumn.Index) 
            { 
                // Edited column becomes read-only. Exit editing mode.
                if (!EndCellEdit(true /*commitCellEdit*/, true /*exitEditingMode*/, this.ContainsFocus /*keepFocus*/)) 
                {
                    throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                } 
            }
        }
 
        internal void OnColumnVisibleStateChanged(DataGridColumnBase updatedColumn) 
        {
            Debug.Assert(updatedColumn != null); 
            //

            UpdateColumnHeadersSeparatorVisibility(); 
            PerformLayout();
            if (updatedColumn.Visibility == Visibility.Visible &&
                this.ColumnsInternal.VisibleColumnCount == 1 && this.CurrentColumnIndex == -1) 
            { 
                DataGridRow dataGridRow = GetRowFromItem(this.SelectedItem);
                if (dataGridRow != null) 
                {
                    SetAndSelectCurrentCell(updatedColumn.Index, dataGridRow.Index, true /*forceCurrentCellSelection*/);
                } 
                else
                {
                    MakeFirstDisplayedCellCurrentCell(); 
                } 
            }
        } 

        internal void OnColumnVisibleStateChanging(DataGridColumnBase targetColumn)
        { 
            Debug.Assert(targetColumn != null);

            if (targetColumn.Visibility == Visibility.Visible && 
                this.CurrentColumn == targetColumn) 
            {
                // Column of the current cell is made invisible. Trying to move the current cell to a neighbor column. May throw an exception. 
                DataGridColumnBase dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(targetColumn);
                if (dataGridColumn == null)
                { 
                    dataGridColumn = this.ColumnsInternal.GetPreviousVisibleColumn(targetColumn);
                }
                if (dataGridColumn == null) 
                { 
                    SetCurrentCellCore(-1, -1);
                } 
                else
                {
                    SetCurrentCellCore(dataGridColumn.Index, this.CurrentRowIndex); 
                }
            }
 
            CorrectColumnFrozenStates(targetColumn, false); 
        }
 
        internal void OnColumnWidthChanged(DataGridColumnBase updatedColumn)
        {
            Debug.Assert(updatedColumn != null); 
            if (updatedColumn.Visibility == Visibility.Visible)
            {
                PerformLayout(true /* forceDataCellsHorizontalLayout */); 
            } 
        }
 
        internal void OnInsertedColumn_PostNotification(DataGridCellCoordinates newCurrentCellCoordinates)
        {
            // Update current cell if needed 
            if (newCurrentCellCoordinates.ColumnIndex != -1)
            {
                Debug.Assert(this.CurrentColumnIndex == -1); 
                bool success = SetAndSelectCurrentCell(newCurrentCellCoordinates.ColumnIndex, 
                                                       newCurrentCellCoordinates.RowIndex,
                                                       this.ColumnsInternal.VisibleColumnCount == 1 /*forceCurrentCellSelection*/); 
                Debug.Assert(success);
            }
        } 

        internal void OnInsertedColumn_PreNotification(DataGridColumnBase insertedColumn)
        { 
            // Fix the OldFirstDisplayedScrollingCol 
            this.DisplayData.CorrectColumnIndexAfterInsertion(insertedColumn.Index, 1);
 
            // Fix the Index of all following columns
            CorrectColumnIndexesAfterInsertion(insertedColumn, 1);
 
            Debug.Assert(insertedColumn.Index >= 0);
            Debug.Assert(insertedColumn.Index < this.ColumnsItemsInternal.Count);
            Debug.Assert(insertedColumn.OwningGrid == this); 
 
            if (insertedColumn.DisplayIndex == -1 || insertedColumn.DisplayIndex >= this.ColumnsItemsInternal.Count)
            { 
                // Developer did not assign a DisplayIndex or picked a large number.
                // Choose the Index as the DisplayIndex.
                insertedColumn.DisplayIndexInternal = insertedColumn.Index; 
            }

            CorrectColumnDisplayIndexesAfterInsertion(insertedColumn); 
 
            InsertDisplayedColumnHeader(insertedColumn);
 
            // Insert the missing data cells
            if (_rowCount > 0)
            { 
                int newColumnCount = this.ColumnsItemsInternal.Count;

                if (_cells != null) 
                { 
                    //
                    for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++) 
                    {
                        DataGridRow dataGridRow = this._cells.Children[childIndex] as DataGridRow;
                        if (dataGridRow.Cells.Count < newColumnCount) 
                        {
                            AddNewCellPrivate(dataGridRow, insertedColumn);
                        } 
                        foreach (DataGridCell dataGridCell in dataGridRow.Cells) 
                        {
                            if (ColumnRequiresRightGridline(dataGridCell.OwningColumn, true /*includeLastRightGridlineWhenPresent*/)) 
                            {
                                dataGridCell.EnsureGridline();
                            } 
                        }
                    }
                } 
            } 

            // 


 


 
 

 


 


        } 
 
        internal DataGridCellCoordinates OnInsertingColumn(int columnIndexInserted, DataGridColumnBase insertColumn)
        { 
            DataGridCellCoordinates newCurrentCellCoordinates;
            Debug.Assert(insertColumn != null);
 
            if (insertColumn.OwningGrid != null)
            {
                throw DataGridError.DataGrid.ColumnCannotBeReassignedToDifferentDataGrid(); 
            } 

            // Trash all recyclable rows 
            this._recyclableRows.Clear();

            // check for correctness of frozen state - throws exception if state is incorrect. 
            CorrectColumnFrozenState(insertColumn, columnIndexInserted);

            // Reset current cell if there is one, no matter the relative position of the columns involved 
            if (this.CurrentColumnIndex != -1) 
            {
                newCurrentCellCoordinates = new DataGridCellCoordinates(columnIndexInserted <= this.CurrentColumnIndex ? this.CurrentColumnIndex + 1 : this.CurrentColumnIndex, 
                     this.CurrentRowIndex);
                ResetCurrentCellCore();
            } 
            else
            {
                newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1); 
            } 
            return newCurrentCellCoordinates;
        } 

        internal void OnRemovedColumn_PostNotification(DataGridCellCoordinates newCurrentCellCoordinates)
        { 
            // Update current cell if needed
            if (newCurrentCellCoordinates.ColumnIndex != -1)
            { 
                Debug.Assert(this.CurrentColumnIndex == -1); 
                bool success = SetAndSelectCurrentCell(newCurrentCellCoordinates.ColumnIndex,
                                                       newCurrentCellCoordinates.RowIndex, 
                                                       false /*forceCurrentCellSelection*/);
                Debug.Assert(success);
            } 
        }

        internal void OnRemovedColumn_PreNotification(DataGridColumnBase removedColumn) 
        { 
            Debug.Assert(removedColumn.Index >= 0);
            Debug.Assert(removedColumn.OwningGrid == null); 

            // Intentionally keep the DisplayIndex intact after detaching the column.
            CorrectColumnIndexesAfterDeletion(removedColumn); 

            CorrectColumnDisplayIndexesAfterDeletion(removedColumn);
 
            // Fix the OldFirstDisplayedScrollingCol 
            this.DisplayData.CorrectRowIndexAfterDeletion(removedColumn.Index);
 
            // Fix the existing rows by removing cells at correct index
            int newColumnCount = this.ColumnsItemsInternal.Count;
 
            if (this._cells != null)
            {
                for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++) 
                { 
                    DataGridRow dataGridRow = this._cells.Children[childIndex] as DataGridRow;
                    if (dataGridRow.Cells.Count > newColumnCount) 
                    {
                        dataGridRow.Cells.RemoveAt(removedColumn.Index);
                    } 
                }
            }
 
            // 

 


 


 
            RemoveDisplayedColumnHeader(removedColumn); 
        }
 
        internal DataGridCellCoordinates OnRemovingColumn(DataGridColumnBase dataGridColumn)
        {
            Debug.Assert(dataGridColumn != null); 
            Debug.Assert(dataGridColumn.Index >= 0 && dataGridColumn.Index < this.ColumnsItemsInternal.Count);

            DataGridCellCoordinates newCurrentCellCoordinates; 
 
            this._temporarilyResetCurrentCell = false;
            int columnIndex = dataGridColumn.Index; 

            // Trash all recyclable rows
            this._recyclableRows.Clear(); 

            // Reset the current cell's address if there is one.
            if (this.CurrentColumnIndex != -1) 
            { 
                int newCurrentColumnIndex = this.CurrentColumnIndex;
                if (columnIndex == newCurrentColumnIndex) 
                {
                    DataGridColumnBase dataGridColumnNext = this.ColumnsInternal.GetNextVisibleColumn(this.ColumnsItemsInternal[columnIndex]);
                    if (dataGridColumnNext != null) 
                    {
                        if (dataGridColumnNext.Index > columnIndex)
                        { 
                            newCurrentColumnIndex = dataGridColumnNext.Index - 1; 
                        }
                        else 
                        {
                            newCurrentColumnIndex = dataGridColumnNext.Index;
                        } 
                    }
                    else
                    { 
                        DataGridColumnBase dataGridColumnPrevious = this.ColumnsInternal.GetPreviousVisibleColumn(this.ColumnsItemsInternal[columnIndex]); 
                        if (dataGridColumnPrevious != null)
                        { 
                            if (dataGridColumnPrevious.Index > columnIndex)
                            {
                                newCurrentColumnIndex = dataGridColumnPrevious.Index - 1; 
                            }
                            else
                            { 
                                newCurrentColumnIndex = dataGridColumnPrevious.Index; 
                            }
                        } 
                        else
                        {
                            newCurrentColumnIndex = -1; 
                        }
                    }
                } 
                else if (columnIndex < newCurrentColumnIndex) 
                {
                    newCurrentColumnIndex--; 
                }
                newCurrentCellCoordinates = new DataGridCellCoordinates(newCurrentColumnIndex, (newCurrentColumnIndex == -1) ? -1 : this.CurrentRowIndex);
                if (columnIndex == this.CurrentColumnIndex) 
                {
                    // Left cell is not validated since cancelling validation wouldn't have any effect anyways.
                    bool success = SetCurrentCellCore(-1, -1/* */); 
                    Debug.Assert(success); 
                }
                else // 
                {
                    // Underlying data of deleted column is gone. It cannot be accessed anymore.
                    // Do not end editing mode so that CellValidation doesn't get raised, since that event needs the current formatted value. 
                    this._temporarilyResetCurrentCell = true;
                    bool success = SetCurrentCellCore(-1, -1/* */);
                    Debug.Assert(success); 
                } 
                //
 


 

            }
            else 
            { 
                newCurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);
            } 

            // If the last column is removed, delete all the rows first.
            if (this.ColumnsItemsInternal.Count == 1) 
            {
                ClearRows();
            } 
            else 
            {
                // Removing the potential vertical gridlines 
                RemoveDisplayedVerticalGridlines(dataGridColumn);
            }
 
            // Is deleted column scrolled off screen?
            if (dataGridColumn.Visibility == Visibility.Visible &&
                !dataGridColumn.IsFrozen && 
                this.DisplayData.FirstDisplayedScrollingCol >= 0) 
            {
                // Deleted column is part of scrolling columns. 
                if (this.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index)
                {
                    // Deleted column is first scrolling column 
                    this._horizontalOffset -= this._negHorizontalOffset;
                    this._negHorizontalOffset = 0;
                } 
                else if (!this.ColumnsInternal.DisplayInOrder(this.DisplayData.FirstDisplayedScrollingCol, dataGridColumn.Index)) 
                {
                    // Deleted column is displayed before first scrolling column 
                    Debug.Assert(this._horizontalOffset >= GetEdgedColumnWidth(dataGridColumn));
                    this._horizontalOffset -= GetEdgedColumnWidth(dataGridColumn);
                } 

                if (this._hScrollBar != null && this._hScrollBar.Visibility == Visibility.Visible) //
                { 
                    this._hScrollBar.Value = this._horizontalOffset; 
                }
            } 

            return newCurrentCellCoordinates;
        } 

        /// <summary>
        /// Called when a column property changes, and its cells need to 
        /// adjust that that column change. 
        /// </summary>
        internal void UpdateColumnElements(DataGridColumnBase dataGridColumn, string propertyName) 
        {
            Debug.Assert(dataGridColumn != null);
 
            //

 
 
            // Take care of the non-displayed prepared rows
            for (int index = 0; index < this._preparedRows.Count; ) 
            {
                DataGridRow dataGridRow = this._preparedRows[index];
                Debug.Assert(dataGridRow != null); 
                if (!IsRowDisplayed(dataGridRow.Index))
                {
                    UpdateCellElement(dataGridColumn, dataGridRow, propertyName); 
                } 
                index++;
            } 

            // Take care of the non-displayed edited row
            if (this._editingRow != null && !IsRowDisplayed(this._editingRow.Index)) 
            {
                UpdateCellElement(dataGridColumn, this._editingRow, propertyName);
            } 
 
            // Take care of the displayed rows
            if (this._cells != null) 
            {
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++)
                { 
                    UpdateCellElement(dataGridColumn, GetDisplayedRow(this._firstDisplayedRowIndex + childIndex), propertyName);
                }
            } 
        } 

        #endregion Internal Methods 

        #region Private Methods
 
        //

 
 
        private void AutoGenerateColumnsPrivate()
        { 
            // Always clear existing columns before generating new ones
            this.ColumnsInternal.Clear();
            if (this.DataConnection.DataProperties != null) 
            {
                foreach (PropertyInfo propertyInfo in this.DataConnection.DataProperties)
                { 
                    // 

 


 


 
 

 


 


 
 

 

                    GenerateColumn(propertyInfo.PropertyType, propertyInfo.Name, propertyInfo.Name, propertyInfo.Name, propertyInfo);
                } 
            }
            else if (this.DataConnection.DataIsPrimitive)
            { 
                this.GenerateColumn(this.DataConnection.DataType, string.Empty, string.Empty, "value", null); 
            }
        } 

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private bool ComputeDisplayedColumns() 
        {
            bool invalidate = false;
            DataGridColumnBase dataGridColumn = null; 
            int numVisibleScrollingCols = 0; 
            int visibleScrollingColumnsTmp = 0;
            double displayWidth = this.CellsWidth; 
            double cx = 0;
            int numDisplayedFrozenCols = 0;
            int firstDisplayedFrozenCol = -1; 
            int lastDisplayedFrozenCol = -1;
            int firstDisplayedScrollingCol = this.DisplayData.FirstDisplayedScrollingCol;
 
            // the same problem with negative numbers: 
            // if the width passed in is negative, then return 0
            if (displayWidth <= 0 || this.ColumnsInternal.VisibleColumnCount == 0) 
            {
                this.DisplayData.FirstDisplayedFrozenCol = -1;
                this.DisplayData.NumDisplayedFrozenCols = 0; 
                this.DisplayData.FirstDisplayedScrollingCol = -1;
                this.DisplayData.NumDisplayedScrollingCols = 0;
                this.DisplayData.LastDisplayedFrozenCol = -1; 
                this.DisplayData.LastTotallyDisplayedScrollingCol = -1; 
                return invalidate;
            } 

            dataGridColumn = this.ColumnsInternal.FirstColumn;
            while (dataGridColumn != null) 
            {
                if (!dataGridColumn.IsFrozen && dataGridColumn.Visibility == Visibility.Visible)
                { 
                    break; 
                }
                if (dataGridColumn.Visibility == Visibility.Visible) 
                {
                    if (firstDisplayedFrozenCol == -1)
                    { 
                        firstDisplayedFrozenCol = dataGridColumn.Index;
                    }
                    cx += GetEdgedColumnWidth(dataGridColumn); 
                    numDisplayedFrozenCols++; 
                    lastDisplayedFrozenCol = dataGridColumn.Index;
                    if (cx >= displayWidth) 
                    {
                        break;
                    } 
                }
                dataGridColumn = this.ColumnsInternal.GetNextColumn(dataGridColumn);
            } 
 
            Debug.Assert(cx <= this.ColumnsInternal.GetVisibleFrozenEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/));
 
            if (cx < displayWidth && firstDisplayedScrollingCol >= 0)
            {
                dataGridColumn = this.ColumnsItemsInternal[firstDisplayedScrollingCol]; 
                if (dataGridColumn.IsFrozen)
                {
                    dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn; 
                    this._negHorizontalOffset = 0; 
                    if (dataGridColumn == null)
                    { 
                        this.DisplayData.FirstDisplayedFrozenCol = firstDisplayedFrozenCol;
                        this.DisplayData.LastDisplayedFrozenCol = lastDisplayedFrozenCol;
                        this.DisplayData.NumDisplayedFrozenCols = numDisplayedFrozenCols; 
                        this.DisplayData.FirstDisplayedScrollingCol = this.DisplayData.LastTotallyDisplayedScrollingCol = -1;
                        this.DisplayData.NumDisplayedScrollingCols = 0;
                        return invalidate; 
                    } 
                    else
                    { 
                        firstDisplayedScrollingCol = dataGridColumn.Index;
                    }
                } 

                cx -= this._negHorizontalOffset;
                while (cx < displayWidth && dataGridColumn != null) 
                { 
                    cx += GetEdgedColumnWidth(dataGridColumn);
                    visibleScrollingColumnsTmp++; 
                    dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn);
                }
                numVisibleScrollingCols = visibleScrollingColumnsTmp; 

                // if we inflate the data area then we paint columns to the left of firstDisplayedScrollingCol
                if (cx < displayWidth) 
                { 
                    Debug.Assert(firstDisplayedScrollingCol >= 0);
                    //first minimize value of this._negHorizontalOffset 
                    if (this._negHorizontalOffset > 0)
                    {
                        invalidate = true; 
                        if (displayWidth - cx > this._negHorizontalOffset)
                        {
                            cx += this._negHorizontalOffset; 
                            this._horizontalOffset -= this._negHorizontalOffset; 
                            this._negHorizontalOffset = 0;
                        } 
                        else
                        {
                            this._horizontalOffset -= displayWidth - cx; 
                            this._negHorizontalOffset -= displayWidth - cx;
                            cx = displayWidth;
                        } 
                    } 
                    // second try to scroll entire columns
                    if (cx < displayWidth && this._horizontalOffset > 0) 
                    {
                        Debug.Assert(this._negHorizontalOffset == 0);
                        dataGridColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(this.ColumnsItemsInternal[firstDisplayedScrollingCol]); 
                        while (dataGridColumn != null && cx + GetEdgedColumnWidth(dataGridColumn) <= displayWidth)
                        {
                            cx += GetEdgedColumnWidth(dataGridColumn); 
                            visibleScrollingColumnsTmp++; 
                            invalidate = true;
                            firstDisplayedScrollingCol = dataGridColumn.Index; 
                            this._horizontalOffset -= GetEdgedColumnWidth(dataGridColumn);
                            dataGridColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(dataGridColumn);
                        } 
                    }
                    // third try to partially scroll in first scrolled off column
                    if (cx < displayWidth && this._horizontalOffset > 0) 
                    { 
                        Debug.Assert(this._negHorizontalOffset == 0);
                        dataGridColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(this.ColumnsItemsInternal[firstDisplayedScrollingCol]); 
                        Debug.Assert(dataGridColumn != null);
                        Debug.Assert(GetEdgedColumnWidth(dataGridColumn) > displayWidth - cx);
                        firstDisplayedScrollingCol = dataGridColumn.Index; 
                        this._negHorizontalOffset = GetEdgedColumnWidth(dataGridColumn) - displayWidth + cx;
                        this._horizontalOffset -= displayWidth - cx;
                        visibleScrollingColumnsTmp++; 
                        invalidate = true; 
                        cx = displayWidth;
                        Debug.Assert(this._negHorizontalOffset == GetNegHorizontalOffsetFromHorizontalOffset(this._horizontalOffset)); 
                    }

                    // update the number of visible columns to the new reality 
                    Debug.Assert(numVisibleScrollingCols <= visibleScrollingColumnsTmp, "the number of displayed columns can only grow");
                    numVisibleScrollingCols = visibleScrollingColumnsTmp;
                } 
 
                int jumpFromFirstVisibleScrollingCol = numVisibleScrollingCols - 1;
                if (cx > displayWidth) 
                {
                    jumpFromFirstVisibleScrollingCol--;
                } 

                Debug.Assert(jumpFromFirstVisibleScrollingCol >= -1);
 
                if (jumpFromFirstVisibleScrollingCol < 0) 
                {
                    this.DisplayData.LastTotallyDisplayedScrollingCol = -1; // no totally visible scrolling column at all 
                }
                else
                { 
                    Debug.Assert(firstDisplayedScrollingCol >= 0);
                    dataGridColumn = this.ColumnsItemsInternal[firstDisplayedScrollingCol];
                    for (int jump = 0; jump < jumpFromFirstVisibleScrollingCol; jump++) 
                    { 
                        dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn);
                        Debug.Assert(dataGridColumn != null); 
                    }
                    this.DisplayData.LastTotallyDisplayedScrollingCol = dataGridColumn.Index;
                } 
            }
            else
            { 
                this.DisplayData.LastTotallyDisplayedScrollingCol = -1; 
            }
            this.DisplayData.FirstDisplayedFrozenCol = firstDisplayedFrozenCol; 
            this.DisplayData.LastDisplayedFrozenCol = lastDisplayedFrozenCol;
            this.DisplayData.NumDisplayedFrozenCols = numDisplayedFrozenCols;
            this.DisplayData.FirstDisplayedScrollingCol = firstDisplayedScrollingCol; 
            this.DisplayData.NumDisplayedScrollingCols = numVisibleScrollingCols;
            Debug.Assert((this.DisplayData.NumDisplayedScrollingCols > 0 && this.DisplayData.FirstDisplayedScrollingCol != -1) ||
                         (this.DisplayData.NumDisplayedScrollingCols == 0 && this.DisplayData.FirstDisplayedScrollingCol == -1)); 
 
            return invalidate;
        } 

        private int ComputeFirstVisibleScrollingColumn()
        { 
            if (this.ColumnsInternal.GetVisibleFrozenEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/) >= this.CellsWidth)
            {
                // Not enough room for scrolling columns. 
                this._negHorizontalOffset = 0; 
                return -1;
            } 

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn;
 
            if (this._horizontalOffset == 0)
            {
                this._negHorizontalOffset = 0; 
                return (dataGridColumn == null) ? -1 : dataGridColumn.Index; 
            }
 
            double cx = 0;
            while (dataGridColumn != null)
            { 
                cx += GetEdgedColumnWidth(dataGridColumn);
                if (cx > this._horizontalOffset)
                { 
                    break; 
                }
                dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn); 
            }

            if (dataGridColumn == null) 
            {
                Debug.Assert(cx <= this._horizontalOffset);
                dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn; 
                if (dataGridColumn == null) 
                {
                    this._negHorizontalOffset = 0; 
                    return -1;
                }
                else 
                {
                    if (this._negHorizontalOffset != this._horizontalOffset)
                    { 
                        this._negHorizontalOffset = 0; 
                    }
                    return dataGridColumn.Index; 
                }
            }
            else 
            {
                this._negHorizontalOffset = GetEdgedColumnWidth(dataGridColumn) - (cx - this._horizontalOffset);
                return dataGridColumn.Index; 
            } 
        }
 
        private void CorrectColumnDisplayIndexesAfterDeletion(DataGridColumnBase deletedColumn)
        {
            // Column indexes have already been adjusted. 
            // This column has already been detached and has retained its old Index and DisplayIndex

            Debug.Assert(deletedColumn != null); 
            Debug.Assert(deletedColumn.OwningGrid == null); 
            Debug.Assert(deletedColumn.Index >= 0);
            Debug.Assert(deletedColumn.DisplayIndex >= 0); 

            try
            { 
                InDisplayIndexAdjustments = true;

                // All remaining columns with a DisplayIndex greater than dataGridColumn.DisplayIndex need to be decremented 
                foreach (DataGridColumnBase dataGridColumn in this.ColumnsItemsInternal) 
                {
                    if (dataGridColumn.DisplayIndex > deletedColumn.DisplayIndex) 
                    {
                        dataGridColumn.DisplayIndexInternal = dataGridColumn.DisplayIndex - 1;
                        dataGridColumn.DisplayIndexHasChanged = true; // OnColumnDisplayIndexChanged needs to be raised later on 
                    }
                }
#if DEBUG 
                Debug.Assert(this.ColumnsInternal.Debug_VerifyColumnDisplayIndexes()); 
#endif
                // Now raise all the OnColumnDisplayIndexChanged events 
                FlushDisplayIndexChanged(true /*raiseEvent*/);
            }
            finally 
            {
                InDisplayIndexAdjustments = false;
                FlushDisplayIndexChanged(false /*raiseEvent*/); 
            } 
        }
 
        private void CorrectColumnDisplayIndexesAfterInsertion(DataGridColumnBase insertedColumn)
        {
            Debug.Assert(insertedColumn != null); 
            Debug.Assert(insertedColumn.OwningGrid == this);
            // insertedColumn.DisplayIndex has been set already.
            Debug.Assert(insertedColumn.DisplayIndex >= 0); 
            Debug.Assert(insertedColumn.DisplayIndex < this.ColumnsItemsInternal.Count); 

            try 
            {
                InDisplayIndexAdjustments = true;
 
                // All other columns with a DisplayIndex equal or greater than dataGridColumn.DisplayIndex need to be incremented
                foreach (DataGridColumnBase dataGridColumn in this.ColumnsItemsInternal)
                { 
                    if (dataGridColumn != insertedColumn && dataGridColumn.DisplayIndex >= insertedColumn.DisplayIndex) 
                    {
                        dataGridColumn.DisplayIndexInternal = dataGridColumn.DisplayIndex + 1; 
                        dataGridColumn.DisplayIndexHasChanged = true; // OnColumnDisplayIndexChanged needs to be raised later on
                    }
                } 

#if DEBUG
                Debug.Assert(this.ColumnsInternal.Debug_VerifyColumnDisplayIndexes()); 
#endif 
                // Now raise all the OnColumnDisplayIndexChanged events
                FlushDisplayIndexChanged(true /*raiseEvent*/); 
            }
            finally
            { 
                InDisplayIndexAdjustments = false;
                FlushDisplayIndexChanged(false /*raiseEvent*/);
            } 
        } 

        private void CorrectColumnFrozenState(DataGridColumnBase targetColumn, int anticipatedColumnIndex) 
        {
            Debug.Assert(targetColumn != null);
            Debug.Assert(anticipatedColumnIndex >= 0 && anticipatedColumnIndex <= this.ColumnsItemsInternal.Count); 

            int anticipatedColumnDisplayIndex;
            if (targetColumn.DisplayIndex == -1 || targetColumn.DisplayIndex > this.ColumnsItemsInternal.Count) 
            { 
                anticipatedColumnDisplayIndex = anticipatedColumnIndex; // By default, we pick the Index as the DisplayIndex.
            } 
            else
            {
                Debug.Assert(targetColumn.DisplayIndex >= 0 && targetColumn.DisplayIndex <= this.ColumnsItemsInternal.Count); 
                anticipatedColumnDisplayIndex = targetColumn.DisplayIndex; // The specified DisplayIndex is just fine.
            }
 
            DataGridColumnBase previousColumn; 
            int displayIndex = anticipatedColumnDisplayIndex - 1;
            do 
            {
                previousColumn = this.ColumnsInternal.GetColumnAtDisplayIndex(displayIndex);
                displayIndex--; 
            }
            while (displayIndex >= 0 && (previousColumn == null || previousColumn.Visibility == Visibility.Collapsed));
            if (previousColumn != null && !previousColumn.IsFrozen && targetColumn.IsFrozen) 
            { 
                throw DataGridError.DataGrid.CannotAddNonFrozenColumn();
            } 
            else
            {
                DataGridColumnBase nextColumn; 
                displayIndex = anticipatedColumnDisplayIndex;
                do
                { 
                    nextColumn = this.ColumnsInternal.GetColumnAtDisplayIndex(displayIndex); 
                    displayIndex++;
                } 
                while (displayIndex < this.ColumnsItemsInternal.Count && (nextColumn == null || nextColumn.Visibility == Visibility.Collapsed));
                if (nextColumn != null && nextColumn.IsFrozen && !targetColumn.IsFrozen)
                { 
                    throw DataGridError.DataGrid.CannotAddNonFrozenColumn();
                }
            } 
        } 

        private void CorrectColumnFrozenStates(DataGridColumnBase targetColumn, bool frozenStateChanging) 
        {
            Debug.Assert(targetColumn != null);
            DataGridColumnBase column; 
            if ((targetColumn.IsFrozen && !frozenStateChanging) ||
                (!targetColumn.IsFrozen && frozenStateChanging))
            { 
                // make sure the previous visible columns are frozen as well 
                column = this.ColumnsInternal.GetPreviousVisibleFrozenColumn(targetColumn);
                if (column == null) 
                {
                    DataGridColumnBase firstVisibleColumn = this.ColumnsInternal.FirstVisibleColumn;
                    if (firstVisibleColumn != targetColumn) 
                    {
                        column = firstVisibleColumn;
                    } 
                } 
                while (column != null && this.ColumnsInternal.DisplayInOrder(column.Index, targetColumn.Index))
                { 
                    column.IsFrozen = true;
                    column = this.ColumnsInternal.GetNextVisibleScrollingColumn(column);
                } 
            }
            else
            { 
                // make sure the next visible columns are not frozen 
                column = this.ColumnsInternal.GetNextVisibleScrollingColumn(targetColumn);
                if (column == null) 
                {
                    DataGridColumnBase lastColumn = targetColumn;
                    do 
                    {
                        column = this.ColumnsInternal.GetNextVisibleColumn(lastColumn);
                        if (column != null) 
                        { 
                            lastColumn = column;
                        } 
                    }
                    while (column != null);
                    if (lastColumn != targetColumn) 
                    {
                        column = lastColumn;
                    } 
                } 
                while (column != null && this.ColumnsInternal.DisplayInOrder(targetColumn.Index, column.Index))
                { 
                    column.IsFrozen = false;
                    column = this.ColumnsInternal.GetPreviousVisibleFrozenColumn(column);
                } 
            }
        }
 
        // This method checks to see if the column moving is frozen or if the column is moving into a group 
        // of frozen columns.  It throws an exception in both cases.
        private void CorrectColumnFrozenStatesForMove(DataGridColumnBase targetColumn, int newDisplayIndex) 
        {
            Debug.Assert(targetColumn != null);
            Debug.Assert(newDisplayIndex != targetColumn.DisplayIndex); 
            Debug.Assert(!InDisplayIndexAdjustments);

            // No check necessary when: 
            // - column is invisible. 
            // - DisplayIndex decreases and column is frozen.
            // - DisplayIndex increases and column is unfrozen. 

            if (targetColumn.Visibility == Visibility.Collapsed ||
                (newDisplayIndex < targetColumn.DisplayIndex && targetColumn.IsFrozen) || 
                (newDisplayIndex > targetColumn.DisplayIndex && !targetColumn.IsFrozen))
            {
                return; 
            } 

            int colCount = this.ColumnsItemsInternal.Count, displayIndex; 

            if (newDisplayIndex < targetColumn.DisplayIndex)
            { 
                // DisplayIndex decreases.
                // Throw an exception if the visible unfrozen column is placed before a frozen column
                // Get the closest visible column placed after the displaced column 
                DataGridColumnBase nextColumn; 
                displayIndex = newDisplayIndex;
                do 
                {
                    nextColumn = this.ColumnsInternal.GetColumnAtDisplayIndex(displayIndex);
                    displayIndex++; 
                }
                while (displayIndex < colCount && (nextColumn == null || nextColumn == targetColumn || nextColumn.Visibility == Visibility.Collapsed));
 
                if (nextColumn != null && nextColumn.IsFrozen) 
                {
                    throw DataGridError.DataGrid.CannotMoveNonFrozenColumn(); 
                }
            }
            else 
            {
                // DisplayIndex increases.
                // Throw an exception if the visible frozen column is placed after a non-frozen column 
                // Get the closest visible column placed before the displaced column 
                DataGridColumnBase previousColumn;
                displayIndex = newDisplayIndex; 
                do
                {
                    previousColumn = this.ColumnsInternal.GetColumnAtDisplayIndex(displayIndex); 
                    displayIndex--;
                }
                while (displayIndex >= 0 && (previousColumn == null || previousColumn.Visibility == Visibility.Collapsed)); 
 
                if (previousColumn != null && !previousColumn.IsFrozen)
                { 
                    throw DataGridError.DataGrid.CannotMoveFrozenColumn();
                }
            } 
        }

        private void CorrectColumnIndexesAfterDeletion(DataGridColumnBase deletedColumn) 
        { 
            Debug.Assert(deletedColumn != null);
            for (int columnIndex = deletedColumn.Index; columnIndex < this.ColumnsItemsInternal.Count; columnIndex++) 
            {
                this.ColumnsItemsInternal[columnIndex].Index = this.ColumnsItemsInternal[columnIndex].Index - 1;
                Debug.Assert(this.ColumnsItemsInternal[columnIndex].Index == columnIndex); 
            }
        }
 
        private void CorrectColumnIndexesAfterInsertion(DataGridColumnBase insertedColumn, int insertionCount) 
        {
            Debug.Assert(insertedColumn != null); 
            Debug.Assert(insertionCount > 0);
            for (int columnIndex = insertedColumn.Index + insertionCount; columnIndex < this.ColumnsItemsInternal.Count; columnIndex++)
            { 
                this.ColumnsItemsInternal[columnIndex].Index = columnIndex;
            }
        } 
 
        private void FlushDisplayIndexChanged(bool raiseEvent)
        { 
            foreach (DataGridColumnBase column in this.ColumnsInternal)
            {
                if (column.DisplayIndexHasChanged) 
                {
                    column.DisplayIndexHasChanged = false;
                    if (raiseEvent) 
                    { 
                        OnColumnDisplayIndexChanged(column);
                    } 
                }
            }
        } 

        private bool GenerateColumn(Type type, string memberBinding, string memberName, string header, PropertyInfo propertyInfo)
        { 
            // Create a new DataBoundColumn for the Property 

            DataGridBoundColumnBase newColumn = GetDataGridColumnFromType(type); 
            newColumn.DisplayMemberBinding = new Binding(memberBinding);
            newColumn.DisplayMemberPath = memberName;
            newColumn.Header = header; 

            // Raise the AutoGeneratingColumn event in case the user wants to Cancel or Replace the
            // column being generated 
            DataGridAutoGeneratingColumnEventArgs e = new DataGridAutoGeneratingColumnEventArgs(propertyInfo, newColumn); 
            OnAutoGeneratingColumn(e);
            if (e.Cancel) 
            {
                return false;
            } 
            else
            {
                newColumn = e.Column; 
            } 

            this.ColumnsInternal.Add(newColumn); 

            return true;
        } 

        private bool GetColumnEffectiveReadOnlyState(DataGridColumnBase dataGridColumn)
        { 
            Debug.Assert(dataGridColumn != null); 

            return this.IsReadOnly || dataGridColumn.IsReadOnly; 
        }

        /// <devdoc> 
        ///      Returns the absolute coordinate of the left edge of the given column (including
        ///      the potential gridline - that is the left edge of the gridline is returned). Note that
        ///      the column does not need to be in the display area. 
        /// </devdoc> 
        private double GetColumnXFromIndex(int index)
        { 
            Debug.Assert(index < this.Columns.Count);
            Debug.Assert(this.Columns[index].Visibility == Visibility.Visible);
 
            double x = (double)this._cells.GetValue(Canvas.LeftProperty);

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleFrozenColumn; 
            while (dataGridColumn != null) 
            {
                if (index == dataGridColumn.Index) 
                {
                    return x;
                } 
                x += GetEdgedColumnWidth(dataGridColumn);
                dataGridColumn = this.ColumnsInternal.GetNextVisibleFrozenColumn(dataGridColumn);
            } 
 
            double xFirstVisibleScrollingCol = x;
 
            dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn;

            Debug.Assert(dataGridColumn.Visibility == Visibility.Visible && !dataGridColumn.IsFrozen); 

            while (dataGridColumn != null)
            { 
                if (index == dataGridColumn.Index) 
                {
                    return x; 
                }
                x += GetEdgedColumnWidth(dataGridColumn);
                dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn); 
            }

            // The column is completely hidden on the left/right of the dataGrid 
            x = xFirstVisibleScrollingCol; 
            dataGridColumn = this.Columns[this.DisplayData.FirstDisplayedScrollingCol];
            dataGridColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(dataGridColumn); 
            while (dataGridColumn != null)
            {
                x -= GetEdgedColumnWidth(dataGridColumn); 
                if (index == dataGridColumn.Index)
                {
                    return x; 
                } 
                dataGridColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(dataGridColumn);
            } 

            Debug.WriteLine("Could not find column in GetColumnXFromIndex");
            return 0; 
        }

        private static DataGridBoundColumnBase GetDataGridColumnFromType(Type type) 
        { 
            Debug.Assert(type != null);
            // 


 

            if (type == typeof(bool))
            { 
                return new DataGridCheckBoxColumn { Type = type }; 
            }
            else if (type == typeof(bool?)) 
            {
                DataGridCheckBoxColumn column = new DataGridCheckBoxColumn { Type = type };
                column.IsThreeState = true; 
                return column;
            }
            return new DataGridTextBoxColumn { Type = type }; 
        } 

        private double GetNegHorizontalOffsetFromHorizontalOffset(double horizontalOffset) 
        {
            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn;
            while (dataGridColumn != null && GetEdgedColumnWidth(dataGridColumn) <= horizontalOffset) 
            {
                horizontalOffset -= GetEdgedColumnWidth(dataGridColumn);
                dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn); 
            } 
            return horizontalOffset;
        } 

        private void InsertDisplayedColumnHeader(DataGridColumnBase dataGridColumn)
        { 
            Debug.Assert(dataGridColumn != null);
            if (this.AreColumnHeadersVisible && _columnHeaders != null)
            { 
                dataGridColumn.HeaderCell.Visibility = dataGridColumn.Visibility; 
                Debug.Assert(!this._columnHeaders.Children.Contains(dataGridColumn.HeaderCell));
                _columnHeaders.Children.Insert(dataGridColumn.DisplayIndex, dataGridColumn.HeaderCell); 
            }
        }
 
        private bool ScrollColumnIntoView(int columnIndex)
        {
            Debug.Assert(columnIndex >= 0 && columnIndex < this.Columns.Count); 
 
            if (this.DisplayData.FirstDisplayedScrollingCol != -1 &&
                !this.Columns[columnIndex].IsFrozen && 
                (columnIndex != this.DisplayData.FirstDisplayedScrollingCol || this._negHorizontalOffset > 0))
            {
                int columnsToScroll; 
                if (this.ColumnsInternal.DisplayInOrder(columnIndex, this.DisplayData.FirstDisplayedScrollingCol))
                {
                    columnsToScroll = this.ColumnsInternal.GetColumnCount(true /* isVisible */, false /* isFrozen */, columnIndex, this.DisplayData.FirstDisplayedScrollingCol); 
                    if (this._negHorizontalOffset > 0) 
                    {
                        columnsToScroll++; 
                    }
                    ScrollColumns(-columnsToScroll);
                } 
                else if (columnIndex == this.DisplayData.FirstDisplayedScrollingCol && this._negHorizontalOffset > 0)
                {
                    ScrollColumns(-1); 
                } 
                else if (this.DisplayData.LastTotallyDisplayedScrollingCol == -1 ||
                         (this.DisplayData.LastTotallyDisplayedScrollingCol != columnIndex && 
                          this.ColumnsInternal.DisplayInOrder(this.DisplayData.LastTotallyDisplayedScrollingCol, columnIndex)))
                {
                    double xColumnLeftEdge = GetColumnXFromIndex(columnIndex); 
                    double xColumnRightEdge = xColumnLeftEdge + GetEdgedColumnWidth(this.Columns[columnIndex]);
                    double change = xColumnRightEdge - this.HorizontalOffset - this.CellsWidth;
                    double widthRemaining = change; 
 
                    DataGridColumnBase newFirstDisplayedScrollingCol = this.Columns[this.DisplayData.FirstDisplayedScrollingCol];
                    DataGridColumnBase nextColumn = this.ColumnsInternal.GetNextVisibleColumn(newFirstDisplayedScrollingCol); 
                    double newColumnWidth = GetEdgedColumnWidth(newFirstDisplayedScrollingCol) - this._negHorizontalOffset;
                    while (nextColumn != null && widthRemaining >= newColumnWidth)
                    { 
                        this.DisplayData.NumDisplayedScrollingCols--;
                        newFirstDisplayedScrollingCol = nextColumn;
                        newColumnWidth = GetEdgedColumnWidth(newFirstDisplayedScrollingCol); 
                        nextColumn = this.ColumnsInternal.GetNextVisibleColumn(newFirstDisplayedScrollingCol); 
                        this._negHorizontalOffset = 0;
                    } 
                    this._negHorizontalOffset += widthRemaining;
                    this.DisplayData.LastTotallyDisplayedScrollingCol = columnIndex;
                    if (newFirstDisplayedScrollingCol.Index == columnIndex) 
                    {
                        this._negHorizontalOffset = 0;
                        // If the entire column cannot be displayed, we want to start showing it from its LeftEdge 
                        if (newColumnWidth > this.CellsWidth) 
                        {
                            this.DisplayData.LastTotallyDisplayedScrollingCol = -1; 
                            change = xColumnLeftEdge - this.HorizontalOffset;
                        }
                    } 
                    this.DisplayData.FirstDisplayedScrollingCol = newFirstDisplayedScrollingCol.Index;

                    // At this point DisplayData.FirstDisplayedScrollingColumn and LastDisplayedScrollingColumn 
                    // should be correct 
                    if (change != 0)
                    { 
                        UpdateHorizontalOffset(this.HorizontalOffset + change);
                    }
                } 
            }
            return true;
        } 
 
        private void ScrollColumns(int columns)
        { 
            DataGridColumnBase newFirstVisibleScrollingCol = null;
            DataGridColumnBase dataGridColumnTmp;
            int colCount = 0; 
            if (columns > 0)
            {
                if (this.DisplayData.LastTotallyDisplayedScrollingCol >= 0) 
                { 
                    dataGridColumnTmp = this.Columns[this.DisplayData.LastTotallyDisplayedScrollingCol];
                    while (colCount < columns && dataGridColumnTmp != null) 
                    {
                        dataGridColumnTmp = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumnTmp);
                        colCount++; 
                    }

                    if (dataGridColumnTmp == null) 
                    { 
                        // no more column to display on the right of the last totally seen column
                        return; 
                    }
                }
                Debug.Assert(this.DisplayData.FirstDisplayedScrollingCol >= 0); 
                dataGridColumnTmp = this.Columns[this.DisplayData.FirstDisplayedScrollingCol];
                colCount = 0;
                while (colCount < columns && dataGridColumnTmp != null) 
                { 
                    dataGridColumnTmp = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumnTmp);
                    colCount++; 
                }
                newFirstVisibleScrollingCol = dataGridColumnTmp;
            } 

            if (columns < 0)
            { 
                Debug.Assert(this.DisplayData.FirstDisplayedScrollingCol >= 0); 
                dataGridColumnTmp = this.Columns[this.DisplayData.FirstDisplayedScrollingCol];
                if (this._negHorizontalOffset > 0) 
                {
                    colCount++;
                } 
                while (colCount < -columns && dataGridColumnTmp != null)
                {
                    dataGridColumnTmp = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(dataGridColumnTmp); 
                    colCount++; 
                }
                newFirstVisibleScrollingCol = dataGridColumnTmp; 
                if (newFirstVisibleScrollingCol == null)
                {
                    if (this._negHorizontalOffset == 0) 
                    {
                        // no more column to display on the left of the first seen column
                        this.DisplayData.Dirty = false; 
                        return; 
                    }
                    else 
                    {
                        newFirstVisibleScrollingCol = this.Columns[this.DisplayData.FirstDisplayedScrollingCol];
                    } 
                }
            }
 
            double newColOffset = 0; 
            for (DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleScrollingColumn;
                dataGridColumn != newFirstVisibleScrollingCol; 
                dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(dataGridColumn))
            {
                newColOffset += GetEdgedColumnWidth(dataGridColumn); 
            }

            UpdateHorizontalOffset(newColOffset); 
        } 

        private static void UpdateCellElement(DataGridColumnBase dataGridColumn, DataGridRow dataGridRow, string propertyName) 
        {
            Debug.Assert(dataGridColumn != null);
            Debug.Assert(dataGridRow != null); 

            DataGridCell dataGridCell = dataGridRow.Cells[dataGridColumn.Index];
            Debug.Assert(dataGridCell != null); 
            FrameworkElement element = dataGridCell.Content as FrameworkElement; 
            if (element != null)
            { 
                dataGridColumn.UpdateElement(element, propertyName);
            }
        } 

        private void UpdateColumnHeadersSeparatorVisibility()
        { 
            foreach (DataGridColumnBase dataGridColumn in this.ColumnsItemsInternal) 
            {
                if (dataGridColumn.HasHeaderCell) 
                {
                    dataGridColumn.HeaderCell.UpdateSeparatorVisibility();
                } 
            }
            if (this.ColumnsInternal.FillerColumn.HasHeaderCell)
            { 
                this.ColumnsInternal.FillerColumn.HeaderCell.SeparatorVisibility = Visibility.Collapsed; 
            }
        } 

        #endregion Private Methods
    } 
}
