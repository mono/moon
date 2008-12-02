// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Net;
using System.Threading;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A provider of code coverage information to an external process.
    /// </summary>
    public class WebCodeCoverageProvider : CodeCoverageProvider
    {
        /// <summary>
        /// The MethodName_SaveCodeCoverage method name.
        /// </summary>
        private const string MethodName_SaveCodeCoverage = "saveCodeCoverage";

        /// <summary>
        /// Initializes a new code coverage provider.
        /// </summary>
        /// <param name="testService">The test service.</param>
        public WebCodeCoverageProvider(TestServiceProvider testService) 
            : base(testService)
        {
        }

        /// <summary>
        /// Save string-based code coverage data.
        /// </summary>
        /// <param name="data">The code coverage data, as a string.</param>
        /// <param name="callback">The callback action.</param>
        public override void SaveCoverageData(string data, Action<ServiceResult> callback)
        {
            string guid = TestService.UniqueTestRunIdentifier;
            Dictionary<string, string> parameters = string.IsNullOrEmpty(guid) ? WebTestService.Dictionary() : WebTestService.Dictionary("guid", guid);
            ((SilverlightTestService)TestService).WebService.CallMethod(MethodName_SaveCodeCoverage, parameters, data, callback);
        }
    }
}