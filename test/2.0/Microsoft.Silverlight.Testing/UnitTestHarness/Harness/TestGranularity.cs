// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// The hierarchy of test execution represented in more generic terms, this
    /// allows for more robust and useful actions by log providers.
    /// </summary>
    public enum TestGranularity
    {
        /// <summary>
        /// Harness-level granularity.
        /// </summary>
        Harness = 0,

        /// <summary>
        /// Group of test-level granularity.
        /// </summary>
        TestGroup = 1,

        /// <summary>
        /// Test-level granularity.
        /// </summary>
        Test = 2,

        /// <summary>
        /// Scenario-level granularity.
        /// </summary>
        TestScenario = 3,
    }
}