// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridTemplateColumn : DataGridColumnBase 
    {
        #region Constants 

        #endregion Constants
 
        #region Data

        private DataTemplate _cellTemplate; 
        private DataTemplate _cellEditingTemplate; 

        #endregion Data 

        public DataGridTemplateColumn()
        { 
        }

        #region Dependency Properties 
        /* 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 
*/

        #endregion 
 
        #region Public Properties
        public DataTemplate CellEditingTemplate 
        {
            get
            { 
                return this._cellEditingTemplate;
            }
            set 
            { 
                if (this._cellEditingTemplate != value)
                { 
                    this._cellEditingTemplate = value;
                    //
 


 
                } 
            }
        } 

        public DataTemplate CellTemplate
        { 
            get
            {
                return this._cellTemplate; 
            } 
            set
            { 
                if (this._cellTemplate != value)
                {
                    this._cellTemplate = value; 
                    //

 
 

                } 
            }
        }
 
        #endregion Public Properties

        #region Internal Properties 
 
        internal bool HasDistinctTemplates
        { 
            get
            {
                return this.CellTemplate != this.CellEditingTemplate; 
            }
        }
 
        #endregion Internal Properties 

        #region Public Methods 

        #endregion Public Methods
 
        #region Internal Methods

        #endregion Internal Methods 
 
        #region Private Methods
 
        #endregion Private Methods
    }
} 
