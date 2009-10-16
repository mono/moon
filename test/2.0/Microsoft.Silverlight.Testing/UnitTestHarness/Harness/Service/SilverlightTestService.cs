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
        /// Gets the service type that is in use.
        /// </summary>
        public ServiceType ServiceType { get; private set; }

        /// <summary>
        /// The service verifier and information.
        /// </summary>
        private ServiceVerifier _webService;

        /// <summary>
        /// Gets the web service proxy.
        /// </summary>
        public WebTestService WebService { get; private set; }

        /// <summary>
        /// Initializes a new SilverlightTestServiceProvider.
        /// </summary>
        public SilverlightTestService() : base()
        {
            if (HtmlPage.IsEnabled && HtmlPage.Document != null && HtmlPage.Document.DocumentUri != null)
            {
                Uri uri = HtmlPage.Document.DocumentUri;
                switch (uri.Scheme)
                {
                    case "file":
                        ServiceType = ServiceType.Direct;
                        break;

                    case "http":
                        ServiceType = ServiceType.WebService;
                        SetServicePath();
                        SetCustomIdentification();
                        break;

                    // Default: unknowns plus https://
                    default:
                        ServiceType = ServiceType.None;
                        // TODO: Provide a non-fatal warning here
                        break;
                }
            }
        }

        /// <summary>
        /// Initializes the Silverlight test service.  Performs a service check 
        /// if needed before initializing the other providers.
        /// </summary>
        public override void Initialize()
        {
            if (ServiceType.WebService == ServiceType)
            {
                AttemptServiceConnection();
            }
            else
            {
                ContinueInitialization();
            }
        }

        /// <summary>
        /// Sets the custom ID information for the test run, if passed into 
        /// the run.
        /// </summary>
        public void SetCustomIdentification()
        {
            string testRun = "test_run";
            char[] seps = { '&', '?' };

            Uri page = HtmlPage.Document.DocumentUri;
            if (page.Query.Contains(testRun))
            {
                string rest = page.Query.Substring(page.Query.IndexOf(testRun, StringComparison.OrdinalIgnoreCase) + testRun.Length + 1);
                int after = rest.IndexOfAny(seps);
                if (after >= 0)
                {
                    rest = rest.Substring(0, after);
                }
                UniqueTestRunIdentifier = rest;
            }
        }

        /// <summary>
        /// Determine the service path to attempt to use, and prepares the 
        /// verification object using those parameters.
        /// </summary>
        private void SetServicePath()
        {
            // CONSIDER: May want to allow test settings to provide different 
            // values for the service address, for additional flexibility and 
            // security.
            _webService = new ServiceVerifier
            {
               Hostname = "localhost",
               Port = 8000,
               ServicePath = "/externalInterface/",
            };
        }

        /// <summary>
        /// Pauses the initialization process to attempt a service connection. 
        /// The result will alter the underlying ServiceType being used by 
        /// this provider to ensure a fallback experience can be used.  
        /// 
        /// This verification step will block the initialization and entire 
        /// test run until it continues.
        /// </summary>
        private void AttemptServiceConnection()
        {
            _webService.Verify(
                // Success
                delegate
                {
                    WebService = new WebTestService(_webService.ServiceUri);
                    ContinueInitialization();
                },
                // Failure, fallback to the direct mode
                delegate
                {
                    ServiceType = ServiceType.Direct;
                    _webService = null;
                    ContinueInitialization();
                });
        }

        /// <summary>
        /// Continues the initialization process for the test service provider.
        /// </summary>
        private void ContinueInitialization()
        {
            PopulateProviders();

            // Base initializes the service providers and then fires off the 
            // complete event
            base.Initialize();
        }

        /// <summary>
        /// Populates with the standard providers for Silverlight in-browser 
        /// testing.
        /// </summary>
        private void PopulateProviders()
        {
            if (ServiceType.Direct == ServiceType)
            {
                // Settings provider
		// IsolatedStorageSettingsProvider settings = new IsolatedStorageSettingsProvider(this);
		// RegisterService(TestServiceFeature.RunSettings, settings);
            }

            if (ServiceType.WebService == ServiceType)
            {
                // Command line run settings provider
                SettingsProvider settings = new WebSettingsProvider(this);
                RegisterService(TestServiceFeature.RunSettings, settings);

                // Code coverage provider
                CodeCoverageProvider coverage = new WebCodeCoverageProvider(this);
                RegisterService(TestServiceFeature.CodeCoverageReporting, coverage);

                // Reporting provider
                TestReportingProvider reporting = new WebTestReportingProvider(this);
                RegisterService(TestServiceFeature.TestReporting, reporting);

                // Environment provider
                EnvironmentProvider environment = new WebEnvironmentProvider(this);
                RegisterService(TestServiceFeature.EnvironmentServices, environment);
            }
        }
    }
}
