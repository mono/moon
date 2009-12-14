// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System.Windows; 
 
namespace System.Windows.Controls.Primitives
{ 
    /// <summary>
    /// Provides data for a Scroll event that occurs when the Thumb of a ScrollBar moves.
    /// </summary> 
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "Derives from RoutedEventArgs instead of EventArgs")]
    public sealed class ScrollEventArgs : RoutedEventArgs
    { 
        /// <summary> 
        /// Gets a value that represents the new location of the Thumb in the ScrollBar.
        /// </summary> 
        public double NewValue
        {
            get { return this._newValue; } 
        }
        private double _newValue;
 
        /// <summary> 
        /// Gets the ScrollEventType enumeration value that describes the change in the Thumb position that caused this event.
        /// </summary> 
        public ScrollEventType ScrollEventType
        {
            get { return this._scrollEventType; } 
        }
        private ScrollEventType _scrollEventType;
 
        /// <summary> 
        /// Initializes an instance of the ScrollEventArgs class by using the specified ScrollEventType enumeration value and the new location of the Thumb control in the ScrollBar.
        /// </summary> 
        /// <param name="scrollEventType">A ScrollEventType enumeration value that describes the type of Thumb movement that caused the event.</param>
        /// <param name="newValue">The value that corresponds to the new location of the Thumb in the ScrollBar.</param>
        public ScrollEventArgs(ScrollEventType scrollEventType, double newValue) 
        {
            this._scrollEventType = scrollEventType;
            this._newValue = newValue; 
        } 
    }
 
    public delegate void ScrollEventHandler(object sender, ScrollEventArgs e);
}
