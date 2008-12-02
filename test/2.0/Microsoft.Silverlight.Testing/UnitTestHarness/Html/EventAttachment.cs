// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A type that represents an event attachment, with the handler and name.
    /// </summary>
    /// <typeparam name="T">The type of handler.</typeparam>
    public class EventAttachment<T>
    {
        /// <summary>
        /// Gets or sets the event handler.
        /// </summary>
        public T Handler
        {
            get;
            set;
        }
        
        /// <summary>
        /// Gets or sets the name of the event.
        /// </summary>
        public string EventName
        {
            get;
            set;
        }
    }
}