// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridCellCancelEventArgs : DataGridCellEventArgs 
    {
        public DataGridCellCancelEventArgs(DataGridColumnBase dataGridColumn, 
                                           DataGridRow dataGridRow,
                                           FrameworkElement element) : base(dataGridColumn, dataGridRow, element)
        { 
        }

        #region Public Properties 
 
        public bool Cancel
        { 
            get;
            set;
        } 

        #endregion Public Properties
    } 
} 
