// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using VS = Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio
{
    /// <summary>
    /// Expected exception metadata.
    /// </summary>
    [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "Standard unit test framework naming")]
    public class ExpectedException : IExpectedException
    {
        /// <summary>
        /// Private constructor.
        /// </summary>
        private ExpectedException() { }

        /// <summary>
        /// Creates a new expected exception metadata wrapper.
        /// </summary>
        /// <param name="expectedExceptionAttribute">Attribute value.</param>
        public ExpectedException(VS.ExpectedExceptionAttribute expectedExceptionAttribute)
        {
            _exp = expectedExceptionAttribute;
            if (_exp == null)
            {
                throw new ArgumentNullException("expectedExceptionAttribute");
            }
        }

        /// <summary>
        /// The expected exception attribute.
        /// </summary>
        private VS.ExpectedExceptionAttribute _exp;

        /// <summary>
        /// Gets the type of the expected exception.
        /// </summary>
        public Type ExceptionType
        {
            get { return _exp.ExceptionType; }
        }

        /// <summary>
        /// Gets any message to include in a failure.
        /// </summary>
        public string Message
        {
            get { return _exp.Message; }
        }
    }
}