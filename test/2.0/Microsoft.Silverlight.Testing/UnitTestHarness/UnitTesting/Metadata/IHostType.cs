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
    /// Represents the host type information for a test.
    /// </summary>
    public interface IHostType
    {
        /// <summary>
        /// Gets the Host type.
        /// </summary>
        string HostType { get; }

        /// <summary>
        /// Gets the host data.
        /// </summary>
        string HostData { get; }
    }
}