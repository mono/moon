// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridRowDetailsEventArgs : EventArgs
    {
        public DataGridRowDetailsEventArgs(DataGridRow row, FrameworkElement detailsElement)
        {
            this.Row = row;
            this.DetailsElement = detailsElement;
        }

        public FrameworkElement DetailsElement
        {
            get;
            private set;
        }

        public DataGridRow Row
        {
            get;
            private set;
        }
    }
}
