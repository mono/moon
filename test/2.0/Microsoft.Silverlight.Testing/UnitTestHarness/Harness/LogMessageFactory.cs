// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A simple factory used for creating new log messages.
    /// </summary>
    public class LogMessageFactory
    {
        /// <summary>
        /// Gets or sets the default log message type used for the creation of
        /// unspecific log message types.
        /// </summary>
        protected LogMessageType DefaultLogMessageType { get; set; }

        /// <summary>
        /// Create a new LogMessage instance.
        /// </summary>
        /// <returns>Returns a new LogMessage instance.</returns>
        public LogMessage Create()
        {
            return Create(DefaultLogMessageType);
        }

        /// <summary>
        /// Create a new LogMessage instance.
        /// </summary>
        /// <param name="messageType">The type of message to create.</param>
        /// <returns>Returns a new LogMessage instance.</returns>
        public virtual LogMessage Create(LogMessageType messageType)
        {
            return new LogMessage(messageType);
        }
    }
}