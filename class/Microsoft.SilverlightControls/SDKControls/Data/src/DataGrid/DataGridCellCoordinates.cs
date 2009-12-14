// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Globalization;

namespace System.Windows.Controls
{
    internal class DataGridCellCoordinates
    {
        public DataGridCellCoordinates(int columnIndex, int rowIndex)
        {
            this.ColumnIndex = columnIndex;
            this.RowIndex = rowIndex;
        }

        public DataGridCellCoordinates(DataGridCellCoordinates dataGridCellCoordinates) : this(dataGridCellCoordinates.ColumnIndex, dataGridCellCoordinates.RowIndex)
        { 
        }

        public int ColumnIndex
        {
            get;
            set;
        }

        public int RowIndex
        {
            get;
            set;
        }                
        
        public override bool Equals(object o)
        {
            DataGridCellCoordinates dataGridCellCoordinates = o as DataGridCellCoordinates;
            if (dataGridCellCoordinates != null)
            {
                return dataGridCellCoordinates.ColumnIndex == this.ColumnIndex && dataGridCellCoordinates.RowIndex == this.RowIndex;
            }
            return false;
        }
        
        public override int GetHashCode()
        {
            return ((~this.ColumnIndex * (this.RowIndex+1)) & 0x00ffff00) >> 8;
        }

        public override string ToString()
        {
            return "DataGridCellCoordinates {ColumnIndex = " + this.ColumnIndex.ToString(CultureInfo.CurrentCulture) + 
                   ", RowIndex = " + this.RowIndex.ToString(CultureInfo.CurrentCulture) + "}";
        }
    }
}
