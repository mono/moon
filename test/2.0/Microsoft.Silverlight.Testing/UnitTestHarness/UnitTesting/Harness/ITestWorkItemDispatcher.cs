//------------------------------------------------------------------------------
// <copyright file="IWorkItemQueue.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Queue of tasks that need to be executed
    /// </summary>
    public interface ITestWorkItemDispatcher : ITestWorkItem
    {

        void Enqueue(ITestWorkItem item);

        ITestWorkItem Dequeue();

        ITestWorkItem Peek();

        bool HasTasks();

        event EventHandler Complete;

    }

}
