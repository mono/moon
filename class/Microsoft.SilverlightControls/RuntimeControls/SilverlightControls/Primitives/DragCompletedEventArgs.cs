// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 

using System;
using System.Windows; 
 
namespace System.Windows.Controls.Primitives
{ 
    /// <summary>
    /// Provides information about the DragCompleted event that occurs when a
    /// user completes a drag operation with the mouse of a Thumb control. 
    /// </summary>
    public class DragCompletedEventArgs : RoutedEventArgs
    { 
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

        /// <summary> 
        /// Read-only access to the horizontal offset of the mouse click with respect to the screen
        /// coordinates of the Thumb.
        /// </summary> 
        public double HorizontalChange { get; private set; } 

        /// <summary> 
        /// Read-only access to the vertical offset of the mouse click with respect to the screen
        /// coordinates of the Thumb.
        /// </summary> 
        public double VerticalChange { get; private set; }

        /// <summary> 
        /// Gets whether the drag operation for a Thumb was canceled by a call 
        /// to the CancelDrag method.
        /// </summary> 
        public bool Canceled { get; private set; }

    } 

    /// <summary>
    /// Represents the methods that handle the DragEventCompleted event. 
    /// </summary> 
    /// <param name="sender">The current element along the event's route.</param>
    /// <param name="e">The event arguments containing additional information about the DragEventCompleted event.</param> 
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")]
    public delegate void DragCompletedEventHandler(object sender, DragCompletedEventArgs e);
} 
