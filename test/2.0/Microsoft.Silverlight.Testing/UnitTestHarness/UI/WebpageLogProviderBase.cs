// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// Base class for Html-based web page providers, with utility methods 
    /// and optional style sheet injection.
    /// </summary>
    public abstract class WebpageLogProviderBase : LogProvider, ITestSettingsLogProvider
    {
        /// <summary>
        /// The key name for changing the resize behavior.
        /// </summary>
        private const string EnablePluginResizeKeyName = "enablePluginResize";

        /// <summary>
        /// The key name for a custom test column width.
        /// </summary>
        private const string TestColumnWidthKeyName = "testColumnWidth";

        /// <summary>
        /// Base constructor for the WebPageProvider.
        /// </summary>
        protected WebpageLogProviderBase()
        {
            EnablePluginResize = true;
            TestColumnWidth = WebBrowserTestPage.DefaultTestColumnWidth;
        }

        /// <summary>
        /// Initializes the test harness.
        /// </summary>
        /// <param name="settings">The test harness settings object.</param>
        public virtual void Initialize(TestHarnessSettings settings)
        {
            // Look in the settings for the resize override
            string resizeString;
            if (settings.Parameters.TryGetValue(EnablePluginResizeKeyName, out resizeString))
            {
                bool value;
                if (bool.TryParse(resizeString, out value))
                {
                    EnablePluginResize = value;
                }
            }

            string widthString;
            if (settings.Parameters.TryGetValue(TestColumnWidthKeyName, out widthString))
            {
                int value;
                if (int.TryParse(widthString, out value))
                {
                    TestColumnWidth = value;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating what width to use for the HTML log.
        /// </summary>
        protected static int TestColumnWidth { get; private set; }

        /// <summary>
        /// Gets a value indicating whether the plugin should be resized.
        /// </summary>
        protected static bool EnablePluginResize { get; private set; }

        /// <summary>
        /// The test page control.
        /// </summary>
        private static WebBrowserTestPage _testPage;

        /// <summary>
        /// Gets the test page control.  There is only one (static) instance per 
        /// application/plugin.
        /// </summary>
        public static WebBrowserTestPage TestPage
        {
            get
            {
                if (_testPage == null)
                {
                    _testPage = new WebBrowserTestPage(WebBrowserTestPage.DefaultMinimumPluginWidth, WebBrowserTestPage.DefaultMinimumPluginHeight, TestColumnWidth, EnablePluginResize);
                }
                return _testPage;
            }
        }

        /// <summary>
        /// Gets the test column element.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Provided here so that the static, single-column instance could be removed in the future easily.")]
        protected HtmlTestColumn TestColumn
        {
            get 
            {
                return TestPage.TestColumn;
            }
        }
    }
}