// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections;
using System.Diagnostics; 
 
namespace System.Windows.Controlsb1
{ 
    internal class DataGridSelectedItemsCollection : IList
    {
        #region Data 

        private IndexToValueTable<bool> _selectedItemsTable;
 
        #endregion Data 

        public DataGridSelectedItemsCollection(DataGrid owningGrid) 
        {
            this.OwningGrid = owningGrid;
            this._selectedItemsTable = new IndexToValueTable<bool>(); 
        }

        #region IList Implementation 
 
        public object this[int index]
        { 
            get
            {
                if (index < 0 || index >= this._selectedItemsTable.IndexCount) 
                {
                    throw DataGridError.DataGrid.ValueMustBeBetween("index", "Index", 0, true, this._selectedItemsTable.IndexCount, false);
                } 
                int rowIndex = this._selectedItemsTable.IndexOf(index); 
                Debug.Assert(rowIndex > -1);
                return this.OwningGrid.DataConnection.GetDataItem(rowIndex); 
            }
            set
            { 
                throw new NotSupportedException();
            }
        } 
 
        public bool IsFixedSize
        { 
            get
            {
                return false; 
            }
        }
 
        public bool IsReadOnly 
        {
            get 
            {
                return false;
            } 
        }

        public int Add(object dataItem) 
        { 
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.SingleFullRow)
            { 
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            }
 
            DataGridRow dataGridRow = this.OwningGrid.GetRowFromItem(dataItem);
            if (dataGridRow == null)
            { 
                throw DataGridError.DataGrid.ItemIsNotContainedInTheItemsSource("dataItem"); 
            }
            Debug.Assert(dataGridRow.Index > -1); 

            if (this._selectedItemsTable.RangeCount == 0)
            { 
                this.OwningGrid.SelectedItem = dataItem;
            }
            else 
            { 
                this.OwningGrid.SetRowSelection(dataGridRow.Index, true /*isSelected*/, false /*setAnchorRowIndex*/);
            } 
            int cumulatedSelectedRows = 0;
            foreach (Range<bool> selectedRange in this._selectedItemsTable)
            { 
                if (selectedRange.UpperBound >= dataGridRow.Index)
                {
                    cumulatedSelectedRows += dataGridRow.Index - selectedRange.LowerBound; 
                } 
                else
                { 
                    cumulatedSelectedRows += selectedRange.Count;
                }
            } 
            return cumulatedSelectedRows;
        }
 
        public void Clear() 
        {
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.SingleFullRow) 
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            } 

            if (this._selectedItemsTable.RangeCount > 0)
            { 
                // Clearing the selection does not reset the potential current cell. 
                if (!this.OwningGrid.EndEdit(true /*commitChanges*/, true /*exitEditing*/))
                { 
                    // Edited value couldn't be committed or aborted
                    throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                } 
                this.OwningGrid.ClearRowSelection(true /*resetAnchorRowIndex*/);
            }
        } 
 
        public bool Contains(object dataItem)
        { 
            DataGridRow dataGridRow = this.OwningGrid.GetRowFromItem(dataItem);
            if (dataGridRow == null)
            { 
                return false;
            }
            Debug.Assert(dataGridRow.Index > -1); 
 
            return Contains(dataGridRow.Index);
        } 

        public int IndexOf(object dataItem)
        { 
            DataGridRow dataGridRow = this.OwningGrid.GetRowFromItem(dataItem);
            if (dataGridRow == null)
            { 
                return -1; 
            }
            Debug.Assert(dataGridRow.Index > -1); 
            int cumulatedSelectedRows = 0;
            foreach (Range<bool> selectedRange in this._selectedItemsTable)
            { 
                if (selectedRange.UpperBound >= dataGridRow.Index)
                {
                    if (selectedRange.LowerBound <= dataGridRow.Index) 
                    { 
                        return cumulatedSelectedRows + dataGridRow.Index - selectedRange.LowerBound;
                    } 
                    else
                    {
                        return -1; 
                    }
                }
                else 
                { 
                    cumulatedSelectedRows += selectedRange.Count;
                } 
            }
            return -1;
        } 

        public void Insert(int index, object dataItem)
        { 
            throw new NotSupportedException(); 
        }
 
        public void Remove(object dataItem)
        {
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.SingleFullRow) 
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            } 
 
            DataGridRow dataGridRow = this.OwningGrid.GetRowFromItem(dataItem);
            if (dataGridRow == null) 
            {
                return;
            } 
            Debug.Assert(dataGridRow.Index > -1);

            if (dataGridRow.Index == this.OwningGrid.CurrentRowIndex && 
                !this.OwningGrid.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/)) 
            {
                // Edited value couldn't be committed or aborted 
                throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
            }
 
            //
            this.OwningGrid.SetRowSelection(dataGridRow.Index, false /*isSelected*/, false /*setAnchorRowIndex*/);
        } 
 
        public void RemoveAt(int index)
        { 
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.SingleFullRow)
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode(); 
            }

            if (index < 0 || index >= this._selectedItemsTable.IndexCount) 
            { 
                throw DataGridError.DataGrid.ValueMustBeBetween("index", "Index", 0, true, this._selectedItemsTable.IndexCount, false);
            } 
            int rowIndex = this._selectedItemsTable.IndexOf(index);
            Debug.Assert(rowIndex > -1);
 
            if (rowIndex == this.OwningGrid.CurrentRowIndex &&
                !this.OwningGrid.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/))
            { 
                // Edited value couldn't be committed or aborted 
                throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
            } 

            //
            this.OwningGrid.SetRowSelection(rowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/); 
        }

        #endregion IList Implementation 
 
        #region ICollection Implementation
 
        public int Count
        {
            get 
            {
                return this._selectedItemsTable.IndexCount;
            } 
        } 

        public bool IsSynchronized 
        {
            get
            { 
                return false;
            }
        } 
 
        public object SyncRoot
        { 
            get
            {
                return this; 
            }
        }
 
        public void CopyTo(System.Array array, int index) 
        {
            // 
            throw new NotImplementedException();
        }
 
        #endregion ICollection Implementation

        #region IEnumerable Implementation 
 
        public IEnumerator GetEnumerator()
        { 
            //

            return new SelectedItemsEnumerator(this.OwningGrid, this._selectedItemsTable); 
        }

        #endregion IEnumerable Implementation 
 
        #region Internal Properties
 
        internal DataGrid OwningGrid
        {
            get; 
            private set;
        }
 
        #endregion 

        #region Internal Methods 

        internal void ClearRows()
        { 
            this._selectedItemsTable.Clear();
        }
 
        internal bool Contains(int rowIndex) 
        {
            return this._selectedItemsTable.Contains(rowIndex); 
        }

        internal bool ContainsAll(int startRowIndex, int endRowIndex) 
        {
            return this._selectedItemsTable.ContainsAll(startRowIndex, endRowIndex);
        } 
 
        //
 


 

        internal IEnumerator GetIndexesEnumerator()
        { 
            return this._selectedItemsTable.GetIndexesEnumerator(); 
        }
 
        internal void InsertIndexes(int rowIndexInserted, int insertionCount)
        {
            this._selectedItemsTable.InsertIndexes(rowIndexInserted, insertionCount); 
        }

        internal void RemoveIndex(int rowIndex) 
        { 
            this._selectedItemsTable.RemoveIndex(rowIndex);
        } 

        internal void SelectRow(int rowIndex, bool select)
        { 
            if (select)
            {
                this._selectedItemsTable.AddValue(rowIndex, true); 
            } 
            else
            { 
                this._selectedItemsTable.RemoveValue(rowIndex);
            }
        } 

        internal void SelectRows(int startRowIndex, int rowCount, bool select)
        { 
            if (select) 
            {
                this._selectedItemsTable.AddValues(startRowIndex, rowCount, true); 
            }
            else
            { 
                this._selectedItemsTable.RemoveValues(startRowIndex, rowCount);
            }
        } 
 
        #endregion Internal Methods
 
        #region Private Methods

        #endregion 
    }

    // 
    internal class SelectedItemsEnumerator : IEnumerator 
    {
        private DataGrid OwningGrid; 
        private IEnumerator _rowIndexesEnumerator;

        public SelectedItemsEnumerator(DataGrid owningGrid, IndexToValueTable<bool> selectedItemsTable) 
        {
            Debug.Assert(owningGrid != null);
            Debug.Assert(selectedItemsTable != null); 
            this.OwningGrid = owningGrid; 
            this._rowIndexesEnumerator = selectedItemsTable.GetIndexesEnumerator();
        } 

        public object Current
        { 
            get
            {
                int rowIndex = (int)this._rowIndexesEnumerator.Current; 
                Debug.Assert(rowIndex > -1); 
                return this.OwningGrid.DataConnection.GetDataItem(rowIndex);
            } 
        }

        public bool MoveNext() 
        {
            return this._rowIndexesEnumerator.MoveNext();
        } 
 
        public void Reset()
        { 
            this._rowIndexesEnumerator.Reset();
        }
    } 
}
