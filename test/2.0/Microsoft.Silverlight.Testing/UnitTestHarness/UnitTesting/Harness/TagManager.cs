// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A helper class that manages tags and associated metadata. Tag
    /// expressions are evaluated at the TestClass level.
    /// </summary>
    public partial class TagManager
    {
        // This class is not meant to have a long lifetime as there are 
        // situations in a test run where ITestMethod and ITestClass 
        // references in fact are renewed.
        // 
        // Three primary sets of relationships are stored:
        // _classTags: List of tags for the Type associated with this instance
        // _methodTags: Relates ITestMethod instances to a list of tags
        // _tagsToMethods: Relates a tag to a set of method metadata

        /// <summary>
        /// A reference to the tag attribute type.
        /// </summary>
        private static readonly Type TagType = typeof(TagAttribute);

        /// <summary>
        /// The test tags associated with the class.
        /// </summary>
        private Tags _classTags;

        /// <summary>
        /// The test tags associated with methods.
        /// </summary>
        private Dictionary<ITestMethod, Tags> _methodTags;

        /// <summary>
        /// The ability to grab the set of methods, given a test class type, 
        /// and the tag of interest.
        /// </summary>
        private Dictionary<string, List<ITestMethod>> _tagsToMethods;

        /// <summary>
        /// The test class type.
        /// </summary>
        private Type _testClass;

        /// <summary>
        /// Gets or sets the universe of all test methods for expression 
        /// evaluation.
        /// </summary>
        private List<ITestMethod> Universe { get; set; }

        /// <summary>
        /// Initializes a new tag manager.
        /// </summary>
        /// <param name="test">The test class type.</param>
        /// <param name="methods">The set of methods to run.</param>
        public TagManager(Type test, IEnumerable<ITestMethod> methods)
        {
            _testClass = test;
            _classTags = new Tags();
            _methodTags = new Dictionary<ITestMethod, Tags>();
            _tagsToMethods = new Dictionary<string, List<ITestMethod>>();
            Universe = new List<ITestMethod>(methods);

            CreateClassTags(_testClass);
            foreach (ITestMethod method in methods)
            {
                CreateMethodTags(method);
            }
        }

        /// <summary>
        /// Reflect, read and prepare the tags for the class metadata. Performs 
        /// the work if this is the first time the metadata has been seen.
        /// </summary>
        /// <param name="test">The reflection object for the test class.</param>
        private void CreateClassTags(Type test)
        {
            // 1. Full class name
            _classTags.Add(test.FullName);

            // 2. Class name
            _classTags.Add(test.Name);
            
            // 3. All [Tag] attributes on the type
            foreach (Attribute attribute in ReflectionUtility.GetAttributes(test, TagType))
            {
                _classTags.Add(attribute);
            }
        }

        /// <summary>
        /// Reflect, read and prepare the tags for the method metadata. Performs 
        /// the work if this is the first time the metadata has been seen.
        /// </summary>
        /// <param name="method">The method metadata.</param>
        private void CreateMethodTags(ITestMethod method)
        {
            MethodInfo m = method.Method;

            // 1. All the tags from the class
            Tags tags = new Tags(_classTags);

            // 2. Method.Name
            tags.Add(m.Name);

            // 3. Type.FullName + Method.Name
            tags.Add(m.ReflectedType.FullName + "." + m.Name);

            // 4. Type.Name + Method.Name
            tags.Add(m.ReflectedType.Name + "." + m.Name);

            // 5. Implicit Inherited tag
            if (m.ReflectedType != m.DeclaringType)
            {
                tags.Add("Inherited");
            }

            // 6. All [Tag] attributes on the method
            foreach (Attribute attribute in ReflectionUtility.GetAttributes(method, TagType, false))
            {
                tags.Add(attribute);
            }
            _methodTags.Add(method, tags);

            foreach (string tag in tags)
            {
                List<ITestMethod> methods;
                if (!_tagsToMethods.TryGetValue(tag, out methods))
                {
                    methods = new List<ITestMethod>();
                    _tagsToMethods[tag] = methods;
                }
                methods.Add(method);
            }
        }

        /// <summary>
        /// Get the test methods that correspond to a tag expression.
        /// </summary>
        /// <param name="tagExpression">Tag expression.</param>
        /// <returns>Test methods for the tag expression.</returns>
        public IEnumerable<ITestMethod> EvaluateExpression(string tagExpression)
        {
            if (tagExpression == null)
            {
                throw new ArgumentNullException("tagExpression");
            }
            else if (tagExpression.Length == 0)
            {
                throw new ArgumentException(Properties.UnitTestMessage.TagManager_ExpressionEvaluator_EmptyTagExpression, "tagExpression");
            }
            return ExpressionEvaluator.Evaluate(this, tagExpression);
        }
    }
}