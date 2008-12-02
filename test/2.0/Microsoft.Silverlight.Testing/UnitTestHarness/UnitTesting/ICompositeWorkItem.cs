// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    using System;

    /// <summary>
    /// Queue of tasks that need to be executed.
    /// </summary>
    public interface ICompositeWorkItem : IWorkItem
    {
        /// <summary>
        /// Enqueue a work item.
        /// </summary>
        /// <param name="item">Test work item.</param>
        void Enqueue(IWorkItem item);

        /// <summary>
        /// Dequeue a test work item.
        /// </summary>
        /// <returns>Any available work item, or null if none are available in 
        /// the queue.</returns>
        IWorkItem Dequeue();

        /// <summary>
        /// Peek at the end of the test work item container.
        /// </summary>
        /// <returns>Any available test work item.</returns>
        IWorkItem Peek();

        /// <summary>
        /// Gets a value indicating whether any tasks are available in the queue.
        /// </summary>
        bool RemainingWork { get; }

        /// <summary>
        /// Completion event for the work item container.
        /// </summary>
        event EventHandler Complete;

        /// <summary>
        /// Event fired when an unhandled exception occurs inside the 
        /// test work item queue.
        /// </summary>
        event EventHandler<UnhandledExceptionEventArgs> UnhandledException;
    }
}