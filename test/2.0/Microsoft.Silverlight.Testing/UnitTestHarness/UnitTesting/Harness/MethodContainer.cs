// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using Microsoft.Silverlight.Testing.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A method container.
    /// </summary>
    public class MethodContainer : CompositeWorkItem
    {
        /// <summary>
        /// Constructs a new method container.
        /// </summary>
        private MethodContainer() { }

        /// <summary>
        /// Constructs a new method container.
        /// </summary>
        /// <param name="instance">An instance of the method's type.</param>
        /// <param name="method">The method reflection object.</param>
        /// <param name="testMethod">The test method.</param>
        public MethodContainer(object instance, MethodInfo method, ITestMethod testMethod)
            : base()
        {
            _methodTask = new MethodInvokeWorkItem(instance, method, testMethod);
            _methodInfo = method;
        }

        /// <summary>
        /// The task that involves the method, and contains its own internal 
        /// test queue, if needed for asynchronous tasks.
        /// </summary>
        private MethodInvokeWorkItem _methodTask;

        /// <summary>
        /// The reflection object for the method.
        /// </summary>
        private MethodInfo _methodInfo;

        /// <summary>
        /// Invoke into the method.
        /// </summary>
        /// <returns>Returns the condition of any remaining work.</returns>
        public override bool Invoke()
        {
            // FEATURE: This is where [Timeout(...)] support would go if we'd like to support it
            return base.Invoke();
        }

        /// <summary>
        /// On the first invoke, make sure there's a task to call the method.
        /// </summary>
        protected override void FirstInvoke()
        {
            Enqueue(_methodTask);
        }

        /// <summary>
        /// Gets the method's reflection object.
        /// </summary>
        protected MethodInfo MethodInfo
        {
            get { return _methodInfo; }
        }
    }
}