// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Xml.Linq;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A test service that reports test run results.
    /// </summary>
    public abstract class TestReportingProvider : ProviderBase
    {
        /// <summary>
        /// Initializes a new reporting provider instance.
        /// </summary>
        /// <param name="testService">The test service.</param>
        protected TestReportingProvider(TestServiceProvider testService)
            : base(testService, "TestReporting")
        {
        }

        /// <summary>
        /// Begins a call to the test service to write to the log.
        /// </summary>
        /// <param name="callback">The callback, used to read or verify results 
        /// from the service call.</param>
        /// <param name="logName">The name of the log to write.</param>
        /// <param name="content">The log file content.</param>
        public virtual void WriteLog(Action<ServiceResult> callback, string logName, string content)
        {
            Callback(callback, ServiceResult.CreateExceptionalResult(new NotSupportedException()));
        }

        /// <summary>
        /// Begins a call to the test service to report a test run's results.
        /// </summary>
        /// <param name="callback">The callback, used to read or verify results 
        /// from the service call.</param>
        /// <param name="failure">A value indicating whether the test run was a 
        /// failure.</param>
        /// <param name="failures">The failed scenario count.</param>
        /// <param name="totalScenarios">The total scenario count.</param>
        /// <param name="message">Any message to report along with the failure.</param>
        public virtual void ReportFinalResult(Action<ServiceResult> callback, bool failure, int failures, int totalScenarios, string message)
        {
            Callback(callback, ServiceResult.CreateExceptionalResult(new NotSupportedException()));
        }
    }
}