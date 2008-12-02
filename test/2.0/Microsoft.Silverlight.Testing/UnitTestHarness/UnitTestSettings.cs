// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using Microsoft.Silverlight.Testing.Harness;
using System.Collections.ObjectModel;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// Settings for the unit test system.
    /// </summary>
    public class UnitTestSettings : TestHarnessSettings
    {
        /// <summary>
        /// The tag expression key name.
        /// </summary>
        private const string TagExpressionKey = "tagExpression";

        /// <summary>
        /// By default test methods are sorted.
        /// </summary>
        private const bool DefaultSortTestMethods = true;

        /// <summary>
        /// By default test classes are sorted.
        /// </summary>
        private const bool DefaultSortTestClasses = true;

        /// <summary>
        /// Settings for the unit test system.
        /// </summary>
        public UnitTestSettings() : base()
        {
            SortTestMethods = DefaultSortTestMethods;
            SortTestClasses = DefaultSortTestClasses;
            TestClassesToRun = new Collection<string>();
        }

        /// <summary>
        /// Gets or sets a value indicating whether test methods are sorted 
        /// alphabetically. By default this value is true.
        /// </summary>
        /// <remarks>
        /// It is worth understanding that the order of unit test 
        /// execution should not affect the results of a test run.  Any expected
        /// ordering and verification from multiple test methods should be 
        /// refactored into a single unit test.
        /// </remarks>
        public bool SortTestMethods { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether test classes are sorted 
        /// alphabetically. This setting is True by default.
        /// </summary>
        public bool SortTestClasses { get; set; }

        /// <summary>
        /// Gets or sets the tag expression used for selecting tests to run. 
        /// </summary>
        public string TagExpression
        {
            get 
            { 
                string value;
                if (Parameters.TryGetValue(TagExpressionKey, out value))
                {
                    return value;
                }

                return null;
            }

            set { Parameters[TagExpressionKey] = value; }
        }

        /// <summary>
        /// Gets a list of test classes to run. Enables filtering.
        /// </summary>
        /// <remarks>This property should be considered obsolete.</remarks>
        public Collection<string> TestClassesToRun { get; private set; }
    }
}