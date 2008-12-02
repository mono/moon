// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
 
namespace System.Windows.Controlsb1 
{
    internal class DataGridFillerColumn : DataGridColumnBase 
    {
        private double _width;
 
        public DataGridFillerColumn(DataGrid owningGrid)
        {
            this.OwningGrid = owningGrid; 
        } 

        // True if there is room for the filler column; otherwise, false 
        internal bool IsActive
        {
            get; 
            set;
        }
 
        // True if the FillerColumn's header cell is contained in the visual tree 
        internal bool IsRepresented
        { 
            get;
            set;
        } 

        internal new double Width
        { 
            get 
            {
                return this.IsActive ? _width : 0; 
            }
            set
            { 
                Debug.Assert(value > 0);
                _width = value;
            } 
        } 
    }
} 
