// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using Microsoft.Silverlight.Testing;

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    /// <summary>
    /// A work item for tests to use which will call the delegate when the work 
    /// item is executed.
    /// </summary>
    public class CallbackWorkItem : WorkItem
    {
        /// <summary>
        /// Stored void delegate.
        /// </summary>
        private Action _action;

        /// <summary>
        /// Create a new Callback work item.
        /// </summary>
        /// <param name="callback">Action to execute on Invoke.</param>
        public CallbackWorkItem(Action callback) : base()
        {
            _action = callback;
        }

        /// <summary>
        /// Execute the callback.
        /// </summary>
        /// <returns>Whether the work item is complete.</returns>
        public override bool Invoke()
        {
            _action();
            WorkItemComplete();
            return base.Invoke();
        }
    }
}