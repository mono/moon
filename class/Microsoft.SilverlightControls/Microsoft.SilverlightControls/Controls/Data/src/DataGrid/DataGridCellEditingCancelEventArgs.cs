// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridCellEditingCancelEventArgs : DataGridCellCancelEventArgs 
    {
        public DataGridCellEditingCancelEventArgs(DataGridColumnBase dataGridColumn, 
                                                  DataGridRow dataGridRow,
                                                  FrameworkElement element,
                                                  DataGridEditingTriggerInfo editingTriggerInfo) : base(dataGridColumn, dataGridRow, element) 
        {
            this.EditingTriggerInfo = editingTriggerInfo;
        } 
 
        #region Public Properties
 
        public DataGridEditingTriggerInfo EditingTriggerInfo
        {
            get; 
            private set;
        }
 
        #endregion Public Properties 
    }
} 
