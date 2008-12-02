// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Reflection;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// Work item for a test.
    /// </summary>
    public interface IWorkItemMetadata
    {
        /// <summary>
        /// Gets the associated information from the work item.
        /// </summary>
        string Data { get; }
    }
}