// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridEndingEditEventArgs : EventArgs
    {
        public DataGridEndingEditEventArgs(DataGridColumn column,
                                           DataGridRow row,
                                           FrameworkElement editingElement,
                                           DataGridEditingUnit editingUnit)
        {
            this.Column = column;
            this.Row = row;
            this.EditingElement = editingElement;
            this.EditingUnit = editingUnit;
        }

        #region Properties

        public bool Cancel
        {
            get;
            set;
        }

        public DataGridColumn Column
        {
            get;
            private set;
        }

        public FrameworkElement EditingElement
        {
            get;
            private set;
        }

        public DataGridEditingUnit EditingUnit
        {
            get;
            private set;
        }

        public DataGridRow Row
        {
            get;
            private set;
        }

        #endregion Properties
    }
}
