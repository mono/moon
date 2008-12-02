// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridDataErrorEventArgs : EventArgs 
    {
        #region Data 

        private bool _throwException;
 
        #endregion Data

        public DataGridDataErrorEventArgs(Exception exception, 
                                          DataGridColumnBase dataGridColumn, 
                                          DataGridRow dataGridRow)
        { 
            if (dataGridColumn == null)
            {
                throw new ArgumentNullException("dataGridColumn"); 
            }
            //
 
 

 

            this.Column = dataGridColumn;
            this.Row = dataGridRow; 
            this.Exception = exception;
        }
 
        #region Public Properties 

        public bool Cancel 
        {
            get;
            set; 
        }

        public DataGridColumnBase Column 
        { 
            get;
            private set; 
        }

        public Exception Exception 
        {
            get;
            private set; 
        } 

        public DataGridRow Row 
        {
            get;
            private set; 
        }

        // 
        public bool ThrowException 
        {
            get 
            {
                return this._throwException;
            } 
            set
            {
                if (value && this.Exception == null) 
                { 
                    throw DataGridError.DataGridDataErrorEventArgs.CannotThrowNullException();
                } 
                this._throwException = value;
            }
        } 

        #endregion Public Properties
    } 
} 
