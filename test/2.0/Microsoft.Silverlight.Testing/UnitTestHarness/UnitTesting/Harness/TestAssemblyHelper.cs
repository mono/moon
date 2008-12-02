// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Helper code for TestAssembly logic.
    /// </summary>
    public static class TestAssemblyHelper
    {
        /// <summary>
        /// Check whether [Exclusive] attribute is present on any classes.
        /// </summary>
        /// <param name="classes">Collection of class metadata objects.</param>
        /// <returns>Returns a value indicating whether any of the classes 
        /// include an [Exclusive] attribute.</returns>
        public static bool HasExclusiveClasses(IList<ITestClass> classes)
        {
            foreach (var cl in classes)
            {
                if (ReflectionUtility.HasAttribute(cl.Type, typeof(ExclusiveAttribute)))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Sorts the test classes alphabetically by name.
        /// </summary>
        /// <param name="classes">A list of test class metadata objects.</param>
        public static void SortTestClasses(IList<ITestClass> classes)
        {
            classes.Replace(classes.OrderBy(a => a.Name));
        }
    }
}