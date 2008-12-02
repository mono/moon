// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    /// <summary>
    /// A simple work item that invokes a method through the reflection 
    /// MethodInfo instance.
    /// </summary>
    public class MethodInvokeWorkItem : WorkItem
    {
        /// <summary>
        /// An empty object array.
        /// </summary>
        private static readonly object[] None = { };

        /// <summary>
        /// An object instance.
        /// </summary>
        private object _instance;

        /// <summary>
        /// Method reflection object.
        /// </summary>
        private MethodInfo _method;

        /// <summary>
        /// The test method to invoke.
        /// </summary>
        private ITestMethod _testMethod;

        /// <summary>
        /// Creates a new method invoke work item for a MethodInfo instance.
        /// </summary>
        /// <param name="instance">The type instance.</param>
        /// <param name="method">The method on the type to invoke when the 
        /// work item is executed.</param>
        /// <param name="testMethod">The test method metadata.</param>
        public MethodInvokeWorkItem(object instance, MethodInfo method, ITestMethod testMethod)
            : base()
        {
            _instance = instance;
            _method = method;
            _testMethod = testMethod;
        }

        /// <summary>
        /// Invokes the underlying method on the instance and marks the 
        /// test work item as complete.
        /// </summary>
        /// <returns>False, noting the completion of the work item.</returns>
        public override bool Invoke()
        {
            if (_testMethod != null)
            {
                _testMethod.Invoke(_instance);
            }
            else if (_method != null)
            {
                _method.Invoke(_instance, None);
            }
            
            WorkItemComplete();
            return base.Invoke();
        }
    }
}