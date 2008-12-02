// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 


namespace System.Windows.Controlsb1
{ 
    /// <summary>
    /// Provides data for the DateSelected and DisplayDateChanged events. 
    /// </summary>
    public class CalendarDateChangedEventArgs : RoutedEventArgs
    { 
        internal CalendarDateChangedEventArgs(DateTime? removedDate, DateTime? addedDate)
        {
            this.RemovedDate = removedDate; 
            this.AddedDate = addedDate; 
        }
 
        /// <summary>
        /// Gets the date to be newly displayed.
        /// </summary> 
        public DateTime? AddedDate
        {
            get; 
            private set; 
        }
 
        /// <summary>
        /// Gets the date that was previously displayed.
        /// </summary> 
        public DateTime? RemovedDate
        {
            get; 
            private set; 
        }
    } 
}
