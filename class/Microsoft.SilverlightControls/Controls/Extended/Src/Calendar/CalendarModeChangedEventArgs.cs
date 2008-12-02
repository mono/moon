// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 


namespace System.Windows.Controlsb1
{ 
    /// <summary>
    /// Specifies values for the different modes of operation of a Calendar. 
    /// </summary>
    public enum CalendarMode
    { 
        /// <summary>
        /// The Calendar displays a month at a time.
        /// </summary> 
        Month = 0, 
        /// <summary>
        ///  The Calendar displays a year at a time. 
        /// </summary>
        Year = 1,
    } 

    /// <summary>
    /// Provides data for the DisplayModeChanged event. 
    /// </summary> 
    public class CalendarModeChangedEventArgs : RoutedEventArgs
    { 
        /// <summary>
        /// Initializes a new instance of the CalendarModeChangedEventArgs class.
        /// </summary> 
        /// <param name="oldMode">Previous value of the property, prior to the event being raised.</param>
        /// <param name="newMode">Current value of the property at the time of the event.</param>
        public CalendarModeChangedEventArgs(CalendarMode oldMode, CalendarMode newMode) 
        { 
            this.OldMode = oldMode;
            this.NewMode = newMode; 
        }

        /// <summary> 
        /// Gets the new mode of the Calendar.
        /// </summary>
        public CalendarMode NewMode 
        { 
            get;
            private set; 
        }

        /// <summary> 
        /// Gets the previous mode of the Calendar.
        /// </summary>
        public CalendarMode OldMode 
        { 
            get;
            private set; 
        }
    }
} 
