// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Interface that all test harnesses in this framework must use.  Basic
    /// enough to permit wrapping existing harnesses if needed.
    /// 
    /// Setters are provided for properties in the hope that they will be used
    /// only for enabling testability/ wrapping other harnesses/ customization
    /// points.  In general a test developer should not access the underlying
    /// TestHarness implementations or objects.
    /// </summary>
    public interface ITestHarness
    {
        /// <summary>
        /// Gets or sets the state of the harness.
        /// </summary>
        TestHarnessState State { get; set; }

        /// <summary>
        /// Event that is called after the test run is complete.
        /// </summary>
        event EventHandler<TestHarnessCompletedEventArgs> TestHarnessCompleted;

        /// <summary>
        /// Adds a log provider to the log listening routines.
        /// </summary>
        /// <param name="provider">The log provider to add.</param>
        void AddLogProvider(LogProvider provider);

        /// <summary>
        /// Queues a log message for processing by the log providers.
        /// </summary>
        /// <param name="message">The log message object.</param>
        void QueueLogMessage(LogMessage message);

        /// <summary>
        /// Allow the Harness to process initialization settings before having
        /// to run tests.
        /// </summary>
        /// <param name="settings">
        /// Settings to initialize the harness with.
        /// </param>
        void Initialize(TestHarnessSettings settings);
        
        /// <summary>
        /// Begins running the test harness.
        /// </summary>
        void Run();
    }
}