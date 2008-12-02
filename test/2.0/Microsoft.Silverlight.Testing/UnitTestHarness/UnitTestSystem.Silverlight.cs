// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Reflection;
using System.Diagnostics;
using System.Windows;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Harness.Service;
using Microsoft.Silverlight.Testing.UI;
using Microsoft.Silverlight.Testing.UnitTesting.UI;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// A central entry point for unit test projects and applications.
    /// </summary>
    public partial class UnitTestSystem
    {
#if SILVERLIGHT
        /// <summary>
        /// A partial method for PrepareDefaultLogManager.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        public static void PrepareCustomLogProviders(TestHarnessSettings settings)
        {
            if (HtmlPage.IsEnabled)
            {
                settings.LogProviders.Add(new WebpageHeaderLogProvider(SystemName));
                settings.LogProviders.Add(new UnitTestWebpageLog());
                settings.LogProviders.Add(new TextFailuresLogProvider());
            }
        }
#else
        /// <summary>
        /// A partial method for PrepareDefaultLogManager.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        private static void PrepareDefaultLogManagerOptional(TestHarnessSettings settings)
        {
        }
#endif

#if SILVERLIGHT
        /// <summary>
        /// A partial method for setting the TestService.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        public static void SetTestService(TestHarnessSettings settings)
        {
            settings.TestService = new SilverlightTestService();
        }
#else
        /// <summary>
        /// A partial method for setting the TestService.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        private static void SetTestService(TestHarnessSettings settings)
        {
        }
#endif

        /// <summary>
        /// Creates a new TestPage visual that in turn will setup and begin a 
        /// unit test run.
        /// </summary>
        /// <returns>A new RootVisual.</returns>
        /// <remarks>Assumes the calling assembly is a test assembly.</remarks>
        public static UIElement CreateTestPage()
        {
            UnitTestSettings settings = CreateDefaultSettings(Assembly.GetCallingAssembly());
            return CreateTestPage(settings);
        }

        /// <summary>
        /// Creates a new TestPage visual that in turn will setup and begin a 
        /// unit test run.
        /// </summary>
        /// <param name="settings">Test harness settings to be applied.</param>
        /// <returns>A new RootVisual.</returns>
        /// <remarks>Assumes the calling assembly is a test assembly.</remarks>
        public static UIElement CreateTestPage(UnitTestSettings settings)
        {
            UnitTestSystem system = new UnitTestSystem();
            PrepareTestService(settings, () => system.Run(settings));
            return new TestPage();
        }

        /// <summary>
        /// Merge any settings provided by a test service with the parameters 
        /// that were passed inside the TestHarnessSettings.
        /// </summary>
        /// <param name="testService">The test service.</param>
        /// <param name="inputSettings">The run settings.</param>
        private static void MergeSettingsAndParameters(TestServiceProvider testService, UnitTestSettings inputSettings)
        {
            if (testService != null && testService.HasService(TestServiceFeature.RunSettings))
            {
                SettingsProvider settings = testService.GetService<SettingsProvider>(TestServiceFeature.RunSettings);
                foreach (string key in settings.Settings.Keys)
                {
                    if (inputSettings.Parameters.ContainsKey(key))
                    {
                        Debug.WriteLine("MergeSettingsAndParameters: Overwriting " + key + " key during merge.");
                    }
                    inputSettings.Parameters[key] = settings.Settings[key];
                }
            }
        }

        /// <summary>
        /// Initializes the test service and its contained providers.
        /// </summary>
        /// <param name="inputSettings">The run settings.</param>
        /// <param name="complete">Action to call once the test service is 
        /// initialized and ready to continue the run's execution.</param>
        private static void PrepareTestService(UnitTestSettings inputSettings, Action complete)
        {
            TestServiceProvider testService = inputSettings.TestService;
            if (testService != null && testService.Initialized == false)
            {
                Action after = delegate
                {
                    MergeSettingsAndParameters(testService, inputSettings);
                    complete();
                };
                testService.InitializeCompleted += delegate(object sender, EventArgs e) { after(); };
                testService.Initialize();
            }
            else
            {
                complete();
            }
        }
    }
}