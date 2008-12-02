// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A container which is able to attach to the underlying test dispatcher 
    /// stack to enable advanced asynchronous functionality, when supported.
    /// </summary>
    public class UnitTestMethodContainer : MethodContainer
    {
        /// <summary>
        /// The unit test harness.
        /// </summary>
        private UnitTestHarness _harness;

        /// <summary>
        /// The granularity of the contained item.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields", Justification = "Provides more options for derived classes.")]
        private TestGranularity _granularity;

        /// <summary>
        /// The test method metadata object.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields", Justification = "Provides more options for derived classes.")]
        private ITestMethod _testMethod;

        /// <summary>
        /// Initializes dispatcher-stack attaching method container work item.
        /// </summary>
        /// <param name="testHarness">Test harness.</param>
        /// <param name="instance">Test instance.</param>
        /// <param name="method">Method reflection object.</param>
        /// <param name="testMethod">Test method metadata.</param>
        /// <param name="granularity">Granularity of test.</param>
        public UnitTestMethodContainer(ITestHarness testHarness, object instance, MethodInfo method, ITestMethod testMethod, TestGranularity granularity)
            : base(instance, method, testMethod)
        {
            _granularity = granularity;
            _harness = testHarness as UnitTestHarness;
            _testMethod = testMethod;
        }

        /// <summary>
        /// Connect to dispatcher stack for advanced functions, if supported.
        /// </summary>
        protected override void FirstInvoke()
        {
            if (_harness != null && SupportsWorkItemQueue())
            {
                // Disable automatic completion of this dispatcher queue
                FinishWhenEmpty = false;

                // Connect
                _harness.DispatcherStack.Push(this);

                // Disconnect
                Complete += delegate(object sender, EventArgs e)
                {
                    _harness.DispatcherStack.Pop(); 
                };
            }
            base.FirstInvoke();
        }

        /// <summary>
        /// Check a MethodInfo for the advanced async attribute.
        /// </summary>
        /// <returns>True if the work item queue is supported.</returns>
        private bool SupportsWorkItemQueue()
        {
            if (_testMethod != null)
            {
                return ReflectionUtility.HasAttribute(_testMethod, typeof(AsynchronousAttribute));
            }
            else if (MethodInfo != null)
            {
                return ReflectionUtility.HasAttribute(MethodInfo, typeof(AsynchronousAttribute));
            }
            else
            {
                return false;
            }
        }
    }
}