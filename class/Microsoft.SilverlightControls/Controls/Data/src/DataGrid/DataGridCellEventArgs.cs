// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
 
namespace System.Windows.Controlsb1
{
    public class DataGridCellEventArgs : EventArgs 
    {
        public DataGridCellEventArgs(DataGridColumnBase dataGridColumn,
                                     DataGridRow dataGridRow, 
                                     FrameworkElement element)
        {
            if (dataGridColumn == null) 
            { 
                throw new ArgumentNullException("dataGridColumn");
            } 
            if (dataGridRow == null)
            {
                throw new ArgumentNullException("dataGridRow"); 
            }
            if (element == null)
            { 
                throw new ArgumentNullException("element"); 
            }
            this.Column = dataGridColumn; 
            this.Row = dataGridRow;
            this.Element = element;
        } 

        internal DataGridCellEventArgs(DataGridCell dataGridCell)
        { 
            Debug.Assert(dataGridCell != null); 
            this.ColumnIndex = dataGridCell.ColumnIndex;
            this.RowIndex = dataGridCell.RowIndex; 
            this.Cell = dataGridCell;
        }
 
        internal DataGridCellEventArgs(int columnIndex, DataGridCell dataGridCell)
        {
            this.ColumnIndex = columnIndex; 
            this.RowIndex = -1; 
            this.Cell = dataGridCell;
        } 

        #region Public Properties
 
        public DataGridColumnBase Column
        {
            get; 
            private set; 
        }
 
        public FrameworkElement Element
        {
            get; 
            private set;
        }
 
        public DataGridRow Row 
        {
            get; 
            private set;
        }
 
        #endregion Public Properties

        #region Internal Properties 
 
        internal DataGridCell Cell
        { 
            get;
            private set;
        } 

        internal int ColumnIndex
        { 
            get; 
            private set;
        } 

        internal int RowIndex
        { 
            get;
            private set;
        } 
 
        #endregion Internal Properties
    } 
}
