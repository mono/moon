// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridPreparingCellForEditEventArgs : EventArgs
    {
        public DataGridPreparingCellForEditEventArgs(DataGridColumn column, 
                                                     DataGridRow row, 
                                                     RoutedEventArgs editingEventArgs,
                                                     FrameworkElement editingElement)
        {
            this.Column = column;
            this.Row = row;
            this.EditingEventArgs = editingEventArgs;
            this.EditingElement = editingElement;
        }

        #region Public Properties

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

        #endregion Public Properties
    }
}
