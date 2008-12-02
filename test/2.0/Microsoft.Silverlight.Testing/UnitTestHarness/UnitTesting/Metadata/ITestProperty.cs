// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// A property for a test method.
    /// </summary>
    public interface ITestProperty
    {
        /// <summary>
        /// Gets the test property name.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets the test property value.
        /// </summary>
        string Value { get; }
    }
}