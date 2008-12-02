// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 


namespace System.Windows.Controls 
{ 
    /// <summary>
    /// Provides data for the DateSelected event. 
    /// </summary>
    public class DatePickerDateChangedEventArgs : RoutedEventArgs
    { 
        internal DatePickerDateChangedEventArgs(DateTime? removedDate, DateTime? addedDate)
        {
            this.RemovedDate = removedDate; 
            this.AddedDate = addedDate; 
        }
 
        /// <summary>
        /// Gets the newly selected date.
        /// </summary> 
        public DateTime? AddedDate
        {
            get; 
            private set; 
        }
 
        /// <summary>
        /// Gets the previous selected date.
        /// </summary> 
        public DateTime? RemovedDate
        {
            get; 
            private set; 
        }
    } 
}
