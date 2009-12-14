// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace System.Windows.Controls
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
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.Single)
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            }

            int itemIndex = this.OwningGrid.DataConnection.IndexOf(dataItem);
            if (itemIndex == -1)
            {
                throw DataGridError.DataGrid.ItemIsNotContainedInTheItemsSource("dataItem");
            }
            Debug.Assert(itemIndex >= 0);

            if (this._selectedItemsTable.RangeCount == 0)
            {
                this.OwningGrid.SelectedItem = dataItem;
            }
            else
            {
                this.OwningGrid.SetRowSelection(itemIndex, true /*isSelected*/, false /*setAnchorRowIndex*/);
            }
            int cumulatedSelectedRows = 0;
            foreach (Range<bool> selectedRange in this._selectedItemsTable)
            {
                if (selectedRange.UpperBound >= itemIndex)
                {
                    cumulatedSelectedRows += itemIndex - selectedRange.LowerBound;
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
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.Single)
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            }

            if (this._selectedItemsTable.RangeCount > 0)
            {
                // Clearing the selection does not reset the potential current cell.
                if (!this.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                {
                    // Edited value couldn't be committed or aborted
                    throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                }
                this.OwningGrid.ClearRowSelection(true /*resetAnchorRowIndex*/);
            }
        }

        public bool Contains(object dataItem)
        {
            int itemIndex = this.OwningGrid.DataConnection.IndexOf(dataItem);
            if (itemIndex == -1)
            {
                return false;
            }
            Debug.Assert(itemIndex >= 0);

            return Contains(itemIndex);
        }

        public int IndexOf(object dataItem)
        {
            int itemIndex = this.OwningGrid.DataConnection.IndexOf(dataItem);
            if (itemIndex == -1)
            {
                return -1;
            }
            Debug.Assert(itemIndex >= 0);
            int cumulatedSelectedRows = 0;
            foreach (Range<bool> selectedRange in this._selectedItemsTable)
            {
                if (selectedRange.UpperBound >= itemIndex)
                {
                    if (selectedRange.LowerBound <= itemIndex)
                    {
                        return cumulatedSelectedRows + itemIndex - selectedRange.LowerBound;
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
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.Single)
            {
                throw DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode();
            }

            int itemIndex = this.OwningGrid.DataConnection.IndexOf(dataItem);
            if (itemIndex == -1)
            {
                return;
            }
            Debug.Assert(itemIndex >= 0);

            if (itemIndex == this.OwningGrid.CurrentRowIndex &&
                !this.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
            {
                // Edited value couldn't be committed or aborted
                throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
            }

            // 
            this.OwningGrid.SetRowSelection(itemIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
        }

        public void RemoveAt(int index)
        {
            if (this.OwningGrid.SelectionMode == DataGridSelectionMode.Single)
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
                !this.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
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
            return GetSelectedItems(this.OwningGrid, this._selectedItemsTable).GetEnumerator();
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
            foreach (int rowIndex in this._selectedItemsTable.GetIndexes())
            {
                this.OwningGrid.OnRemovedSelectedItem(rowIndex);
            }
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

        // Called when an item is deleted from the ItemsSource as opposed to just being unselected
        internal void Delete(int index, object item)
        {
            this.OwningGrid.OnRemovedSelectedItem(index, item);
            this._selectedItemsTable.RemoveIndex(index);
        }

        // 





        // Returns the exclusive index count between lowerBound and upperBound of all indexes with the given value
        internal int GetIndexCount(int lowerBound, int upperBound)
        {
            return this._selectedItemsTable.GetIndexCount(lowerBound, upperBound, true);
        }

        internal IEnumerable<int> GetIndexes()
        {
            return this._selectedItemsTable.GetIndexes();
        }

        internal void InsertIndexes(int rowIndexInserted, int insertionCount)
        {
            this._selectedItemsTable.InsertIndexes(rowIndexInserted, insertionCount);
        }

        internal void SelectRow(int rowIndex, bool select)
        {
            if (select)
            {
                this.OwningGrid.OnAddedSelectedItem(rowIndex);
                this._selectedItemsTable.AddValue(rowIndex, true);
            }
            else
            {
                this.OwningGrid.OnRemovedSelectedItem(rowIndex);
                this._selectedItemsTable.RemoveValue(rowIndex);
            }
        }

        internal void SelectRows(int startRowIndex, int rowCount, bool select)
        {
            if (select)
            {
                for (int rowIndex = startRowIndex; rowIndex < rowCount + startRowIndex; rowIndex++)
                {
                    this.OwningGrid.OnAddedSelectedItem(rowIndex);
                }
                this._selectedItemsTable.AddValues(startRowIndex, rowCount, true);
            }
            else
            {
                for (int rowIndex = startRowIndex; rowIndex < rowCount + startRowIndex; rowIndex++)
                {
                    this.OwningGrid.OnRemovedSelectedItem(rowIndex);
                }
                this._selectedItemsTable.RemoveValues(startRowIndex, rowCount);
            }
        }

        #endregion Internal Methods

        #region Private Methods

        private static IEnumerable<object> GetSelectedItems(DataGrid owningGrid, IndexToValueTable<bool> selectedItemsTable)
        {
            Debug.Assert(owningGrid != null);
            Debug.Assert(owningGrid.DataConnection != null);
            Debug.Assert(selectedItemsTable != null);

            DataGridDataConnection dataConnection = owningGrid.DataConnection;

            foreach (int rowIndex in selectedItemsTable.GetIndexes())
            {
                Debug.Assert(rowIndex > -1);
                yield return dataConnection.GetDataItem(rowIndex);
            }
        }

        #endregion
    }
}
