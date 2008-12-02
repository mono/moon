// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A type filter for just a specific test.  Allows the re-running of a 
    /// single result again in the same process.
    /// </summary>
    public class RetryTestRunFilter : TestRunFilter
    {
        /// <summary>
        /// The test class.
        /// </summary>
        private ITestClass _test;

        /// <summary>
        /// The test method.
        /// </summary>
        private ITestMethod _method;

        /// <summary>
        /// Initializes a new test run filter using an existing settings file.
        /// </summary>
        /// <param name="test">The test class metadata.</param>
        /// <param name="method">The test method metadata.</param>
        public RetryTestRunFilter(ITestClass test, ITestMethod method) : base(null, null)
        {
            TestRunName = "Retry of " + test.Name + " " + method.Name;
            
            _test = test;
            _method = method;
        }

        /// <summary>
        /// Retrieve a set of test classes from a test assembly.
        /// </summary>
        /// <param name="assembly">The test assembly metadata object.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        /// <returns>Returns a new list of test class metadata objects.</returns>
        [SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Although exposed as a public type, this is intended for internal framework consumption only.")]
        public override List<ITestClass> GetTestClasses(IAssembly assembly, TestClassInstanceDictionary instances)
        {
            return new List<ITestClass>
            {
                _test
            };
        }

        /// <summary>
        /// Retrieves the test methods from a test class metadata object.
        /// </summary>
        /// <param name="test">The test class metadata object.</param>
        /// <param name="instance">The test class instance.</param>
        /// <returns>Returns a list of test method metadata objects.</returns>
        [SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Although exposed as a public type, this is intended for internal framework consumption only.")]
        public override List<ITestMethod> GetTestMethods(ITestClass test, object instance)
        {
            return new List<ITestMethod>
            {
                _method
            };
        }
    }
}