// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata
{
    /// <summary>
    /// A set of capabilities that a unit test provider may chose to implement 
    /// through the metadata interfaces.
    /// </summary>
    [SuppressMessage("Microsoft.Design", "CA1008:EnumsShouldHaveZeroValue", Justification = "Simplified for now."), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2217:DoNotMarkEnumsWithFlags", Justification = "Simplified for now."), Flags]
    public enum UnitTestProviderCapabilities
    {
        /// <summary>
        /// No defined capabilities.
        /// </summary>
        None = 0,

        /// <summary>
        /// Support for attribute [AssemblyInitialize].
        /// </summary>
        AssemblySupportsInitializeMethod = 2 ^ 1,

        /// <summary>
        /// Support for attribute [AssemblyCleanup].
        /// </summary>
        AssemblySupportsCleanupMethod = 2 ^ 2,

        /// <summary>
        /// Support for attribute [Ignore] for classes.
        /// </summary>
        ClassCanIgnore = 2 ^ 3,

        /// <summary>
        /// Support for attribute [Ignore] for methods.
        /// </summary>
        MethodCanIgnore = 2 ^ 4,

        /// <summary>
        /// Support for attribute [Description] on methods.
        /// </summary>
        MethodCanDescribe = 2 ^ 5,

        /// <summary>
        /// Support for attribute [Category] on methods.
        /// </summary>
        MethodCanCategorize = 2 ^ 6,

        /// <summary>
        /// Support for attribute [Owner] on method.
        /// </summary>
        MethodCanHaveOwner = 2 ^ 7,

        /// <summary>
        /// Support for attribute [Priority] on method.
        /// </summary>
        MethodCanHavePriority = 2 ^ 8,

        /// <summary>
        /// Support for attribute [TestProperty](...) on methods.
        /// </summary>
        MethodCanHaveProperties = 2 ^ 9,

        /// <summary>
        /// Support for attribute [Timeout] on methods.
        /// </summary>
        MethodCanHaveTimeout = 2 ^ 10,

        /// <summary>
        /// Support for attribute [WorkItem(...)]('s) on methods.
        /// </summary>
        MethodCanHaveWorkItems = 2 ^ 11,
    }
}