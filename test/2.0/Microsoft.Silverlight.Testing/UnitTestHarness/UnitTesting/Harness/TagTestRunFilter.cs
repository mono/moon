// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A method and class filter that uses expressions and the TagAttribute.
    /// </summary>
    public class TagTestRunFilter : TestRunFilter
    {
        /// <summary>
        /// The name to use for the test run when the tag expression is null.
        /// </summary>
        private const string NullTagRunName = "Unknown tag expression run";

        /// <summary>
        /// Initializes a new test run filter with the tag expression setting.
        /// </summary>
        /// <param name="settings">Unit test settings.</param>
        /// <param name="harness">Unit test harness.</param>
        public TagTestRunFilter(UnitTestSettings settings, UnitTestHarness harness) : base(settings, harness)
        {
            SetTagExpression(settings.TagExpression);
        }

        /// <summary>
        /// Initializes a new test run filter with the tag expression.
        /// </summary>
        /// <param name="settings">Unit test settings.</param>
        /// <param name="harness">Unit test harness.</param>
        /// <param name="tagExpression">The tag expression to use.</param>
        public TagTestRunFilter(UnitTestSettings settings, UnitTestHarness harness, string tagExpression)
            : base(settings, harness)
        {
            SetTagExpression(tagExpression);
        }

        /// <summary>
        /// Sets the tag expression property.
        /// </summary>
        /// <param name="tagExpression">The tag expression to use.</param>
        private void SetTagExpression(string tagExpression)
        {
            TagExpression = tagExpression;
            TestRunName = string.IsNullOrEmpty(TagExpression) ? NullTagRunName : TagExpression;
        }

        /// <summary>
        /// A value indicating whether the warning has been logged yet.
        /// </summary>
        private static bool _hasLoggedWarning;

        /// <summary>
        /// Gets the tag expression in use by the run filter.
        /// </summary>
        public string TagExpression { get; private set; }

        /// <summary>
        /// Apply tag filtering.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected override void FilterCustomTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
            if (string.IsNullOrEmpty(TagExpression))
            {
                return;
            }

            // Temporarily apply to the methods as well. If the class has no 
            // methods that were matched in the expression, then cut the 
            // class from the run.
            List<ITestClass> emptyTests = new List<ITestClass>();
            foreach (ITestClass test in classes)
            {
                object instance = instances.GetInstance(test.Type);
                ICollection<ITestMethod> methods = GetTestMethods(test, instance);
                TagManager tm = new TagManager(test.Type, methods);
                ApplyExpression(tm, methods);
                if (methods.Count == 0)
                {
                    emptyTests.Add(test);
                }
            }
            if (emptyTests.Count > 0)
            {
                foreach (ITestClass test in emptyTests)
                {
                    classes.Remove(test);
                }
            }

            if (!_hasLoggedWarning)
            {
                UnitTestHarness.LogWriter.Information(string.Format(CultureInfo.InvariantCulture, Properties.UnitTestMessage.TagTestRunFilter_TaggingInUse, TagExpression));
                _hasLoggedWarning = true;
            }
        }

        /// <summary>
        /// Apply tag filtering.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected override void FilterCustomTestMethods(IList<ITestMethod> methods)
        {
            if (methods == null || methods.Count == 0 || string.IsNullOrEmpty(TagExpression))
            {
                return;
            }
            
            TagManager tm = new TagManager(methods[0].Method.ReflectedType, methods);
            ApplyExpression(tm, methods);
        }

        /// <summary>
        /// Apply the tag filtering.
        /// </summary>
        /// <param name="tagManager">The tag manager instance.</param>
        /// <param name="methods">Set of methods.</param>
        private void ApplyExpression(TagManager tagManager, ICollection<ITestMethod> methods)
        {
            List<ITestMethod> original = new List<ITestMethod>(methods);
            methods.Clear();
            tagManager.EvaluateExpression(TagExpression);
            foreach (ITestMethod method in tagManager.EvaluateExpression(TagExpression))
            {
                if (original.Contains(method))
                {
                    methods.Add(method);
                }
            }
        }

        /// <summary>
        /// Exclusive attributes are not supported when also using tagging.
        /// </summary>
        /// <param name="classes">List of test classes.</param>
        /// <param name="instances">Test class instance dictionary.</param>
        protected override void FilterExclusiveTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
        }

        /// <summary>
        /// Exclusive attributes are not supported when also using tagging.
        /// </summary>
        /// <param name="methods">List of test methods.</param>
        protected override void FilterExclusiveTestMethods(IList<ITestMethod> methods)
        {
        }
    }
}