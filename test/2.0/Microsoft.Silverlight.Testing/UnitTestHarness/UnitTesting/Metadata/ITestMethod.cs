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
    /// Test method metadata.
    /// </summary>
    public interface ITestMethod
    {
        /// <summary>
        /// Gets the test method reflection object.
        /// </summary>
        MethodInfo Method { get; }

        /// <summary>
        /// Used for decorating with unit test provider-specific capabilities, 
        /// such as the TestContext concept.
        /// </summary>
        /// <param name="instance">Instance to decorate.</param>
        void DecorateInstance(object instance);

        /// <summary>
        /// Hooks up to any unit test provider-enabled WriteLine capability 
        /// for unit tests.
        /// </summary>
        event EventHandler<StringEventArgs> WriteLine;

        /// <summary>
        /// Gets a value indicating whether the test is marked to be ignored.
        /// </summary>
        bool Ignore { get; }

        /// <summary>
        /// Gets any description for the method.
        /// </summary>
        string Description { get; }

        /// <summary>
        /// Gets a name for the method.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets any category information for the method.
        /// </summary>
        string Category { get; }

        /// <summary>
        /// Gets any test owner information.
        /// </summary>
        string Owner { get; }

        /// <summary>
        /// Gets any expected exception attribute .
        /// </summary>
        IExpectedException ExpectedException { get; }

        /// <summary>
        /// Gets any timeout information.
        /// </summary>
        int? Timeout { get; }

        /// <summary>
        /// Gets a collection of any test properties.
        /// </summary>
        ICollection<ITestProperty> Properties { get; }

        /// <summary>
        /// Gets a collection of any test work items.
        /// </summary>
        ICollection<IWorkItemMetadata> WorkItems { get; }

        /// <summary>
        /// Gets any priority information.
        /// </summary>
        IPriority Priority { get; }

        /// <summary>
        /// Get any attribute on the test method that are provided dynamically.
        /// </summary>
        /// <returns>
        /// Dynamically provided attributes on the test method.
        /// </returns>
        [SuppressMessage("Microsoft.Design", "CA1024:UsePropertiesWhereAppropriate", Justification = "This method is intended to be overridden.")]
        IEnumerable<Attribute> GetDynamicAttributes();

        /// <summary>
        /// Invoke the test method.
        /// </summary>
        /// <param name="instance">Instance of the test class.</param>
        void Invoke(object instance);
    }
}