// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// A test work item is a task that is invoked until it is complete.  It 
    /// maintains its own state to be able to notify the caller when it is 
    /// finally complete, with no further work to be run.
    /// 
    /// It is possible that some implementations of a TestWorkItem may actually 
    /// contain a set of sub-tasks by implementing a composite pattern.
    /// </summary>
    public class WorkItem : IWorkItem
    {
        /// <summary>
        /// Invoke the task.  Return false only when the task is complete.
        /// </summary>
        /// <returns>True if there is additional work to be completed.  False 
        /// when there is none.</returns>
        public virtual bool Invoke()
        {
            return ! IsComplete;
        }

        /// <summary>
        /// Gets a value indicating whether the task's work is complete.
        /// </summary>
        public bool IsComplete
        {
            get;
            protected set;
        }

        /// <summary>
        /// Called by the task after the work is complete.
        /// </summary>
        protected virtual void WorkItemComplete()
        {
            IsComplete = true;
        }
    }
}