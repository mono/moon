// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Reflection;
 
namespace System.Windows.Controlsb1
{
    public class DataGridAutoGeneratingColumnEventArgs : DataGridColumnEventArgs 
    {
        public DataGridAutoGeneratingColumnEventArgs(PropertyInfo property, DataGridBoundColumnBase column) :
            base(column) 
        {
            this.Property = property;
        } 
 
        public bool Cancel
        { 
            get;
            set;
        } 

        public new DataGridBoundColumnBase Column
        { 
            get 
            {
                return base.Column as DataGridBoundColumnBase; 
            }
        }
 
        public PropertyInfo Property
        {
            get; 
            private set; 
        }
    } 
}
