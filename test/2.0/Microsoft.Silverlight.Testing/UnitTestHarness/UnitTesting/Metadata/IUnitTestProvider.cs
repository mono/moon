// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Reflection;
using ITestHarness = Microsoft.Silverlight.Testing.Harness.ITestHarness;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// Provider model for a unit test system that uses reflection and metadata 
    /// to enable a unit test run.
    /// </summary>
    public interface IUnitTestProvider
    {
        /// <summary>
        /// Retrieve the metadata instance for a test assembly given a 
        /// reflection Assembly instance.
        /// </summary>
        /// <param name="testHarness">The test harness using the provider.</param>
        /// <param name="assemblyReference">Reflected test assembly.</param>
        /// <returns>Unit test provider-specific metadata instance for 
        /// the test assembly.</returns>
        IAssembly GetUnitTestAssembly(ITestHarness testHarness, Assembly assemblyReference);

        /// <summary>
        /// Gets the name of the unit test provider.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets the capabilities that the unit test provider implements.
        /// </summary>
        UnitTestProviderCapabilities Capabilities { get; }

        /// <summary>
        /// Gets a value indicating whether a specific capability or set of 
        /// capabilities are supported by the unit test provider.
        /// </summary>
        /// <param name="capability">Capability of interest.</param>
        /// <returns>Gets a value indicating whether the capability is 
        /// supported.</returns>
        bool HasCapability(UnitTestProviderCapabilities capability);

        /// <summary>
        /// Checks if an Exception actually represents an assertion that failed 
        /// to improve the logging experience.
        /// </summary>
        /// <param name="exception">Exception object.</param>
        /// <returns>Returns true if the Exception is an assertion exception 
        /// type.</returns>
        bool IsFailedAssert(Exception exception);
    }
}