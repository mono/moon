// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows.Controls.Primitives
{
    /// <summary> 
    /// Provides information about the DragCompleted event that occurs when a
    /// user completes a drag operation with the mouse of a Thumb control.
    /// </summary> 
    public class DragStartedEventArgs : RoutedEventArgs
    {
        public double HorizontalOffset { get; private set; }
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
                public DragStartedEventArgs(double horizontalOffset, double verticalOffset)
            : base() 
        {
            HorizontalOffset = horizontalOffset;
            VerticalOffset = verticalOffset;
        }
    } 

    /// <summary>
    /// Represents the methods that handle the DragStarted event. 
    /// </summary> 
    /// <param name="sender">The current element along the event's route.</param>
    /// <param name="e">The event arguments containing additional information about the DragStarted event.</param> 
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")]
    public delegate void DragStartedEventHandler(object sender, DragStartedEventArgs e);
} 
