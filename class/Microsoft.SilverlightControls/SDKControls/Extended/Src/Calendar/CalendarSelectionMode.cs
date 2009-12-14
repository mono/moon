// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    /// <summary>
    /// Specifies values for the different selection modes of a Calendar. 
    /// </summary>
    public enum CalendarSelectionMode
    {
        /// <summary>
        /// One date can be selected at a time.
        /// </summary>
        SingleDate = 0,
        /// <summary>
        /// One range of dates can be selected at a time.
        /// </summary>
        SingleRange = 1,
        /// <summary>
        /// Multiple dates or ranges can be selected at a time.
        /// </summary>
        MultipleRange = 2,
        /// <summary>
        /// No dates can be selected.
        /// </summary>
        None = 3,
    }

}
