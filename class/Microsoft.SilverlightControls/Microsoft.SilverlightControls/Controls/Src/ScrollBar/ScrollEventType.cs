// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controls.Primitives
{ 
    /// <summary> 
    /// Specifies the type of action used to raise the Scroll event.
    /// </summary> 
    public enum ScrollEventType
    {
        /// <summary> 
        /// The Thumb was dragged to a new position and is now no longer being dragged by the user.
        /// </summary>
        EndScroll = 8, 
 
        /// <summary>
        /// The Thumb moved to the Minimum position of the ScrollBar. For a vertical ScrollBar, this movement occurs when the CTRL+HOME keys are pressed. This movement corresponds to a ScrollToTopCommand in a vertical ScrollBar and a ScrollToLeftEndCommand in a horizontal ScrollBar. 
        /// </summary>
        First = 6,
 
        /// <summary>
        /// The Thumb moved a specified distance, as determined by the value of LargeChange, to the left for a horizontal ScrollBar or upward for a vertical ScrollBar. For a vertical ScrollBar, this movement occurs when the page button that is above the Thumb is pressed, or when the PAGE UP key is pressed, and corresponds to a PageUpCommand. For a horizontal ScrollBar, this movement occurs when the page button to the left of the Thumb is pressed, and corresponds to a PageLeftCommand.
        /// </summary> 
        LargeDecrement = 2, 

        /// <summary> 
        /// The Thumb moved a specified distance, as determined by the value of LargeChange, to the right for a horizontal ScrollBar or downward for a vertical ScrollBar. For a vertical ScrollBar, this movement occurs when the page button that is below the Thumb is pressed, or when the PAGE DOWN key is pressed, and corresponds to a PageDownCommand. For a horizontal ScrollBar, this movement occurs when the page button to the right of the Thumb is pressed, and corresponds to a PageRightCommand.
        /// </summary>
        LargeIncrement = 3, 

        /// <summary>
        /// The Thumb moved to the Maximum position of the ScrollBar. For a vertical ScrollBar, this movement occurs when the CTRL+END keys are pressed. This movement corresponds to a ScrollToEndCommand in a vertical ScrollBar and a ScrollToRightEndCommand in a horizontal ScrollBar. 
        /// </summary> 
        Last = 7,
 
        /// <summary>
        /// The Thumb moved a small distance, as determined by the value of SmallChange, to the left for a horizontal ScrollBar or upward for a vertical ScrollBar. For a vertical ScrollBar, this movement occurs when the upper RepeatButton is pressed or when the UP ARROW key is pressed, and corresponds to a LineUpCommand. For a horizontal ScrollBar, this movement occurs when the left RepeatButton is pressed, and corresponds to a LineLeftCommand.
        /// </summary> 
        SmallDecrement = 0,

        /// <summary> 
        /// The Thumb moved a small distance, as determined by the value of SmallChange, to the right for a horizontal ScrollBar or downward for a vertical ScrollBar. For a vertical ScrollBar, this movement occurs when the lower RepeatButton is pressed or when the DOWN ARROW key is pressed, and corresponds to a LineDownCommand. For a horizontal ScrollBar, this movement occurs when the right RepeatButton is pressed, and corresponds to a LineRightCommand. 
        /// </summary>
        SmallIncrement = 1, 

        /// <summary>
        /// The Thumb moved to a new position because the user selected Scroll Here in the shortcut menu of the ScrollBar. This movement corresponds to the ScrollHereCommand. To view the shortcut menu, right-click the mouse when the pointer is over the ScrollBar. 
        /// </summary>
        ThumbPosition = 4,
 
        /// <summary> 
        /// The Thumb was dragged and caused a MouseMove event. A Scroll event of this ScrollEventType may occur more than one time when the Thumb is dragged in the ScrollBar.
        /// </summary> 
        ThumbTrack = 5
    }
} 
