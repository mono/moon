// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridColumnEventArgs : EventArgs
    {
        public DataGridColumnEventArgs(DataGridColumn column)
        {
            if (column == null)
            {
                throw new ArgumentNullException("column");
            }
            this.Column = column;
        }

        public DataGridColumn Column
        {
            get;
            private set;
        }
    }
}
