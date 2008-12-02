// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using Microsoft.Silverlight.Testing.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A factory for creating the unit test objects.
    /// </summary>
    public class UnitTestLogicFactory
    {
        /// <summary>
        /// The unit test harness.
        /// </summary>
        private UnitTestHarness _harness;

        /// <summary>
        /// Initializes a new unit test logic factory.
        /// </summary>
        /// <param name="harness">The unit test harness reference.</param>
        public UnitTestLogicFactory(UnitTestHarness harness)
        {
            _harness = harness;
        }

        /// <summary>
        /// Creates a new AssemblyManager.
        /// </summary>
        /// <param name="provider">The unit test provider.</param>
        /// <param name="filter">The run filter.</param>
        /// <param name="assembly">The unit test assembly metadata object.</param>
        /// <returns>Returns a new AssemblyManager.</returns>
        public AssemblyManager CreateAssemblyManager(IUnitTestProvider provider, TestRunFilter filter, IAssembly assembly)
        {
            return new AssemblyManager(filter, _harness, provider, assembly);
        }

        /// <summary>
        /// Creates a new TestClassManager.
        /// </summary>
        /// <param name="provider">The unit test provider.</param>
        /// <param name="filter">The run filter.</param>
        /// <param name="testClass">The test class metadata.</param>
        /// <param name="instance">The test class instance.</param>
        /// <returns>Returns a new TestClassManager.</returns>
        public TestClassManager CreateTestClassManager(IUnitTestProvider provider, TestRunFilter filter, ITestClass testClass, object instance)
        {
            return new TestClassManager(filter, _harness, testClass, instance, provider);
        }

        /// <summary>
        /// Creates a new TestMethodManager.
        /// </summary>
        /// <param name="provider">The unit test provider.</param>
        /// <param name="testClass">The test class metadata.</param>
        /// <param name="method">The test method metadata.</param>
        /// <param name="instance">The test class instance.</param>
        /// <returns>Returns a new TestMethodManager.</returns>
        public TestMethodManager CreateTestMethodManager(IUnitTestProvider provider, ITestClass testClass, ITestMethod method, object instance)
        {
            return new TestMethodManager(_harness, testClass, method, instance, provider);
        }
    }
}