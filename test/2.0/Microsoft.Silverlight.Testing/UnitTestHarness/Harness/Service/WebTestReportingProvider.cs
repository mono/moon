// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Xml.Linq;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A test service that reports test run results.
    /// </summary>
    public class WebTestReportingProvider : TestReportingProvider
    {
        /// <summary>
        /// Name of the method MethodName_ReportTestResults.
        /// </summary>
        private const string MethodName_ReportTestResults = "reportTestResults";

        /// <summary>
        /// Name of the method MethodName_WriteLog.
        /// </summary>
        private const string MethodName_WriteLog = "saveLogFile";

        /// <summary>
        /// Initializes a new reporting provider instance.
        /// </summary>
        /// <param name="testService">The test service.</param>
        public WebTestReportingProvider(TestServiceProvider testService)
            : base(testService)
        {
        }

        /// <summary>
        /// Begins a call to the test service to write to the log.
        /// </summary>
        /// <param name="callback">The callback, used to read or verify results 
        /// from the service call.</param>
        /// <param name="logName">The name of the log to write.</param>
        /// <param name="content">The log file content.</param>
        public override void WriteLog(Action<ServiceResult> callback, string logName, string content)
        {
            string guid = TestService.UniqueTestRunIdentifier;
            Dictionary<string, string> parameters = string.IsNullOrEmpty(guid) ? WebTestService.Dictionary("logName", logName) : WebTestService.Dictionary("guid", guid, "logName", logName);
            ((SilverlightTestService)TestService).WebService.CallMethod(MethodName_WriteLog, parameters, content, callback);
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
        public override void ReportFinalResult(Action<ServiceResult> callback, bool failure, int failures, int totalScenarios, string message)
        {
            string guid = TestService.UniqueTestRunIdentifier;
            if (string.IsNullOrEmpty(guid))
            {
                Callback(callback, ServiceResult.CreateExceptionalResult(new NotSupportedException("A unique identifier for the test run is required.")));
            }

            Dictionary<string, string> parameters = WebTestService.Dictionary("failure", failure.ToString(CultureInfo.InvariantCulture), "total", totalScenarios.ToString(CultureInfo.InvariantCulture), "failures", failures.ToString(CultureInfo.InvariantCulture), "guid", guid);
            ((SilverlightTestService)TestService).WebService.CallMethod(MethodName_ReportTestResults, parameters, callback);
        }
    }
}