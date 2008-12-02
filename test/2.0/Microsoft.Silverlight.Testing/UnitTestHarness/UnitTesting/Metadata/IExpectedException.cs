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
    /// An expected exception marker for a test method.
    /// </summary>
    [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "The ExpectedException name is the only clear identifier of this functionality.")]
    public interface IExpectedException
    {
        /// <summary>
        /// Gets the expected exception type.
        /// </summary>
        Type ExceptionType { get; }

        /// <summary>
        /// Gets any message associated with the expected exception object.
        /// </summary>
        string Message { get; }
    }
}