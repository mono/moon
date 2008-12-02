// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Test class helper.
    /// </summary>
    public static class TestClassHelper
    {
        /// <summary>
        /// A value indicating whether the warning has been logged in this run.
        /// </summary>
        private static bool _hasWarned;

        /// <summary>
        /// Filter the set of test classes by removing unused classes.
        /// </summary>
        /// <param name="classes">The input list of test class metadata objects.</param>
        /// <param name="classesToRun">The classes to run.</param>
        public static void FilterTestsToRun(IList<ITestClass> classes, ICollection<string> classesToRun)
        {
            if (classesToRun != null && classesToRun.Count > 0)
            {
                List<ITestClass> filtered = new List<ITestClass>(classesToRun.Count);
                foreach (ITestClass test in classes)
                {
                    if (classesToRun.Contains(test.Name))
                    {
                        filtered.Add(test);
                    }
                }
                classes.Replace(filtered);
            }
        }

        /// <summary>
        /// Look through the classes for the [Exclusive] attribute. If found, 
        /// remove any classes where the attribute is not present.
        /// </summary>
        /// <param name="classes">The input list of classes.</param>
        /// <param name="logWriter">The log writer object.</param>
        public static void FilterExclusiveClasses(IList<ITestClass> classes, LogMessageWriter logWriter)
        {
            if (!TestAssemblyHelper.HasExclusiveClasses(classes))
            {
                return;
            }
            if (logWriter != null && ! _hasWarned)
            {
                logWriter.Warning(Properties.UnitTestMessage.TestClassHelper_ExclusiveClassesInUse);
                _hasWarned = true;
            }
            List<ITestClass> filtered = new List<ITestClass>();
            foreach (ITestClass cl in classes)
            {
                if (ReflectionUtility.HasAttribute(cl.Type, typeof(ExclusiveAttribute)))
                {
                    filtered.Add(cl);
                }
            }
            classes.Replace(filtered);
        }

        /// <summary>
        /// Look for the /p:FilterClass parameter in the test harness settings. 
        /// Try to do a substring match on all filtered test classes.
        /// </summary>
        /// <param name="parameters">The parameters.</param>
        /// <param name="classes">List of test classes to be filtered.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1307:SpecifyStringComparison", MessageId = "System.String.IndexOf(System.String)", Justification = "This API is not available in Silverlight's BCL."), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "This API is not available in Silverlight's BCL."), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1045:DoNotPassTypesByReference", MessageId = "1#", Justification = "Although exposed as a public type, this is intended for internal framework consumption only.")]
        public static void FilterByFilterClassParameter(IDictionary<string, string> parameters, IList<ITestClass> classes)
        {
            if (parameters != null)
            {
                if (parameters.ContainsKey("p:filterclass"))
                {
                    string classNameSubstring = parameters["p:filterclass"];
                    if (!String.IsNullOrEmpty(classNameSubstring))
                    {
                        classNameSubstring = classNameSubstring.ToLower(CultureInfo.InvariantCulture);
                        List<ITestClass> filtered = new List<ITestClass>();
                        foreach (ITestClass testClass in classes)
                        {
                            if (testClass.Name.ToLower(CultureInfo.InvariantCulture).IndexOf(classNameSubstring) >= 0)
                            {
                                filtered.Add(testClass);
                            }
                        }
                        classes.Replace(filtered);
                    }
                }
            }
        }
    }
}