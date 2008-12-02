// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// A simple class that represents a work item for testing, the Invoke 
    /// method is called on the work item until the IsComplete property returns 
    /// true. The Invoke bool method returns false when complete.
    /// </summary>
    public interface IWorkItem
    {
        /// <summary>
        /// Invoke the work item to perform some unit of work.  Will be called 
        /// until it returns false.
        /// </summary>
        /// <returns>False when no more work remains.</returns>
        bool Invoke();

        /// <summary>
        /// Gets a value indicating whether the work item has completed.
        /// </summary>
        bool IsComplete { get; }
    }
}