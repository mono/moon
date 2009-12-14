// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridRowEventArgs : EventArgs 
    {
        public DataGridRowEventArgs(DataGridRow dataGridRow) 
        {
            if (dataGridRow == null)
            { 
                throw new ArgumentNullException("dataGridRow");
            }
            this.Row = dataGridRow; 
        } 

        public DataGridRow Row 
        {
            get;
            private set; 
        }
    }
} 
