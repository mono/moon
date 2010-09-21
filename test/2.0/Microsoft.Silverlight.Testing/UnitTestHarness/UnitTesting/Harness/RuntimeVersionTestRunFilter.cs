// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A method and class filter that uses expressions and the TagAttribute.
    /// </summary>
    public class RuntimeVersionTestRunFilter : TestRunFilter
    {
	public RuntimeVersionTestRunFilter(UnitTestSettings settings, UnitTestHarness harness, TestRunFilter subfilter) : base(settings, harness)
        {
		this.subfilter = subfilter;
		this.runtime_version = settings.RuntimeVersion;
        }

	TestRunFilter subfilter;
	int runtime_version;

        protected internal override void FilterCustomTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
		subfilter.FilterCustomTestClasses (classes, instances);

		FilterByRuntimeVersion (classes, instances);
	}

        protected internal override void FilterCustomTestMethods(IList<ITestMethod> methods)
        {
		if (methods == null || methods.Count == 0)
			return;
            
		subfilter.FilterCustomTestMethods (methods);

		FilterByRuntimeVersion (methods);
        }

        protected internal override void FilterExclusiveTestClasses(IList<ITestClass> classes, TestClassInstanceDictionary instances)
        {
		subfilter.FilterExclusiveTestClasses (classes, instances);

		FilterByRuntimeVersion (classes, instances);
        }

        protected internal override void FilterExclusiveTestMethods(IList<ITestMethod> methods)
        {
		subfilter.FilterExclusiveTestMethods (methods);

		FilterByRuntimeVersion (methods);
        }

	private void FilterByRuntimeVersion (IList<ITestClass> classes, TestClassInstanceDictionary instances)
	{
		List<ITestClass> emptyTests = new List<ITestClass>();
		foreach (ITestClass test in classes)
		{
			object instance = instances.GetInstance(test.Type);
			ICollection<ITestMethod> methods = GetTestMethods(test, instance);
			FilterByRuntimeVersion (methods);
			if (methods.Count == 0) {
				emptyTests.Add(test);
			}
		}
		if (emptyTests.Count > 0) {
			foreach (ITestClass test in emptyTests) {
				classes.Remove(test);
			}
		}
	}

        private void FilterByRuntimeVersion (ICollection<ITestMethod> methods)
        {
            List<ITestMethod> original = new List<ITestMethod>(methods);
            methods.Clear();

            foreach (ITestMethod method in original) {
		    // test the runtime version attributes of the method
		    if (RuntimeVersionMatches (method)) 
			    methods.Add(method);
            }
        }

	private bool RuntimeVersionMatches (ITestMethod method)
	{
		// runtime attribute precedence (highest to lowest)
		//
		// 1. RuntimeVersion
		// 2. NotRuntimeVersion
		// 3. MinRuntimeVersion + MaxRuntimeVersion
		//
		// if one set of attributes is present we don't
		// continue checking down the list for others.
		//
		// also, we don't generate warnings or errors if the
		// attributes conflict.
		//

		ICollection<Attribute> attrs;

		attrs = ReflectionUtility.GetAttributes (method.Method, ProviderAttributes.RuntimeVersion, true);
		if (attrs != null && attrs.Count > 0) {
			foreach (var a in attrs) {
				if (((RuntimeVersionAttribute)a).RuntimeVersion == runtime_version)
					return true;
			}
			return false;
		}

		attrs = ReflectionUtility.GetAttributes (method.Method, ProviderAttributes.NotRuntimeVersion, true);
		if (attrs != null && attrs.Count > 0) {
			foreach (var a in attrs) {
				if (((NotRuntimeVersionAttribute)a).RuntimeVersion == runtime_version)
					return false;
			}
			return true;
		}

		bool above_min = true;
		bool below_max = true;

		attrs = ReflectionUtility.GetAttributes (method.Method, ProviderAttributes.MinRuntimeVersion, true);
		if (attrs != null && attrs.Count > 0) {
			foreach (var a in attrs) {
				if (((MinRuntimeVersionAttribute)a).RuntimeVersion > runtime_version)
					return false;
			}
		}

		attrs = ReflectionUtility.GetAttributes (method.Method, ProviderAttributes.MaxRuntimeVersion, true);
		if (attrs != null && attrs.Count > 0) {
			foreach (var a in attrs) {
				if (((MaxRuntimeVersionAttribute)a).RuntimeVersion < runtime_version)
					return false;
			}
		}

		return above_min && below_max;
	}
    }
}