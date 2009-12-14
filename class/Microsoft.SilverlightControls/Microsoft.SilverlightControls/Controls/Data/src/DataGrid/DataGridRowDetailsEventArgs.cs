// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    public class DataGridRowDetailsEventArgs : EventArgs 
    {
        public DataGridRowDetailsEventArgs(FrameworkElement detailsElement) 
        {
            this.DetailsElement = detailsElement;
        } 

        public FrameworkElement DetailsElement
        { 
            get; 
            private set;
        } 
    }
}
