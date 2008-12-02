// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    /// <summary>
    /// A special type dedicated to keeping a running stack of the dispatch 
    /// managers that are actually enabled for "work item" use.  The current 
    /// dispatcher is used by the test work item queue.
    /// </summary>
    public class WorkItemsManager
    {
        /// <summary>
        /// Dispatcher stack; a stack of work item containers.
        /// </summary>
        private Stack<ICompositeWorkItem> _stack;

        /// <summary>
        /// Creates a new empty stack for work item containers.
        /// </summary>
        public WorkItemsManager()
        {
            _stack = new Stack<ICompositeWorkItem>();
        }

        /// <summary>
        /// Gets the current test work item dispatcher, which is the dispatcher 
        /// on the top of the stack.  Returns null if there is none.
        /// </summary>
        public ICompositeWorkItem CurrentCompositeWorkItem
        {
            get { return (_stack.Count == 0) ? null : _stack.Peek(); }
        }

        /// <summary>
        /// Push a new dispatcher onto the stack.
        /// </summary>
        /// <param name="composite">The composite work item to push.</param>
        public void Push(ICompositeWorkItem composite)
        {
            _stack.Push(composite);
        }

        /// <summary>
        /// Pop a dispatcher off the stack.
        /// </summary>
        /// <returns>Returns the top-most container.  Throws an 
        /// InvalidOperationException if none is available.</returns>
        public ICompositeWorkItem Pop()
        {
            if (CurrentCompositeWorkItem == null)
            {
                throw new InvalidOperationException(Properties.UnitTestMessage.UnitTestHarness_RunNextStep_NoCompositeWorkItemsExist);
            }
            ICompositeWorkItem queue = _stack.Peek();
            if (queue != null)
            {
                _stack.Pop();
            }
            return queue;
        }
    }
}