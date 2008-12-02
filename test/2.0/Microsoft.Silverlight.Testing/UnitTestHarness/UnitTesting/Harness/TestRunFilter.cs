// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A type to filter down complete sets of classes, tests and methods.
    /// </summary>
    public class TestRunFilter
    {
        /// <summary>
        /// The default unit test run name.
        /// </summary>
        private const string DefaultTestRunName = "Unit Test Run";

        /// <summary>
        /// Unit test settings.
        /// </summary>
        private UnitTestSettings _settings;

        /// <summary>
        /// Initializes a new test run filter using an existing settings file.
        /// </summary>
        /// <param name="settings">A unit test settings instance.</param>
        /// <param name="harness">The unit test harness.</param>
        public TestRunFilter(UnitTestSettings settings, UnitTestHarness harness)
        {
            TestRunName = DefaultTestRunName;
            _settings = settings;
            UnitTestHarness = harness;
        }

        /// <summary>
        /// Gets a friendly name for the test run.
        /// </summary>
        public string TestRunName { get; protected set; }

        /// <summary>
        /// Gets the unit test harness.
        /// </summary>
        protected UnitTestHarness UnitTestHarness { get; private set; }

        /// <summary>
        /// Retrieve a set of test classes from a test assembly.
        /// </summary>
        /// <param name="assembly">The test assembly metadata object.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        /// <returns>Returns a new list of test class metadata objects.</returns>
        [SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Although exposed as a public type, this is intended for internal framework consumption only.")]
        public virtual List<ITestClass> GetTestClasses(IAssembly assembly, TestClassInstanceDictionary instances)
        {
            List<ITestClass> classes = new List<ITestClass>(assembly.GetTestClasses());
            FilterTestClasses(classes, instances);
            SortTestClasses(classes);
            return classes;
        }

        /// <summary>
        /// Sort the test classes if the settings for alphabetical sorting are 
        /// present.
        /// </summary>
        /// <param name="tests">List of test classes.</param>
        protected void SortTestClasses(IList<ITestClass> tests)
        {
            if (_settings != null)
            {
                TestAssemblyHelper.SortTestClasses(tests);
            }
        }

        /// <summary>
        /// Filter out tests based on the standard-supported methods.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected void FilterTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
            FilterExclusiveTestClasses(classes, instances);
            FilterLegacyTestClasses(classes, instances);
            FilterCustomTestClasses(classes, instances);
        }

        /// <summary>
        /// Perform any custom filtering that the TestRunFilter needs.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected virtual void FilterCustomTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
        }

        /// <summary>
        /// If specific string-contains filters are present.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected virtual void FilterLegacyTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
            if (_settings != null && _settings.TestClassesToRun != null)
            {
                TestClassHelper.FilterTestsToRun(classes, _settings.TestClassesToRun);
            }
            if (_settings != null && _settings.Parameters != null)
            {
                TestClassHelper.FilterByFilterClassParameter(_settings.Parameters, classes);
            }
        }

        /// <summary>
        /// If any exclusive classes are found, filter them.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected virtual void FilterExclusiveTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
            TestClassHelper.FilterExclusiveClasses(classes, UnitTestHarness.LogWriter);
        }

        /// <summary>
        /// Retrieves the test methods from a test class metadata object.
        /// </summary>
        /// <param name="test">The test class metadata object.</param>
        /// <param name="instance">The test class instance.</param>
        /// <returns>Returns a list of test method metadata objects.</returns>
        [SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Although exposed as a public type, this is intended for internal framework consumption only.")]
        public virtual List<ITestMethod> GetTestMethods(ITestClass test, object instance)
        {
            List<ITestMethod> methods = new List<ITestMethod>(test.GetTestMethods());

            // Get any dynamically provided test methods
            IProvideDynamicTestMethods provider = instance as IProvideDynamicTestMethods;
            if (provider != null)
            {
                IEnumerable<ITestMethod> dynamicTestMethods = provider.GetDynamicTestMethods();
                if (dynamicTestMethods != null)
                {
                    methods.AddRange(dynamicTestMethods);
                }
            }

            FilterTestMethods(methods);
            SortTestMethods(methods);
            return methods;
        }

        /// <summary>
        /// Filter the test methods.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected void FilterTestMethods(IList<ITestMethod> methods)
        {
            FilterExclusiveTestMethods(methods);
            FilterCustomTestMethods(methods);
        }

        /// <summary>
        /// Perform any custom filtering that the TestRunFilter needs.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected virtual void FilterCustomTestMethods(IList<ITestMethod> methods)
        {
        }

        /// <summary>
        /// If any exclusive classes are found, filter them.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected virtual void FilterExclusiveTestMethods(IList<ITestMethod> methods)
        {
            TestMethodHelper.FilterExclusiveMethods(methods, UnitTestHarness.LogWriter);
        }

        /// <summary>
        /// Sorts the test methods, if requested.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected void SortTestMethods(IList<ITestMethod> methods)
        {
            if (_settings.SortTestMethods)
            {
                methods.Replace(methods.OrderBy(m => m.Name));
            }
        }
    }
}