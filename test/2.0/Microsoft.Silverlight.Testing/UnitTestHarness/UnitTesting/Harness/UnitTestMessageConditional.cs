// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Helper conditional methods for unit test-specific log messages.
    /// </summary>
    public static class UnitTestMessageConditional
    {
        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool IsUnitTestEndMessage(LogMessage message)
        {
            if (!IsUnitTestMessage(message))
            {
                return false;
            }
            if (message.HasDecorator(LogDecorator.TestStage))
            {
                TestStage ts = (TestStage)message[LogDecorator.TestStage];
                return ts == TestStage.Finishing;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="decorator">The unit test decorator of interest.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static Func<LogMessage, bool> IsUnitTestStartMessage(UnitTestLogDecorator decorator)
        {
            return delegate(LogMessage message)
            {
                if (!IsUnitTestMessage(message))
                {
                    return false;
                }
                if (false == message.HasDecorator(decorator))
                {
                    return false;
                }
                if (message.HasDecorator(LogDecorator.TestStage))
                {
                    TestStage ts = (TestStage)message[LogDecorator.TestStage];
                    return ts == TestStage.Starting;
                }
                else
                {
                    return false;
                }
            };
        }

        /// <summary>
        /// Returns a value indicating whether the message is marked as a unit
        /// test system message.
        /// </summary>
        /// <param name="message">The message.</param>
        /// <returns>Returns true if the message is a unit test system-marked 
        /// message.</returns>
        private static bool IsUnitTestMessage(LogMessage message)
        {
            return message.HasDecorator(UnitTestLogDecorator.IsUnitTestMessage);
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool HasUnitTestOutcome(LogMessage message)
        {
            if (!IsUnitTestMessage(message))
            {
                return false;
            }
            return message.HasDecorator(LogDecorator.TestOutcome);
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool IsIgnoreMessage(LogMessage message)
        {
            return message.HasDecorator(UnitTestLogDecorator.IgnoreMessage);
        }

        /// <summary>
        /// Determines whether a log message has an attached TestRunFilter.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns a value indicating whether the condition is met.</returns>
        public static bool IsTestRunFilterMessage(LogMessage message)
        {
            return message.HasDecorator(UnitTestLogDecorator.TestRunFilter);
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool IsExceptionLogMessage(LogMessage message)
        {
            if (!IsUnitTestMessage(message))
            {
                return false;
            }
            return message.HasDecorator(UnitTestLogDecorator.ActualException);
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool IsIncorrectExceptionLogMessage(LogMessage message)
        {
            if (!IsUnitTestMessage(message))
            {
                return false;
            }
            return message.HasDecorator(UnitTestLogDecorator.IncorrectExceptionMessage);
        }

        /// <summary>
        /// Determines whether a log message meets a specific condition or set 
        /// of conditions.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <returns>Returns true if the condition is met.</returns>
        public static bool IsKnownBug(LogMessage message)
        {
            return (message.MessageType == LogMessageType.KnownIssue);
        }
    }
}