// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    // Determines the location and visibility of the editing row. 
    internal enum DataGridEditingRowLocation
    { 
        Bottom, // The editing row is collapsed below the displayed rows
        Inline, // The editing row is visible and displayed
        Top     // The editing row is collapsed above the displayed rows 
    }

    /// <summary> 
    /// Determines whether the inner cells' vertical/horizontal gridlines are shown or not. 
    /// </summary>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1717:OnlyFlagsEnumsShouldHavePluralNames")] 
    public enum DataGridGridlines
    {
        All, 
        Horizontal,
        Vertical,
        None 
    } 

    [Flags] 
    /// <summary>
    /// Determines whether the row/column headers are shown or not.
    /// </summary> 
    public enum DataGridHeaders
    {
        /// <summary> 
        /// Show Row, Column, and Corner Headers 
        /// </summary>
        All = Row | Column, 

        /// <summary>
        /// Show only Column Headers with top-right corner Header 
        /// </summary>
        Column = 0x01,
 
        /// <summary> 
        /// Show only Row Headers with bottom-left corner
        /// </summary> 
        Row = 0x02,

        /// <summary> 
        /// Don’t show any Headers
        /// </summary>
        None = 0x00 
    } 

    // Determines the location of the new input row in regards to the scrolling rows. 
    //
    enum DataGridNewRowLocation
    { 
        Bottom, // The new row is frozen at the bottom of the grid, on top of the potential horizontal scrollbar
        Inline, // The new row is placed below the last scrolling row, and scrolls with the rows
        Top     // The new row is frozen at the top of the grid between the column headers and scrolling rows 
    } 

    public enum DataGridRowDetailsVisibility 
    {
        Collapsed = 0,          // Show no details by default.  Developer is in charge of toggling visibility
        Visible = 1,	        // Show the details section for all rows 
        VisibleWhenSelected = 2	// Show the details section only for the selected row(s)
    }
 
    /// <summary> 
    /// Determines the selection model
    /// </summary> 
    public enum DataGridSelectionMode
    {
        SingleFullRow, 
        ExtendedFullRow
    }
} 
