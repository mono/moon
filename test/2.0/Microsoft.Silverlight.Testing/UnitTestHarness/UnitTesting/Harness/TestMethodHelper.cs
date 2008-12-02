// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Helper code for TestMethod logic.
    /// </summary>
    public static class TestMethodHelper
    {
        /// <summary>
        /// A value indicating whether the warning has been logged in this run.
        /// </summary>
        private static bool _hasWarned;

        /// <summary>
        /// Check whether [Exclusive] is present on >= 1 of the methods.
        /// </summary>
        /// <param name="methods">The methods to search through.</param>
        /// <returns>True if at least one of the methods has Exclusive.</returns>
        public static bool HasExclusiveMethods(ICollection<ITestMethod> methods)
        {
            foreach (var m in methods)
            {
                if (ReflectionUtility.HasAttribute(m, typeof(ExclusiveAttribute)))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Look through the methods for the [Exclusive] attribute. If found, 
        /// remove any methods where the attribute is not present.
        /// </summary>
        /// <param name="methods">The methods to filter.</param>
        /// <param name="logWriter">The log writer object.</param>
        public static void FilterExclusiveMethods(IList<ITestMethod> methods, LogMessageWriter logWriter)
        {
            if (HasExclusiveMethods(methods) == false)
            {
                return;
            }
            if (logWriter != null && ! _hasWarned)
            {
                logWriter.Warning(Properties.UnitTestMessage.TestMethodHelper_ExclusiveMethodsInUse);
                _hasWarned = true;
            }
            List<ITestMethod> filtered = new List<ITestMethod>();
            foreach (var m in methods)
            {
                if (ReflectionUtility.HasAttribute(m, typeof(ExclusiveAttribute)))
                {
                    filtered.Add(m);
                }
            }
            methods.Replace(filtered);
        }
    }
}