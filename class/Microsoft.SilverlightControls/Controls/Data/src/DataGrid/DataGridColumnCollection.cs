// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections.Generic;
using System.Collections.ObjectModel; 
using System.Diagnostics; 

namespace System.Windows.Controlsb1
{
    internal class DataGridColumnCollection : ObservableCollection<DataGridColumnBase>
    { 
        #region Data

        private int _lastAccessedSortedIndex = -1; 
        private DataGrid _owningGrid; 
        private List<DataGridColumnBase> _sortedColumns;
 
        #endregion Data

        public DataGridColumnCollection(DataGrid owningGrid) 
        {
            this._owningGrid = owningGrid;
            this.ItemsInternal = new List<DataGridColumnBase>(); 
            this.FillerColumn = new DataGridFillerColumn(owningGrid); 
        }
 
        #region Internal Properties

        internal DataGridFillerColumn FillerColumn 
        {
            get;
            private set; 
        } 

        internal DataGridColumnBase FirstColumn 
        {
            get
            { 
                return GetFirstColumn(null /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/);
            }
        } 
 
        internal DataGridColumnBase FirstVisibleColumn
        { 
            get
            {
                return GetFirstColumn(true /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/); 
            }
        }
 
        internal DataGridColumnBase FirstVisibleFrozenColumn 
        {
            get 
            {
                return GetFirstColumn(true /*isVisible*/, true /*isFrozen*/, null /*isReadOnly*/);
            } 
        }

        internal DataGridColumnBase FirstVisibleWritableColumn 
        { 
            get
            { 
                return GetFirstColumn(true /*isVisible*/, null /*isFrozen*/, false /*isReadOnly*/);
            }
        } 

        internal DataGridColumnBase FirstVisibleScrollingColumn
        { 
            get 
            {
                return GetFirstColumn(true /*isVisible*/, false /*isFrozen*/, null /*isReadOnly*/); 
            }
        }
 
        internal List<DataGridColumnBase> ItemsInternal
        {
            get; 
            private set; 
        }
 
        internal DataGridColumnBase LastVisibleColumn
        {
            get 
            {
                return GetLastColumn(true /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/);
            } 
        } 

        internal DataGridColumnBase LastVisibleFrozenColumn 
        {
            get
            { 
                return GetLastColumn(true /*isVisible*/, true /*isFrozen*/, null /*isReadOnly*/);
            }
        } 
 
        internal DataGridColumnBase LastVisibleScrollingColumn
        { 
            get
            {
                return GetLastColumn(true /*isVisible*/, false /*isFrozen*/, null /*isReadOnly*/); 
            }
        }
 
        internal DataGridColumnBase LastVisibleWritableColumn 
        {
            get 
            {
                return GetLastColumn(true /*isVisible*/, null /*isFrozen*/, false /*isReadOnly*/);
            } 
        }

        internal int VisibleColumnCount 
        { 
            get
            { 
                //

                int visibleColumnCount = 0; 
                for (int columnIndex = 0; columnIndex < this.ItemsInternal.Count; columnIndex++)
                {
                    if (this.ItemsInternal[columnIndex].Visibility == Visibility.Visible) 
                    { 
                        visibleColumnCount++;
                    } 
                }
                return visibleColumnCount;
            } 
        }

        #endregion Internal Properties 
 
        #region Protected Methods
 
        protected override void ClearItems()
        {
            Debug.Assert(this._owningGrid != null); 
            if (this.ItemsInternal.Count > 0)
            {
                // 
 

 
                if (this._owningGrid.InDisplayIndexAdjustments)
                {
                    // We are within columns display indexes adjustments. We do not allow changing the column collection while adjusting display indexes. 
                    throw DataGridError.DataGrid.CannotChangeColumnCollectionWhileAdjustingDisplayIndexes();
                }
 
                this._owningGrid.OnClearingColumns(); 
                InvalidateCachedColumnsOrder();
                for (int columnIndex = 0; columnIndex < this.ItemsInternal.Count; columnIndex++) 
                {
                    // Detach the column...
                    this.ItemsInternal[columnIndex].OwningGrid = null; 
                }
                this.ItemsInternal.Clear();
                // 
 
                this._owningGrid.OnColumnCollectionChanged_PreNotification(false /*columnsGrew*/);
                base.ClearItems(); 
                this._owningGrid.OnColumnCollectionChanged_PostNotification(false /*columnsGrew*/);
            }
        } 

        protected override void InsertItem(int columnIndex, DataGridColumnBase dataGridColumn)
        { 
            Debug.Assert(this._owningGrid != null); 
            //
 


            if (this._owningGrid.InDisplayIndexAdjustments) 
            {
                // We are within columns display indexes adjustments. We do not allow changing the column collection while adjusting display indexes.
                throw DataGridError.DataGrid.CannotChangeColumnCollectionWhileAdjustingDisplayIndexes(); 
            } 
            if (dataGridColumn == null)
            { 
                throw new ArgumentNullException("dataGridColumn");
            }
            int originalDisplayIndex = dataGridColumn.DisplayIndex; 
            if (originalDisplayIndex == -1)
            {
                dataGridColumn.DisplayIndex = columnIndex; 
            } 
            DataGridCellCoordinates newCurrentCellCoordinates;
            try 
            {
                newCurrentCellCoordinates = this._owningGrid.OnInsertingColumn(columnIndex, dataGridColumn);   // will throw an exception if the insertion is illegal
            } 
            finally
            {
                dataGridColumn.DisplayIndexInternal = originalDisplayIndex; 
            } 
            InvalidateCachedColumnsOrder();
            this.ItemsInternal.Insert(columnIndex, dataGridColumn); 
            dataGridColumn.Index = columnIndex;
            dataGridColumn.OwningGrid = this._owningGrid;
            this._owningGrid.OnInsertedColumn_PreNotification(dataGridColumn); 
            this._owningGrid.OnColumnCollectionChanged_PreNotification(true /*columnsGrew*/);
            base.InsertItem(columnIndex, dataGridColumn);
            this._owningGrid.OnInsertedColumn_PostNotification(newCurrentCellCoordinates); 
            this._owningGrid.OnColumnCollectionChanged_PostNotification(true /*columnsGrew*/); 
        }
 
        protected override void RemoveItem(int columnIndex)
        {
            // 


 
 
            if (this._owningGrid.InDisplayIndexAdjustments)
            { 
                // We are within columns display indexes adjustments. We do not allow changing the column collection while adjusting display indexes.
                throw DataGridError.DataGrid.CannotChangeColumnCollectionWhileAdjustingDisplayIndexes();
            } 

            Debug.Assert(columnIndex >= 0 && columnIndex < this.ItemsInternal.Count);
            Debug.Assert(this._owningGrid != null); 
 
            DataGridColumnBase dataGridColumn = this.ItemsInternal[columnIndex];
            // 


 
            DataGridCellCoordinates newCurrentCellCoordinates = this._owningGrid.OnRemovingColumn(dataGridColumn);
            InvalidateCachedColumnsOrder();
            this.ItemsInternal.RemoveAt(columnIndex); 
            dataGridColumn.OwningGrid = null; 
            this._owningGrid.OnRemovedColumn_PreNotification(dataGridColumn);
            this._owningGrid.OnColumnCollectionChanged_PreNotification(false /*columnsGrew*/); 
            base.RemoveItem(columnIndex);
            this._owningGrid.OnRemovedColumn_PostNotification(newCurrentCellCoordinates);
            this._owningGrid.OnColumnCollectionChanged_PostNotification(false /*columnsGrew*/); 
        }

        protected override void SetItem(int columnIndex, DataGridColumnBase dataGridColumn) 
        { 
            throw new NotSupportedException();
        } 

        #endregion Protected Methods
 
        #region Internal Methods

        internal bool DisplayInOrder(int columnIndex1, int columnIndex2) 
        { 
            int displayIndex1 = ((DataGridColumnBase)this.ItemsInternal[columnIndex1]).DisplayIndex;
            int displayIndex2 = ((DataGridColumnBase)this.ItemsInternal[columnIndex2]).DisplayIndex; 
            return displayIndex1 < displayIndex2;
        }
 
        internal DataGridColumnBase GetColumnAtDisplayIndex(int displayIndex)
        {
            if (displayIndex < 0 || displayIndex >= this.ItemsInternal.Count) 
            { 
                return null;
            } 
            if (this.ItemsInternal[displayIndex].DisplayIndex == displayIndex)
            {
                // Performance gain if display indexes coincide with indexes. 
                return this.ItemsInternal[displayIndex];
            }
            for (int columnIndex = 0; columnIndex < this.ItemsInternal.Count; columnIndex++) 
            { 
                if (this.ItemsInternal[columnIndex].DisplayIndex == displayIndex)
                { 
                    return this.ItemsInternal[columnIndex];
                }
            } 
            Debug.Assert(false, "no column found in GetColumnAtDisplayIndex");
            return null;
        } 
 
        internal int GetColumnCount(bool isVisible, bool isFrozen, int fromColumnIndex, int toColumnIndex)
        { 
            Debug.Assert(DisplayInOrder(fromColumnIndex, toColumnIndex));
            Debug.Assert((this.ItemsInternal[toColumnIndex].Visibility == Visibility.Visible) == isVisible);
            Debug.Assert(this.ItemsInternal[toColumnIndex].IsFrozen == isFrozen); 

            int columnCount = 0;
            DataGridColumnBase dataGridColumn = this.ItemsInternal[fromColumnIndex]; 
 
            while (dataGridColumn != this.ItemsInternal[toColumnIndex])
            { 
                dataGridColumn = GetNextColumn(dataGridColumn, isVisible, isFrozen, null /*isReadOnly*/);
                Debug.Assert(dataGridColumn != null);
                columnCount++; 
            }
            return columnCount;
        } 
 
        internal DataGridColumnBase GetFirstColumn(bool? isVisible, bool? isFrozen, bool? isReadOnly)
        { 
            if (this._sortedColumns == null)
            {
                UpdateColumnOrderCache(); 
            }
#if DEBUG
            Debug.Assert(Debug_VerifyColumnOrderCache()); 
#endif 
            int index = 0;
            while (index < this._sortedColumns.Count) 
            {
                DataGridColumnBase dataGridColumn = this._sortedColumns[index];
                if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) && 
                    (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) &&
                    (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly))
                { 
                    this._lastAccessedSortedIndex = index; 
                    return dataGridColumn;
                } 
                index++;
            }
            return null; 
        }

        internal DataGridColumnBase GetLastColumn(bool? isVisible, bool? isFrozen, bool? isReadOnly) 
        { 
            if (this._sortedColumns == null)
            { 
                UpdateColumnOrderCache();
            }
#if DEBUG 
            Debug.Assert(Debug_VerifyColumnOrderCache());
#endif
            int index = this._sortedColumns.Count - 1; 
            while (index >= 0) 
            {
                DataGridColumnBase dataGridColumn = this._sortedColumns[index]; 
                if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) &&
                    (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) &&
                    (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly)) 
                {
                    this._lastAccessedSortedIndex = index;
                    return dataGridColumn; 
                } 
                index--;
            } 
            return null;
        }
 
        internal DataGridColumnBase GetNextColumn(DataGridColumnBase dataGridColumnStart)
        {
            return GetNextColumn(dataGridColumnStart, null /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/); 
        } 

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")] 
        internal DataGridColumnBase GetNextColumn(DataGridColumnBase dataGridColumnStart,
                                                  bool? isVisible, bool? isFrozen, bool? isReadOnly)
        { 
            Debug.Assert(dataGridColumnStart != null);
            Debug.Assert(this.ItemsInternal.Contains(dataGridColumnStart));
 
            if (this._sortedColumns == null) 
            {
                UpdateColumnOrderCache(); 
            }
#if DEBUG
            Debug.Assert(Debug_VerifyColumnOrderCache()); 
#endif
            int index = GetColumnSortedIndex(dataGridColumnStart);
            if (index == -1) 
            { 
                bool columnFound = false;
                int indexMin = Int32.MaxValue, displayIndexMin = Int32.MaxValue; 
                for (index = 0; index < this.ItemsInternal.Count; index++)
                {
                    DataGridColumnBase dataGridColumn = this.ItemsInternal[index]; 
                    if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) &&
                        (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) &&
                        (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly) && 
                        (dataGridColumn.DisplayIndex > dataGridColumnStart.DisplayIndex || 
                         (dataGridColumn.DisplayIndex == dataGridColumnStart.DisplayIndex &&
                          dataGridColumn.Index > dataGridColumnStart.Index))) 
                    {
                        if (dataGridColumn.DisplayIndex < displayIndexMin ||
                            (dataGridColumn.DisplayIndex == displayIndexMin && 
                             dataGridColumn.Index < indexMin))
                        {
                            indexMin = index; 
                            displayIndexMin = dataGridColumn.DisplayIndex; 
                            columnFound = true;
                        } 
                    }
                }
                return columnFound ? this.ItemsInternal[indexMin] : null; 
            }
            else
            { 
                index++; 
                while (index < this._sortedColumns.Count)
                { 
                    DataGridColumnBase dataGridColumn = this._sortedColumns[index];

                    if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) && 
                        (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) &&
                        (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly))
                    { 
                        this._lastAccessedSortedIndex = index; 
                        return dataGridColumn;
                    } 
                    index++;
                }
            } 
            return null;
        }
 
        internal DataGridColumnBase GetNextVisibleColumn(DataGridColumnBase dataGridColumnStart) 
        {
            return GetNextColumn(dataGridColumnStart, true /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/); 
        }

        internal DataGridColumnBase GetNextVisibleFrozenColumn(DataGridColumnBase dataGridColumnStart) 
        {
            return GetNextColumn(dataGridColumnStart, true /*isVisible*/, true /*isFrozen*/, null /*isReadOnly*/);
        } 
 
        internal DataGridColumnBase GetNextVisibleScrollingColumn(DataGridColumnBase dataGridColumnStart)
        { 
            return GetNextColumn(dataGridColumnStart, true /*isVisible*/, false /*isFrozen*/, null /*isReadOnly*/);
        }
 
        internal DataGridColumnBase GetNextVisibleWritableColumn(DataGridColumnBase dataGridColumnStart)
        {
            return GetNextColumn(dataGridColumnStart, true /*isVisible*/, null /*isFrozen*/, false /*isReadOnly*/); 
        } 

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")] 
        internal DataGridColumnBase GetPreviousColumn(DataGridColumnBase dataGridColumnStart,
                                                      bool? isVisible, bool? isFrozen, bool? isReadOnly)
        { 
            Debug.Assert(dataGridColumnStart != null);
            Debug.Assert(this.ItemsInternal.Contains(dataGridColumnStart));
 
            if (this._sortedColumns == null) 
            {
                UpdateColumnOrderCache(); 
            }
#if DEBUG
            Debug.Assert(Debug_VerifyColumnOrderCache()); 
#endif
            int index = GetColumnSortedIndex(dataGridColumnStart);
            if (index == -1) 
            { 
                bool columnFound = false;
                int indexMax = -1, displayIndexMax = -1; 
                for (index = 0; index < this.ItemsInternal.Count; index++)
                {
                    DataGridColumnBase dataGridColumn = this.ItemsInternal[index]; 
                    if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) &&
                        (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) &&
                        (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly) && 
                        (dataGridColumn.DisplayIndex < dataGridColumnStart.DisplayIndex || 
                         (dataGridColumn.DisplayIndex == dataGridColumnStart.DisplayIndex &&
                          dataGridColumn.Index < dataGridColumnStart.Index))) 
                    {
                        if (dataGridColumn.DisplayIndex > displayIndexMax ||
                            (dataGridColumn.DisplayIndex == displayIndexMax && 
                             dataGridColumn.Index > indexMax))
                        {
                            indexMax = index; 
                            displayIndexMax = dataGridColumn.DisplayIndex; 
                            columnFound = true;
                        } 
                    }
                }
                return columnFound ? this.ItemsInternal[indexMax] : null; 
            }
            else
            { 
                index--; 
                while (index >= 0)
                { 
                    DataGridColumnBase dataGridColumn = this._sortedColumns[index];
                    if ((isVisible == null || (dataGridColumn.Visibility == Visibility.Visible) == isVisible) &&
                        (isFrozen == null || dataGridColumn.IsFrozen == isFrozen) && 
                        (isReadOnly == null || dataGridColumn.IsReadOnly == isReadOnly))
                    {
                        this._lastAccessedSortedIndex = index; 
                        return dataGridColumn; 
                    }
                    index--; 
                }
            }
            return null; 
        }

        internal DataGridColumnBase GetPreviousVisibleColumn(DataGridColumnBase dataGridColumnStart) 
        { 
            return GetPreviousColumn(dataGridColumnStart, true /*isVisible*/, null /*isFrozen*/, null /*isReadOnly*/);
        } 

        internal DataGridColumnBase GetPreviousVisibleFrozenColumn(DataGridColumnBase dataGridColumnStart)
        { 
            return GetPreviousColumn(dataGridColumnStart, true /*isVisible*/, true /*isFrozen*/, null /*isReadOnly*/);
        }
 
        internal DataGridColumnBase GetPreviousVisibleScrollingColumn(DataGridColumnBase dataGridColumnStart) 
        {
            return GetPreviousColumn(dataGridColumnStart, true /*isVisible*/, false /*isFrozen*/, null /*isReadOnly*/); 
        }

        internal DataGridColumnBase GetPreviousVisibleWritableColumn(DataGridColumnBase dataGridColumnStart) 
        {
            return GetPreviousColumn(dataGridColumnStart, true /*isVisible*/, null /*isFrozen*/, false /*isReadOnly*/);
        } 
 
        internal int GetVisibleColumnCount(int fromColumnIndex, int toColumnIndex)
        { 
            Debug.Assert(DisplayInOrder(fromColumnIndex, toColumnIndex));
            Debug.Assert(this.ItemsInternal[toColumnIndex].Visibility == Visibility.Visible);
 
            int columnCount = 0;
            DataGridColumnBase dataGridColumn = this.ItemsInternal[fromColumnIndex];
 
            while (dataGridColumn != this.ItemsInternal[toColumnIndex]) 
            {
                dataGridColumn = GetNextVisibleColumn(dataGridColumn); 
                Debug.Assert(dataGridColumn != null);
                columnCount++;
            } 
            return columnCount;
        }
 
        internal double GetVisibleEdgedColumnsWidth(bool includeLastRightGridlineWhenPresent) 
        {
            // 

            double visibleColumnsWidth = 0;
            for (int columnIndex = 0; columnIndex < this.ItemsInternal.Count; columnIndex++) 
            {
                if (this.ItemsInternal[columnIndex].Visibility == Visibility.Visible)
                { 
                    visibleColumnsWidth += this._owningGrid.GetEdgedColumnWidth(this.ItemsInternal[columnIndex], includeLastRightGridlineWhenPresent); 
                }
            } 
            return visibleColumnsWidth;
        }
 
        internal double GetVisibleFrozenEdgedColumnsWidth(bool includeLastRightGridlineWhenPresent)
        {
            // 
 
            double visibleFrozenColumnsWidth = 0;
            for (int columnIndex = 0; columnIndex < this.ItemsInternal.Count; columnIndex++) 
            {
                if (this.ItemsInternal[columnIndex].Visibility == Visibility.Visible && this.ItemsInternal[columnIndex].IsFrozen)
                { 
                    visibleFrozenColumnsWidth += this._owningGrid.GetEdgedColumnWidth(this.ItemsInternal[columnIndex], includeLastRightGridlineWhenPresent);
                }
            } 
            return visibleFrozenColumnsWidth; 
        }
 
        internal void InvalidateCachedColumnsOrder()
        {
            this._sortedColumns = null; 
        }

        #endregion Internal Methods 
 
        #region Private Methods
 
        private int GetColumnSortedIndex(DataGridColumnBase dataGridColumn)
        {
            Debug.Assert(dataGridColumn != null); 
            Debug.Assert(this._sortedColumns != null);
            Debug.Assert(this._lastAccessedSortedIndex == -1 || this._lastAccessedSortedIndex < this.ItemsInternal.Count);
 
            if (this._lastAccessedSortedIndex != -1 && 
                this._sortedColumns[this._lastAccessedSortedIndex] == dataGridColumn)
            { 
                return this._lastAccessedSortedIndex;
            }
 
            int index = 0;
            while (index < this._sortedColumns.Count)
            { 
                if (dataGridColumn.Index == this._sortedColumns[index].Index) 
                {
                    this._lastAccessedSortedIndex = index; 
                    return index;
                }
                index++; 
            }
            return -1;
        } 
 
        private void UpdateColumnOrderCache()
        { 
            this._sortedColumns = new List<DataGridColumnBase>(this.ItemsInternal);
            this._sortedColumns.Sort(Comparer<DataGridColumnBase>.Default);
            this._lastAccessedSortedIndex = -1; 
        }

        #endregion Private Methods 
 
        #region Debugging Methods
 
#if DEBUG
        internal bool Debug_VerifyColumnDisplayIndexes()
        { 
            for (int columnDisplayIndex = 0; columnDisplayIndex < this.ItemsInternal.Count; columnDisplayIndex++)
            {
                if (GetColumnAtDisplayIndex(columnDisplayIndex) == null) 
                { 
                    return false;
                } 
            }
            return true;
        } 

        private bool Debug_VerifyColumnOrderCache()
        { 
            if (this._sortedColumns == null) return false; 
            if (this._sortedColumns.Count != this.ItemsInternal.Count) return false;
 
            int index = 0;
            while (index < this._sortedColumns.Count - 1)
            { 
                if (this._sortedColumns[index + 1].DisplayIndex !=
                    this._sortedColumns[index].DisplayIndex + 1) return false;
                index++; 
            } 
            return true;
        } 
#endif

        #endregion Debugging Methods 
    }
}
