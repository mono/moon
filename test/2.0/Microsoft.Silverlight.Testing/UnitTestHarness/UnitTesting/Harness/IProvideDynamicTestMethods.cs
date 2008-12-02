// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// The IProvideDynamicTestMethods interface is used to provide additional
    /// test methods dynamically at runtime.
    /// </summary>
    public interface IProvideDynamicTestMethods
    {
        /// <summary>
        /// Get the dynamic test methods.
        /// </summary>
        /// <returns>Sequence of dynamic test methods.</returns>
        [SuppressMessage("Microsoft.Design", "CA1024:UsePropertiesWhereAppropriate", Justification = "Does more work than a property should.")]
        IEnumerable<ITestMethod> GetDynamicTestMethods();
    }
}