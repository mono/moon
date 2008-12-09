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
    public class DragCompletedEventArgs : RoutedEventArgs
    {
        /// <summary> 
        /// Gets whether the drag operation for a Thumb was canceled by a call 
        /// to the CancelDrag method.
        /// </summary> 
        public bool Canceled { get; private set; }

        public double HorizontalChange { get; private set; }

        public double VerticalChange { get; private set; }

        /// <summary> 
        /// Initializes a new instance of the DragCompletedEventArgs class.
        /// </summary>
        /// <param name="horizontalChange"> 
        /// The horizontal offset of the mouse click with respect to the screen 
        /// coordinates of the Thumb.
        /// </param> 
        /// <param name="verticalChange">
        /// The vertical offset of the mouse click with respect to the screen
        /// coordinates of the Thumb. 
        /// </param>
        /// <param name="canceled">
        /// A Boolean value that indicates whether the drag operation was 
        /// canceled by a call to the CancelDrag method. 
        /// </param>
        public DragCompletedEventArgs(double horizontalChange, double verticalChange, bool canceled) 
            : base()
        {
            Canceled = canceled; 
            HorizontalChange = horizontalChange;
            VerticalChange = verticalChange;
        }
    }
 
    /// <summary> 
    /// Represents the methods that handle the DragEventCompleted event.
    /// </summary> 
    /// <param name="sender">The current element along the event's route.</param>
    /// <param name="e">The event arguments containing additional information about the DragEventCompleted event.</param>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")] 
    public delegate void DragCompletedEventHandler(object sender, DragCompletedEventArgs e);
}
