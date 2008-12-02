// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using ITestHarness = Microsoft.Silverlight.Testing.Harness.ITestHarness;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// Unit test providers.
    /// </summary>
    public static class UnitTestProviders
    {
        /// <summary>
        /// List of unit test providers.
        /// </summary>
        private static List<IUnitTestProvider> _providers;

        /// <summary>
        /// Static constructor that initializes the built-in unit test metadata providers.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1810:InitializeReferenceTypeStaticFieldsInline", Justification = "This is a clear place for extending the framework with additional built-in unit test providers")]
        static UnitTestProviders() 
        {
            _providers = new List<IUnitTestProvider>();

            // Visual Studio Team Test (VSTT)
            _providers.Add(new Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio.VsttProvider());
            // Additional providers can be listed here.
            // At runtime the Providers list can be extended to add additional 
            // providers as well.
        }

        /// <summary>
        /// Gets the list of Unit Test providers.
        /// </summary>
        public static IList<IUnitTestProvider> Providers
        {
            get { return _providers; }
        }

        /// <summary>
        /// Gets the unit test provider for an assembly.  The framework only 
        /// currently supports a single provider per test assembly, so if more 
        /// than one registered provider can handle the assembly, at runtime an 
        /// InvalidOperationException is thrown.
        /// </summary>
        /// <param name="harness">The test harness making the request.</param>
        /// <param name="testAssembly">Assembly reflection object.</param>
        /// <returns>The unit test provider for the test assembly.  Throws if 
        /// more than one can process the assembly.  Returns null if there is 
        /// not a provider for the assembly.</returns>
        public static IUnitTestProvider GetAssemblyProvider(ITestHarness harness, Assembly testAssembly)
        {
            List<IAssembly> assemblies = new List<IAssembly>();
            foreach (IUnitTestProvider provider in _providers)
            {
                IAssembly ia = provider.GetUnitTestAssembly(harness, testAssembly);
                ICollection<ITestClass> tests = ia.GetTestClasses();
                if (tests.Count > 0) 
                {
                    assemblies.Add(ia);
                }
            }

            if (assemblies.Count > 1)
            {
                // TODO: Resource string needed for multiple providers in one 
                // assembly
                throw new InvalidOperationException();
            }

            foreach (IAssembly a in assemblies)
            {
                return a.Provider;
            }

            // Count == 0
            return null;
        }

        /// <summary>
        /// Returns the IAssembly provider for an assembly.
        /// </summary>
        /// <param name="harness">Test harness object.</param>
        /// <param name="testAssembly">Assembly reflection object.</param>
        /// <returns>Returns null or an IAssembly instance.</returns>
        public static IAssembly GetAssemblyWrapper(ITestHarness harness, Assembly testAssembly)
        {
            IUnitTestProvider provider = GetAssemblyProvider(harness, testAssembly);
            return provider == null ? null : provider.GetUnitTestAssembly(harness, testAssembly);
        }
    }
}