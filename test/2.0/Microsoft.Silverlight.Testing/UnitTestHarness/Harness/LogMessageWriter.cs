// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A utility type that writes new log messages to the test harness log
    /// queue.
    /// </summary>
    public class LogMessageWriter
    {
        /// <summary>
        /// The factory used for creating new LogMessage types.
        /// </summary>
        private LogMessageFactory _factory;
        
        /// <summary>
        /// The test harness that contains the method to post new events into
        /// the log message queue.
        /// </summary>
        private ITestHarness _testHarness;

        /// <summary>
        /// Initialize a new writer class, using the default LogMessageFactory
        /// to create new messages.
        /// </summary>
        /// <param name="harness">The test harness instance.</param>
        public LogMessageWriter(ITestHarness harness)
            : this(harness, new LogMessageFactory())
        {
        }

        /// <summary>
        /// Initialize a new writer class.
        /// </summary>
        /// <param name="harness">The test harness instance.</param>
        /// <param name="messageFactory">
        /// The factory to use when creating new messages.
        /// </param>
        public LogMessageWriter(ITestHarness harness, LogMessageFactory messageFactory)
        {
            if (harness == null)
            {
                throw new ArgumentNullException("harness");
            }
            else if (messageFactory == null)
            {
                throw new ArgumentNullException("messageFactory");
            }

            _testHarness = harness;
            _factory = messageFactory;
        }

        /// <summary>
        /// Posts a log message to the test harness queue for processing.
        /// </summary>
        /// <param name="message">The log message object.</param>
        public void Enqueue(LogMessage message)
        {
            _testHarness.QueueLogMessage(message);
        }

        /// <summary>
        /// Creates a new log message using the embedded factory.
        /// </summary>
        /// <returns>Returns a new LogMessage instance.</returns>
        protected LogMessage Create()
        {
            return _factory.Create();
        }

        /// <summary>
        /// Creates a new log message using the embedded factory.
        /// </summary>
        /// <param name="messageType">The message type.</param>
        /// <returns>Returns a new LogMessage instance.</returns>
        protected LogMessage Create(LogMessageType messageType)
        {
            return _factory.Create(messageType);
        }

        /// <summary>
        /// Creates a new log message using the embedded factory.
        /// </summary>
        /// <param name="messageType">The message type.</param>
        /// <param name="message">The text message.</param>
        /// <returns>Returns a new LogMessage instance.</returns>
        protected LogMessage Create(LogMessageType messageType, string message)
        {
            LogMessage m = Create(messageType);
            m.Message = message;
            return m;
        }

        #region Decorate log messages
        /// <summary>
        /// Decorate a log message with a value.
        /// </summary>
        /// <param name="message">The log message to decorate.</param>
        /// <param name="key">The key for this decoration.</param>
        /// <param name="value">The value of this decoration.</param>
        protected static void Decorate(LogMessage message, object key, object value)
        {
            message.Decorators[key] = value;
        }

        /// <summary>
        /// Decorate the log message object with an Exception object.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="exception">The Exception.</param>
        protected static void DecorateException(LogMessage message, Exception exception)
        {
            Decorate(message, LogDecorator.ExceptionObject, exception);
        }

        /// <summary>
        /// Decorate the log message object with a name.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="name">Name property value.</param>
        protected static void DecorateNameProperty(LogMessage message, string name)
        {
            Decorate(message, LogDecorator.NameProperty, name);
        }

        /// <summary>
        /// Decorate the log message object with a test stage value.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="stage">Test stage value.</param>
        protected static void DecorateTestStage(LogMessage message, TestStage stage)
        {
            Decorate(message, LogDecorator.TestStage, stage);
        }

        /// <summary>
        /// Decorate the log message object with a test outcome object.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="outcome">Test outcome object.</param>
        protected static void DecorateTestOutcome(LogMessage message, TestOutcome outcome)
        {
            Decorate(message, LogDecorator.TestOutcome, outcome);
        }

        /// <summary>
        /// Decorate the log message object with a test granularity object.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="granularity">Test granularity object.</param>
        protected static void DecorateTestGranularity(LogMessage message, TestGranularity granularity)
        {
            Decorate(message, LogDecorator.TestGranularity, granularity);
        }

        /// <summary>
        /// Sets the type of the log message.
        /// </summary>
        /// <param name="message">The log message object.</param>
        /// <param name="type">The new value to set the message type to.</param>
        protected static void SetType(LogMessage message, LogMessageType type)
        {
            message.MessageType = type;
        }
        #endregion Decorate log messages

        #region Write to the log
        /// <summary>
        /// This writes a new line of information similar to a Debug::WriteLine
        /// call.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void DebugWriteLine(string text)
        {
            Enqueue(Create(LogMessageType.Debug, text));
        }

        /// <summary>
        /// Writes information through a new log message.
        /// </summary>
        /// <param name="info">The information string.</param>
        public void Information(string info)
        {
            Enqueue(Create(LogMessageType.Information, info));
        }

        /// <summary>
        /// Writes a message relating to the test infrastructure.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void TestInfrastructure(string text)
        {
            Enqueue(Create(LogMessageType.TestInfrastructure, text));
        }

        /// <summary>
        /// Writes an environment message.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void Environment(string text)
        {
            Enqueue(Create(LogMessageType.Environment, text));
        }

        /// <summary>
        /// Writes a TestRun message.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void TestExecution(string text)
        {
            Enqueue(Create(LogMessageType.TestExecution, text));
        }

        /// <summary>
        /// Log an error message.
        /// </summary>
        /// <param name="errorMessage">The error message string.</param>
        /// <param name="exception">The Exception object to decorate the message
        /// with.</param>
        public void Error(string errorMessage, Exception exception)
        {
            LogMessage message = Create(LogMessageType.Error, errorMessage);
            DecorateException(message, exception);
            Enqueue(message);
        }

        /// <summary>
        /// Log an error message.
        /// </summary>
        /// <param name="errorMessage">The error message string.</param>
        public void Error(string errorMessage)
        {
            Enqueue(Create(LogMessageType.Error, errorMessage));
        }

        /// <summary>
        /// Log a warning message.
        /// </summary>
        /// <param name="warningMessage">The warning message string.</param>
        /// <param name="exception">The Exception object to decorate the message
        /// with.</param>
        public void Warning(string warningMessage, Exception exception)
        {
            LogMessage message = Create(LogMessageType.Warning, warningMessage);
            DecorateException(message, exception);
            Enqueue(message);
        }

        /// <summary>
        /// Log a warning message.
        /// </summary>
        /// <param name="warningMessage">The warning message string.</param>
        public void Warning(string warningMessage)
        {
            Enqueue(Create(LogMessageType.Warning, warningMessage));
        }

        /// <summary>
        /// Record a test outcome.
        /// </summary>
        /// <param name="message">The accompanying message.</param>
        /// <param name="outcome">The outcome value.</param>
        public void TestResult(string message, TestOutcome outcome)
        {
            LogMessage m = Create(LogMessageType.TestResult, message);
            DecorateTestOutcome(m, outcome);
        }

        /// <summary>
        /// Writes information about an encountered, known issue.
        /// </summary>
        /// <param name="issue">Information about the known issue.</param>
        public void KnownIssue(string issue)
        {
            Enqueue(Create(LogMessageType.KnownIssue, issue));
        }

        /// <summary>
        /// Records a log message that indicates a named, granular test stage 
        /// has happened.
        /// </summary>
        /// <param name="message">Any message for the log.</param>
        /// <param name="name">A name for the object or event.</param>
        /// <param name="granularity">The test granularity value.</param>
        /// <param name="stage">The test stage value.</param>
        public void GranularTestStage(string message, string name, TestGranularity granularity, TestStage stage)
        {
            LogMessage m = Create(LogMessageType.TestExecution, message);
            DecorateNameProperty(m, name);
            DecorateTestGranularity(m, granularity);
            DecorateTestStage(m, stage);
            Enqueue(m);
        }
        #endregion Write to the log
    }
}