// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls
{
    public class DataGridAutoGeneratingColumnEventArgs : EventArgs
    {
        public DataGridAutoGeneratingColumnEventArgs(string propertyName, Type propertyType, DataGridColumn column)
        {
            this.Column = column;
            this.PropertyName = propertyName;
            this.PropertyType = propertyType;
        }

        public bool Cancel
        {
            get;
            set;
        }

        public DataGridColumn Column
        {
            get;
            set;
        }

        public string PropertyName
        {
            get;
            private set;
        }

        public Type PropertyType
        {
            get;
            private set;
        }
    }
}
