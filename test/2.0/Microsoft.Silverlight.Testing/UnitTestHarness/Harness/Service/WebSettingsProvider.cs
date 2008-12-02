// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Xml.Linq;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A test service that reads command line settings.
    /// </summary>
    public class WebSettingsProvider : SettingsProvider
    {
        /// <summary>
        /// Name of the method MethodName_GetRunParameters.
        /// </summary>
        private const string MethodName_GetRunParameters = "getRunParameters";

        /// <summary>
        /// Initializes a new settings provider instance.
        /// </summary>
        /// <param name="testService">The test service.</param>
        public WebSettingsProvider(TestServiceProvider testService)
            : base(testService)
        {
        }

        /// <summary>
        /// Sets the title of the web page.
        /// </summary>
        /// <param name="title">The title.</param>
        private static void SetPageTitle(string title)
        {
            if (HtmlPage.IsEnabled)
            {
                HtmlPage.Document.SetProperty("title", title);
            }
        }

        /// <summary>
        /// Initialize the web settings provider.
        /// </summary>
        public override void Initialize()
        {
            SetPageTitle("Loading command line parameters...");

            string guid = TestService.UniqueTestRunIdentifier;
            if (string.IsNullOrEmpty(guid))
            {
                base.Initialize();
            }

            ((SilverlightTestService)TestService).WebService.CallMethod(MethodName_GetRunParameters, WebTestService.Dictionary("guid", guid), ReadRunParameters);
        }

        /// <summary>
        /// Read the run parameters.
        /// </summary>
        /// <param name="result">The service result.</param>
        private void ReadRunParameters(ServiceResult result)
        {
            XElement xe = result.TryGetElement();
            if (xe != null)
            {
                Dictionary<string, string> settings = xe.Descendants("option").ToTransformedDictionary((option) => option.Attribute("name").Value, (option) => option.Attribute("value").Value);
                foreach (string key in settings.Keys)
                {
                    Settings[key] = settings[key];
                }
            }

            // Clear the loading message
            SetPageTitle(string.Empty);

            // Allow the other services to initialize
            base.Initialize();
        }
    }
}