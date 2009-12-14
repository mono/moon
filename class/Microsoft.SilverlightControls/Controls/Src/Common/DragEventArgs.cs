// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows 
{
    /// <summary> 
    /// Provides information about the DragStarted and DragDelta events that
    /// occur when a user drags a Thumb control with the mouse.
    /// </summary> 
    public class DragEventArgs : EventArgs
    {
        /// <summary> 
        /// Gets the horizontal offset of the mouse click relative to the screen 
        /// coordinates of the Thumb.
        /// </summary> 
        public double HorizontalOffset { get; private set; }

        /// <summary> 
        /// Gets the vertical offset of the mouse click relative to the screen
        /// coordinates of the Thumb.
        /// </summary> 
        public double VerticalOffset { get; private set; } 

        /// <summary> 
        /// Initializes a new instance of the DragStartedEventArgs class.
        /// </summary>
        /// <param name="horizontalOffset"> 
        /// The horizontal offset of the mouse click with respect to the screen
        /// coordinates of the Thumb.
        /// </param> 
        /// <param name="verticalOffset"> 
        /// The vertical offset of the mouse click with respect to the screen
        /// coordinates of the Thumb. 
        /// </param>
        public DragEventArgs(double horizontalOffset, double verticalOffset)
        { 
            HorizontalOffset = horizontalOffset;
            VerticalOffset = verticalOffset;
        } 
    } 

    /// <summary> 
    /// Represents the methods that handle the Drag event.
    /// </summary>
    /// <param name="sender">The current element along the event's route.</param> 
    /// <param name="e">The event arguments containing additional information about the Drag event.</param>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")]
    public delegate void DragEventHandler(object sender, DragEventArgs e); 
} 
