// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows 
{
    /// <summary> 
    /// Represents methods that handle various routed events that track property
    /// value changes.
    /// </summary> 
    /// <typeparam name="T">
    /// The type of the dependency property that has changed.
    /// </typeparam> 
    /// <param name="sender"> 
    /// The object where the event handler is attached.
    /// </param> 
    /// <param name="e">The event data.</param>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1003:UseGenericEventHandlerInstances", Justification = "Included for compatibility with WPF.")]
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1009:DeclareEventHandlersCorrectly", Justification = "Derives from RoutedEventArgs instead of EventArgs")] 
    public delegate void RoutedPropertyChangedEventHandler<T>(object sender, RoutedPropertyChangedEventArgs<T> e);
}
