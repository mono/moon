// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 

using System;
using System.Windows; 
 
namespace SilverlightControls.Primitives
{ 
    /// <summary>
    /// Provides information about the DragCompleted event that occurs when a
    /// user completes a drag operation with the mouse of a Thumb control. 
    /// </summary>
    public class DragStartedEventArgs : RoutedEventArgs
    { 
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
 
         /// <summary> 
        /// Read-only access to the horizontal offset (relative to Thumb's co-ordinate).
        /// </summary> 
        public double HorizontalOffset { get; private set; }

        /// <summary> 
        /// Read-only access to the vertical offset (relative to Thumb's co-ordinate).
        /// </summary>
        public double VerticalOffset { get; private set; } 
 
    }
 
    /// <summary>
    /// Represents the methods that handle the DragStarted event.
    /// </summary> 
    /// <param name="sender">The current element along the event's route.</param>
    /// <param name="e">The event arguments containing additional information about the DragStarted event.</param>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")] 
    public delegate void DragStartedEventHandler(object sender, DragStartedEventArgs e); 
}
