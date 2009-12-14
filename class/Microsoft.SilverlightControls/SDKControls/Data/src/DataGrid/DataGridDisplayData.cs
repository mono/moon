// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Media;

namespace System.Windows.Controls
{
    internal class DataGridDisplayData
    {
        #region Data

        private Stack<DataGridRow> _fullyRecycledRows; // list of Rows that have been fully recycled (Collapsed)
        private int _headScrollingRowIndex; // index of the row in _scrollingRows that is the first displayed row
        private Stack<DataGridRow> _recyclableRows; // list of Rows which have not been fully recycled (avoids Measure in several cases)
        private List<DataGridRow> _scrollingRows; // circular list of displayed rows
        

        #endregion Data

        public DataGridDisplayData()
        {
            ResetRowIndexes();
            this.FirstDisplayedScrollingCol = -1;
            this.LastTotallyDisplayedScrollingCol = -1;

            _scrollingRows = new List<DataGridRow>();
            _recyclableRows = new Stack<DataGridRow>();
            _fullyRecycledRows = new Stack<DataGridRow>();
        }

        #region Properties

        private int CircularIndexOfFirstScrollingRowBeforeWrapping
        {
            get
            {
                return this.FirstDisplayedScrollingRow + _headScrollingRowIndex;
            }
        }

        public int FirstDisplayedScrollingCol
        {
            get;
            set;
        }

        public int FirstDisplayedScrollingRow
        {
            get;
            private set;
        }

        public int LastDisplayedScrollingRow
        {
            get;
            private set;
        }

        public int LastTotallyDisplayedScrollingCol
        {
            get;
            set;
        }

        public int NumDisplayedScrollingRows
        {
            get
            {
                return _scrollingRows.Count;
            }
        }

        public int NumTotallyDisplayedScrollingRows
        {
            get;
            set;
        }

        internal double PendingVerticalScrollHeight
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        internal void AddRecylableRow(DataGridRow row)
        {
            Debug.Assert(!_recyclableRows.Contains(row));
            row.DetachFromDataGrid(true);
            _recyclableRows.Push(row);
        }

        internal void ClearRows(bool recycleRows)
        {
            ResetRowIndexes();
            if (recycleRows)
            {
                foreach (DataGridRow row in _scrollingRows)
                {
                    if (row.IsRecyclable)
                    {
                        AddRecylableRow(row);
                    }
                    else
                    {
                        row.Clip = new RectangleGeometry();
                    }
                }
            }
            else
            {
                _recyclableRows.Clear();
                _fullyRecycledRows.Clear();
            }
            _scrollingRows.Clear();
        }

        internal void CorrectRowsAfterDeletion(int rowIndex)
        {
            if (IsRowDisplayed(rowIndex))
            {
                UnloadScrollingRow(rowIndex);
            }
            // This cannot be an else condition because if there are 2 rows left, and you delete the first one
            // then these indexes need to be updated as well
            if (rowIndex < this.FirstDisplayedScrollingRow)
            {
                this.FirstDisplayedScrollingRow--;
                this.LastDisplayedScrollingRow--;
            }
        }

        internal void CorrectRowsAfterInsertion(int rowIndex, DataGridRow row)
        {
            if (rowIndex < this.FirstDisplayedScrollingRow)
            {
                // The row was inserted above our viewport, just update our indexes
                this.FirstDisplayedScrollingRow++;
                this.LastDisplayedScrollingRow++;
            }
            else if (rowIndex - 1 <= this.LastDisplayedScrollingRow || this.LastDisplayedScrollingRow == -1)
            {
                Debug.Assert(row != null);
                // The row was inserted in our viewport, add it as a scrolling row
                LoadScrollingRow(rowIndex, row);
            }
        }

        internal void FullyRecycleRows()
        {
            // Fully recycle Recycleable rows and transfer them to Recycled rows
            while (_recyclableRows.Count > 0)
            {
                DataGridRow row = _recyclableRows.Pop() as DataGridRow;
                Debug.Assert(row != null);
                row.Visibility = Visibility.Collapsed;
                Debug.Assert(!_fullyRecycledRows.Contains(row));
                _fullyRecycledRows.Push(row);
            }
        }

        internal DataGridRow GetDisplayedRow(int rowIndex)
        {
            Debug.Assert(rowIndex >= this.FirstDisplayedScrollingRow);
            Debug.Assert(rowIndex <= this.LastDisplayedScrollingRow);
            
            return _scrollingRows[(rowIndex - this.CircularIndexOfFirstScrollingRowBeforeWrapping) % _scrollingRows.Count];
        }

        // Returns an enumeration of the displayed scrolling rows in order starting with the FirstDisplayedScrollingRow
        internal IEnumerable<DataGridRow> GetScrollingRows()
        {
            for (int i = 0; i < _scrollingRows.Count; i++)
            {
                // _scrollingRows is a circular list that wraps
                yield return _scrollingRows[(_headScrollingRowIndex + i) % _scrollingRows.Count];
            }
        }

        internal DataGridRow GetUsedRow()
        {
            if (_recyclableRows.Count > 0)
            {
                return _recyclableRows.Pop();
            }
            else if (_fullyRecycledRows.Count > 0)
            {
                // For fully recycled rows, we need to set the Visibility back to Visible
                DataGridRow row = _fullyRecycledRows.Pop();
                row.Visibility = Visibility.Visible;
                return row;
            }
            return null;
        }

        internal bool IsRowDisplayed(int rowIndex)
        {
            return rowIndex >= this.FirstDisplayedScrollingRow && 
                   rowIndex <= this.LastDisplayedScrollingRow && 
                   rowIndex != -1;
        }

        // Tracks the row at index rowIndex as a scrolling row
        internal void LoadScrollingRow(int rowIndex, DataGridRow row)
        {
            if (_scrollingRows.Count == 0)
            {
                SetScrollingRowIndexes(rowIndex);
                _scrollingRows.Add(row);
            }
            else
            {
                // The row should be adjacent to the other rows being displayed
                Debug.Assert((rowIndex >= this.FirstDisplayedScrollingRow - 1) && (rowIndex <= this.LastDisplayedScrollingRow + 1));
                if (rowIndex < this.FirstDisplayedScrollingRow)
                {
                    this.FirstDisplayedScrollingRow = rowIndex;
                }
                else
                {
                    this.LastDisplayedScrollingRow++;
                }
                int insertIndex = rowIndex - this.CircularIndexOfFirstScrollingRowBeforeWrapping;
                if (insertIndex > _scrollingRows.Count)
                {
                    // We need to wrap around from the bottom to the top of our circular list; as a result the head of the list moves forward
                    insertIndex -= _scrollingRows.Count;
                    _headScrollingRowIndex++;
                }
                _scrollingRows.Insert(insertIndex, row);
            }
        }

        private void ResetRowIndexes()
        {
            SetScrollingRowIndexes(-1);
            this.NumTotallyDisplayedScrollingRows = 0;
            _headScrollingRowIndex = 0;
        }

        private void SetScrollingRowIndexes(int newValue)
        {
            this.FirstDisplayedScrollingRow = newValue;
            this.LastDisplayedScrollingRow = newValue;
        }

        // Stops tracking the row at index rowIndex as a scrolling row
        internal void UnloadScrollingRow(int rowIndex)
        {
            Debug.Assert(IsRowDisplayed(rowIndex));
            int removeIndex = rowIndex - this.CircularIndexOfFirstScrollingRowBeforeWrapping;
            if (removeIndex > _scrollingRows.Count)
            {
                // We need to wrap around from the top to the bottom of our circular list
                removeIndex -= _scrollingRows.Count;
                _headScrollingRowIndex--;
            }
            _scrollingRows.RemoveAt(removeIndex);
            if (rowIndex == this.FirstDisplayedScrollingRow)
            {
                this.FirstDisplayedScrollingRow++;
            }
            else
            {
                this.LastDisplayedScrollingRow--;
            }
            if (this.LastDisplayedScrollingRow < this.FirstDisplayedScrollingRow)
            {
                ResetRowIndexes();
            }
        }

        #endregion Methods

    }
}
