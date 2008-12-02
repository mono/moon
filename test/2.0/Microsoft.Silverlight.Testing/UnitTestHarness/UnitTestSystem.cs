// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// A central entry point for unit test projects and applications.
    /// </summary>
    public partial class UnitTestSystem
    {
        /// <summary>
        /// Friendly unit test system name.
        /// </summary>
        private const string SystemName = "Microsoft.Silverlight.Testing";

        /// <summary>
        /// Most recent test system instance.
        /// </summary>
        private static UnitTestSystem _system;

        /// <summary>
        /// Gets a reference to the most recently run system's test harness.
        /// </summary>
        public static ITestHarness CurrentHarness { get { return _system._harness; } }

        /// <summary>
        /// Gets a reference to the most recently run test system.
        /// </summary>
        public static UnitTestSystem CurrentSystem { get { return _system; } }

        /// <summary>
        /// Register another available unit test provider for the unit test system.
        /// </summary>
        /// <param name="provider">A unit test provider.</param>
        public static void RegisterUnitTestProvider(IUnitTestProvider provider)
        {
            if (!UnitTestProviders.Providers.Contains(provider))
            {
                UnitTestProviders.Providers.Add(provider);
            }
        }

        /// <summary>
        /// Test harness instance.
        /// </summary>
        private ITestHarness _harness;

        /// <summary>
        /// Start a new unit test run.
        /// </summary>
        /// <param name="settings">Unit test settings object.</param>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "This makes the purpose clear to test developers")]
        public void Run(UnitTestSettings settings)
        {
            // Avoid having the Run method called twice
            if (_harness != null)
            {
                return;
            }

            // Track the most recent system in use
            _system = this;

            _harness = settings.TestHarness;
            if (_harness == null)
            {
                throw new InvalidOperationException(Properties.UnitTestMessage.UnitTestSystem_Run_NoTestHarnessInSettings);
            }
            _harness.Initialize(settings);
            _harness.TestHarnessCompleted += (sender, args) => OnTestHarnessCompleted(args);
            _harness.Run();
        }

        /// <summary>
        /// Prepares the default log manager.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        public static void SetStandardLogProviders(TestHarnessSettings settings)
        {
            // Debug provider
            DebugOutputProvider debugger = new DebugOutputProvider();
            debugger.ShowAllFailures = true;
            settings.LogProviders.Add(debugger);

            // Visual Studio log provider
            try
            {
                TryAddVisualStudioLogProvider(settings);
            }
            finally
            {
            }

            PrepareCustomLogProviders(settings);
        }

        /// <summary>
        /// Tries to instantiate and initialize a VSTT provider. Requires that 
        /// XLinq is available and included in the application package.
        /// </summary>
        /// <param name="settings">The test harness settings object.</param>
        private static void TryAddVisualStudioLogProvider(TestHarnessSettings settings)
        {
            VisualStudioLogProvider trx = new VisualStudioLogProvider();
            settings.LogProviders.Add(trx);
        }

        /// <summary>
        /// Creates the default settings that would be used by the UnitTestHarness
        /// if none were specified.
        /// </summary>
        /// <returns>A new RootVisual.</returns>
        /// <remarks>Assumes the calling assembly is a test assembly.</remarks>
        public static UnitTestSettings CreateDefaultSettings()
        {
            return CreateDefaultSettings(Assembly.GetCallingAssembly());
        }

        /// <summary>
        /// A completed test harness handler.
        /// </summary>
        public event EventHandler<TestHarnessCompletedEventArgs> TestHarnessCompleted;
        
        /// <summary>
        /// Call the TestHarnessCompleted event.
        /// </summary>
        /// <param name="args">The test harness completed event arguments.</param>
        private void OnTestHarnessCompleted(TestHarnessCompletedEventArgs args)
        {
            if (TestHarnessCompleted != null)
            {
                TestHarnessCompleted(this, args);
            }
        }

        /// <summary>
        /// Create a default settings object for unit testing.
        /// </summary>
        /// <param name="callingAssembly">The assembly reflection object.</param>
        /// <returns>A unit test settings instance.</returns>
        private static UnitTestSettings CreateDefaultSettings(Assembly callingAssembly)
        {
            UnitTestSettings settings = new UnitTestSettings();
            if (callingAssembly != null)
            {
                settings.TestAssemblies.Add(callingAssembly);
            }
            SetStandardLogProviders(settings);
            settings.TestHarness = new UnitTestHarness();
            // Not a default value for now
            SetTestService(settings);
            return settings;
        }
    }
}