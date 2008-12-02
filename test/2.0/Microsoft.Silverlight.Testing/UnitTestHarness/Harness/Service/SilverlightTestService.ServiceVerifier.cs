// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// The Silverlight test service provider is built for compilation with 
    /// Silverlight builds of the test framework.  Populates with the important 
    /// providers for web browser-hosted test runs.
    /// </summary>
    public partial class SilverlightTestService : TestServiceProvider
    {
        /// <summary>
        /// A special verification class used by SilverlightTestService.
        /// </summary>
        private class ServiceVerifier
        {        
            /// <summary>
            /// The name of a simple 'ping' method exposed by the service.
            /// </summary>
            private const string VerificationServiceName = "ping";

            /// <summary>
            /// Gets or sets the service hostname.
            /// </summary>
            public string Hostname { get; set; }
            
            /// <summary>
            /// Gets or sets the service port.
            /// </summary>
            public int Port { get; set; }

            /// <summary>
            /// Gets or sets path to the simple POX service.
            /// </summary>
            public string ServicePath { get; set; }

            /// <summary>
            /// Gets the URI to the service.
            /// </summary>
            public Uri ServiceUri
            {
                get
                {
                    return HtmlPage.IsEnabled 
                        ? new Uri(HtmlPage.Document.DocumentUri.Scheme + "://" + Hostname + ":" + Port.ToString(CultureInfo.InvariantCulture) + ServicePath) 
                        : null;
                }
            }

            /// <summary>
            /// Attempts to verify the service connection.  Calls the proper 
            /// success/failure Action once a verification result is possible.
            /// </summary>
            /// <param name="success">The Action to call upon connection 
            /// verification.</param>
            /// <param name="failure">An Action to call upon failure.</param>
            public void Verify(Action success, Action failure)
            {
                WebTestService pox = new WebTestService(ServiceUri);
                pox.CallMethod(
                    VerificationServiceName,
                    delegate(ServiceResult result)
                    {
                        if (result.Successful)
                        {
                            success();
                        }
                        else
                        {
                            failure();
                        }
                    });
            }
        }
    }
}