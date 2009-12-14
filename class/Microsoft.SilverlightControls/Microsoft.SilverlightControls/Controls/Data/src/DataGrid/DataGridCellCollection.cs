// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections;
using System.Collections.Generic; 
using System.Diagnostics; 

namespace System.Windows.Controlsb1
{
    //
    internal class DataGridCellCollection : IList<DataGridCell> 
    {
        #region Data
 
        private List<DataGridCell> _cells; 
        private DataGridRow _owningRow;
 
        #endregion Data

        #region Events 

        internal event EventHandler<DataGridCellEventArgs> CellAdded;
        internal event EventHandler<DataGridCellEventArgs> CellRemoved; 
 
        #endregion Events
 
        public DataGridCellCollection(DataGridRow owningRow)
        {
            this._owningRow = owningRow; 
            this._cells = new List<DataGridCell>();
        }
 
        #region IList<DataGridCell> Members 

        public int IndexOf(DataGridCell cell) 
        {
            Debug.Assert(cell != null);
 
            return this._cells.IndexOf(cell);
        }
 
        public void Insert(int cellIndex, DataGridCell cell) 
        {
            Debug.Assert(cellIndex >= 0 && cellIndex <= this._cells.Count); 
            Debug.Assert(cell != null);

            cell.OwningRow = this._owningRow; 
            this._cells.Insert(cellIndex, cell);

            if (CellAdded != null) 
            { 
                CellAdded(this, new DataGridCellEventArgs(cellIndex, cell));
            } 
        }

        public void RemoveAt(int cellIndex) 
        {
            DataGridCell dataGridCell = this._cells[cellIndex];
            this._cells.RemoveAt(cellIndex); 
            dataGridCell.OwningRow = null; 
            if (CellRemoved != null)
            { 
                CellRemoved(this, new DataGridCellEventArgs(cellIndex, dataGridCell));
            }
        } 

        public DataGridCell this[int index]
        { 
            get 
            {
                if (index < 0 || index >= this._cells.Count) 
                {
                    throw DataGridError.DataGrid.ValueMustBeBetween("index", "Index", 0, true, this._cells.Count, false);
                } 
                return this._cells[index];
            }
            set 
            { 
                throw new NotSupportedException();
            } 
        }

        #endregion IList<DataGridCell> Members 


        #region ICollection<DataGridCell> Members 
 
        public void Add(DataGridCell cell)
        { 
            Debug.Assert(cell != null);

            cell.OwningRow = this._owningRow; 
            this._cells.Add(cell);

            if (CellAdded != null) 
            { 
                CellAdded(this, new DataGridCellEventArgs(this._cells.Count-1, cell));
            } 
        }

        public void Clear() 
        {
            this._cells.Clear();
        } 
 
        public bool Contains(DataGridCell cell)
        { 
            return this._cells.Contains(cell);
        }
 
        public void CopyTo(DataGridCell[] array, int arrayIndex)
        {
            throw new NotImplementedException(); 
        } 

        public int Count 
        {
            get
            { 
                return this._cells.Count;
            }
        } 
 
        public bool IsReadOnly
        { 
            get
            {
                return false; 
            }
        }
 
        public bool Remove(DataGridCell cell) 
        {
            Debug.Assert(cell != null); 

            int cellIndex = -1;
            int cellsCount = this._cells.Count; 
            for (int i = 0; i < cellsCount; ++i)
            {
                if (this._cells[i] == cell) 
                { 
                    cellIndex = i;
                    break; 
                }
            }
            if (cellIndex == -1) 
            {
                return false;
            } 
            else 
            {
                RemoveAt(cellIndex); 
                return true;
            }
        } 

        #endregion ICollection<DataGridCell> Members
 
 
        #region IEnumerable<DataGridCell> Members
 
        public IEnumerator<DataGridCell> GetEnumerator()
        {
            return this._cells.GetEnumerator(); 
        }

        #endregion IEnumerable<DataGridCell> Members 
 

        #region IEnumerable Members 

        IEnumerator IEnumerable.GetEnumerator()
        { 
            return this._cells.GetEnumerator();
        }
 
        #endregion IEnumerable Members 
    }
} 
