// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


namespace System.Windows.Controls.Primitives 
{ 
    /// <summary>
    /// Specifies the type of action used to raise the Scroll event. 
    /// </summary>
    public enum ScrollEventType
    { 
        /// <summary>
        /// The Thumb was being dragged to a new position and has stopped moving.
        /// </summary> 
        EndScroll = 8, 

        /// <summary> 
        /// The Thumb moved to the Minimum position of the ScrollBar.
        /// </summary>
        First = 6, 

        /// <summary>
        /// Thumb was moved a large distance. 
        /// The user clicked the scroll bar to the left(horizontal) or above(vertical) the scroll box. 
        /// </summary>
        LargeDecrement = 2, 

        /// <summary>
        /// Thumb was moved a large distance. 
        /// The user clicked the scroll bar to the right(horizontal) or below(vertical) the scroll box.
        /// </summary>
        LargeIncrement = 3, 
 
        /// <summary>
        /// The Thumb moved to the Maximum position of the ScrollBar. 
        /// </summary>
        Last = 7,
 
        /// <summary>
        /// Thumb was moved a small distance. The user clicked the left(horizontal) or top(vertical) scroll arrow.
        /// </summary> 
        SmallDecrement = 0, 

        /// <summary> 
        /// Thumb was moved a small distance. The user clicked the right(horizontal) or bottom(vertical) scroll arrow.
        /// </summary>
        SmallIncrement = 1, 

        /// <summary>
        /// The Thumb moved to a new position because the user selected Scroll Here in the shortcut menu of the ScrollBar. This movement corresponds to the ScrollHereCommand. To view the shortcut menu, right-click the mouse when the pointer is over the ScrollBar. 
        /// </summary> 
        ThumbPosition = 4,
 
        /// <summary>
        /// The Thumb was dragged and is currently being moved due to a MouseMove event.
        /// </summary> 
        ThumbTrack = 5
    }
} 
