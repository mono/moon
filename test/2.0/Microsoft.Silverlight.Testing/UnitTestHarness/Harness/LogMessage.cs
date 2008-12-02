// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A log message.
    /// </summary>
    public class LogMessage
    {
        /// <summary>
        /// The default log message used during initialization.
        /// </summary>
        private const LogMessageType DefaultLogMessageType = LogMessageType.Information;

        /// <summary>
        /// Gets the set of decorator instances attached to the log message.
        /// </summary>
        public DecoratorDictionary Decorators { get; private set; }

        /// <summary>
        /// Gets or sets the type of message.
        /// </summary>
        public LogMessageType MessageType { get; set; }

        /// <summary>
        /// Gets or sets the log's message.
        /// </summary>
        public string Message { get; set; }

        /// <summary>
        /// Gets or sets the decorator type for the message.
        /// </summary>
        /// <param name="decorator">
        /// The type of decorator.  Only one explicit Type is permitted,
        /// although multiple types within the same type hierarchy are
        /// legitimate.
        /// </param>
        /// <returns>Returns the instance of the decorator, if any.</returns>
        public object this[object decorator]
        {
            get { return Decorators[decorator]; }
            set { Decorators[decorator] = value; }
        }

        /// <summary>
        /// Initializes a new log message of the default message type.
        /// </summary>
        public LogMessage()
            : this(DefaultLogMessageType)
        {
        }

        /// <summary>
        /// Initializes a new log message.
        /// </summary>
        /// <param name="messageType">The message type.</param>
        public LogMessage(LogMessageType messageType)
        {
            Decorators = new DecoratorDictionary();
            MessageType = messageType;
        }

        /// <summary>
        /// Converts to string representation.
        /// </summary>
        /// <returns>A string version of the LogMessage.</returns>
        public override string ToString()
        {
            return string.Format(CultureInfo.InvariantCulture, "{0}: {1}", MessageType, Message);
        }

        /// <summary>
        /// Check if a decorator is present on the LogMessage.
        /// </summary>
        /// <param name="decorator">The decorator of interest.</param>
        /// <returns>
        /// Returns a value indicating whether the decorator is present in the
        /// DecoratorDictionary.
        /// </returns>
        public bool HasDecorator(object decorator)
        {
            return HasDecorators(decorator);
        }

        /// <summary>
        /// Check if a set of decorators are present.
        /// </summary>
        /// <param name="decorators">The decorator(s) of interest.</param>
        /// <returns>
        /// Returns a value indicating whether the decorator(s) of interest are
        /// present.
        /// </returns>
        public bool HasDecorators(params object[] decorators)
        {
            return Decorators.HasDecorators(decorators);
        }
    }
}