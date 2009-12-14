// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridColumnEventArgs : EventArgs 
    {
        public DataGridColumnEventArgs(DataGridColumnBase column) 
        {
            if (column == null)
            { 
                throw new ArgumentNullException("column");
            }
            this.Column = column; 
        } 

        public DataGridColumnBase Column 
        {
            get;
            private set; 
        }
    }
} 
