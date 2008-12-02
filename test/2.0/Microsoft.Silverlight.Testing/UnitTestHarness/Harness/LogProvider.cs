// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// LogProvider interface with a single Process method that handles a
    /// LogMessage object.
    /// </summary>
    public abstract class LogProvider
    {
        /// <summary>
        /// A dictionary of conditional action handlers.
        /// </summary>
        private Dictionary<Func<LogMessage, bool>, Action<LogMessage>> _conditionalHandlers = new Dictionary<Func<LogMessage, bool>, Action<LogMessage>>();

        /// <summary>
        /// A dictionary of types/actions for handling specific types of log
        /// messages.
        /// </summary>
        private Dictionary<LogMessageType, Action<LogMessage>> _definedHandlers = new Dictionary<LogMessageType, Action<LogMessage>>();

        /// <summary>
        /// Perform any needed operations to log the message.
        /// </summary>
        /// <param name="logMessage">Instance of LogMessage type.</param>
        public virtual void Process(LogMessage logMessage)
        {
            // 1st: The more advanced conditionals.
            foreach (KeyValuePair<Func<LogMessage, bool>, Action<LogMessage>> pair in _conditionalHandlers)
            {
                if (pair.Value != null && pair.Key(logMessage))
                {
                    pair.Value(logMessage);
                    return;
                }
            }

            // 2nd: The LogMessageType handlers.
            LogMessageType type = logMessage.MessageType;
            Action<LogMessage> action = null;
            if (_definedHandlers.TryGetValue(type, out action) && action != null)
            {
                action(logMessage);
                return;
            }

            // 3rd: Do nothing with the message, unless overloaded
            ProcessRemainder(logMessage);
        }

        /// <summary>
        /// Method that processes any messages not handled any other way.
        /// </summary>
        /// <param name="message">The log message.</param>
        protected virtual void ProcessRemainder(LogMessage message)
        {
        }

        /// <summary>
        /// Registers an action for a specific message type.
        /// </summary>
        /// <param name="type">The type of interest.</param>
        /// <param name="action">The handler for the type.  Takes a LogMessage 
        /// parameter.</param>
        protected void RegisterMessageTypeHandler(LogMessageType type, Action<LogMessage> action)
        {
            _definedHandlers.Add(type, action);
        }

        /// <summary>
        /// Registers a conditional handler.  During the log message processing 
        /// step, all conditional callbacks will be tried.  The first positive 
        /// result will then call the associated processing Action for that 
        /// conditional method entry.
        /// </summary>
        /// <param name="condition">A conditional callback that takes a 
        /// LogMessage input parameter.</param>
        /// <param name="action">A log message processing Action that is called 
        /// when the condition is true.</param>
        protected void RegisterConditionalHandler(Func<LogMessage, bool> condition, Action<LogMessage> action)
        {
            _conditionalHandlers.Add(condition, action);
        }

        /// <summary>
        /// Removes a conditional callback.
        /// </summary>
        /// <param name="condition">The condition.</param>
        protected void UnregisterConditionalHandler(Func<LogMessage, bool> condition)
        {
            _conditionalHandlers.Remove(condition);
        }

        /// <summary>
        /// Clear all existing conditional handlers.
        /// </summary>
        protected void ClearConditionalHandlers()
        {
            _conditionalHandlers.Clear();
        }

        /// <summary>
        /// Clear all existing message type handlers.
        /// </summary>
        protected void ClearMessageTypeHandlers()
        {
            _definedHandlers.Clear();
        }
    }
}