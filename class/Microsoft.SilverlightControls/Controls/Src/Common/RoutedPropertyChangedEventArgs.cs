// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows 
{
    /// <summary> 
    /// Provides data about a change in value to a dependency property as
    /// reported by particular routed events, including the previous and
    /// current value of the property that changed. 
    /// </summary>
    /// <typeparam name="T">
    /// The type of the dependency property that has changed. 
    /// </typeparam> 
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "Derives from RoutedEventArgs instead of EventArgs")]
    public class RoutedPropertyChangedEventArgs<T> : RoutedEventArgs 
    {
        /// <summary>
        /// Gets the previous value of the property being tracked as reported by 
        /// an event.
        /// </summary>
        public T OldValue { get; private set; } 
 
        /// <summary>
        /// Gets the new value of the property being tracked as reported by an 
        /// event.
        /// </summary>
        public T NewValue { get; private set; } 

        /// <summary>
        /// Initializes a new instance of the RoutedPropertyChangedEventArgs 
        /// class, with provided old and new values. 
        /// </summary>
        /// <param name="oldValue"> 
        /// Previous value of the property, prior to the event being raised.
        /// </param>
        /// <param name="newValue"> 
        /// Current value of the property at the time of the event.
        /// </param>
        public RoutedPropertyChangedEventArgs(T oldValue, T newValue) 
        { 
            OldValue = oldValue;
            NewValue = newValue; 
        }
    }
} 
