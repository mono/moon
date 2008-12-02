// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// A representation of a test's priority.
    /// </summary>
    [SuppressMessage("Microsoft.Design", "CA1040:AvoidEmptyInterfaces", Justification = "These interfaces are used for wrapping attributes of different types, so this is needed for the facade pattern")]
    public interface IPriority { }
}