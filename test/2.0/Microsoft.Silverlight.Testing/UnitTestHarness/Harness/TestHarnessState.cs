// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Base class representing the overall state of a test run.
    /// </summary>
    public class TestHarnessState
    {
        /// <summary>
        /// Gets the count of failing scenarios.
        /// </summary>
        public int Failures { get; protected set; }

        /// <summary>
        /// Gets the total count of scenarios run.
        /// </summary>
        public int TotalScenarios { get; protected set; }

        /// <summary>
        /// Gets a value indicating whether the status recorded indicates a
        /// failure.
        /// </summary>
        public bool Failed { get { return Failures > 0; } }

        /// <summary>
        /// Increment the failures counter.
        /// </summary>
        public void IncrementFailures()
        {
            Failures++;
        }

        /// <summary>
        /// Increments the total scenarios counter.
        /// </summary>
        public void IncrementTotalScenarios()
        {
            TotalScenarios++;
        }
    }
}