// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridBeginningEditEventArgs : EventArgs
    {
        public DataGridBeginningEditEventArgs(DataGridColumn column,
                                              DataGridRow row,
                                              RoutedEventArgs editingEventArgs)
        {
            this.Column = column;
            this.Row = row;
            this.EditingEventArgs = editingEventArgs;
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

        public RoutedEventArgs EditingEventArgs
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
