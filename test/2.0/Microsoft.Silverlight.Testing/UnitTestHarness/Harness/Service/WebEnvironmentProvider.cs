// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Net;
using System.Threading;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A provider of environment variables and environmental information that 
    /// uses the test service provider infrastructure.
    /// </summary>
    public class WebEnvironmentProvider : EnvironmentProvider
    {
        /// <summary>
        /// The MethodName_GetEnvironmentVariable method name.
        /// </summary>
        private const string MethodName_GetEnvironmentVariable = "getEnvironmentVariable";

        /// <summary>
        /// Initializes a new environment provider.
        /// </summary>
        /// <param name="testService">The web test service.</param>
        public WebEnvironmentProvider(SilverlightTestService testService)
            : base(testService)
        {
        }

        /// <summary>
        /// Retrieve an environment variable from the system.
        /// </summary>
        /// <param name="name">The variable name.</param>
        /// <param name="callback">The callback action.</param>
        public override void GetEnvironmentVariable(string name, Action<ServiceResult> callback)
        {
            ((SilverlightTestService)TestService).WebService.CallMethod(MethodName_GetEnvironmentVariable, WebTestService.Dictionary("name", name), callback);
        }
    }
}