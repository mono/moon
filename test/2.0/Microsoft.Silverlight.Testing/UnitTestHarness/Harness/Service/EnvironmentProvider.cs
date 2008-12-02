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
    public class EnvironmentProvider : ProviderBase
    {
        /// <summary>
        /// Initializes a new environment provider.
        /// </summary>
        /// <param name="testService">The test service.</param>
        public EnvironmentProvider(TestServiceProvider testService)
            : base(testService, "Environment")
        {
        }

        /// <summary>
        /// Retrieve an environment variable from the system.
        /// </summary>
        /// <param name="name">The variable name.</param>
        /// <param name="callback">The callback action.</param>
        public virtual void GetEnvironmentVariable(string name, Action<ServiceResult> callback)
        {
            Callback(callback, ServiceResult.CreateExceptionalResult(new NotSupportedException("Environment is not yet implemented.")));
        }
    }
}