// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A number of possible test outcomes or results.  For compatibility, this 
    /// information resembles that of the desktop test framework, although many 
    /// of the states may not apply or be valid for an in-browser test harness.
    /// </summary>
    public enum TestOutcome
    {
        /// <summary>
        /// A test outcome of Error.
        /// </summary>
        Error,
        
        /// <summary>
        /// A test outcome of Failed.
        /// </summary>
        Failed,
        
        /// <summary>
        /// A test outcome of Timeout.
        /// </summary>
        Timeout,
        
        /// <summary>
        /// A test outcome of Aborted.
        /// </summary>
        Aborted,
        
        /// <summary>
        /// A test outcome of Inconclusive.
        /// </summary>
        Inconclusive,
        
        /// <summary>
        /// A test outcome of a run that was aborted, but passed.
        /// </summary>
        PassedButRunAborted,
        
        /// <summary>
        /// A test outcome of NotRunnable.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Runnable", Justification = "This is not hungarian notation and is needed for compatibility with the full desktop test framework.")]
        NotRunnable,
        
        /// <summary>
        /// A test outcome of NotExecuted.
        /// </summary>
        NotExecuted,
        
        /// <summary>
        /// A test outcome of Disconnected.
        /// </summary>
        Disconnected,
        
        /// <summary>
        /// A test outcome of Warning.
        /// </summary>
        Warning,
        
        /// <summary>
        /// A test outcome of Passed.
        /// </summary>
        Passed,
        
        /// <summary>
        /// A test outcome of Completed.
        /// </summary>
        Completed,
        
        /// <summary>
        /// A test outcome of InProgress.
        /// </summary>
        InProgress,
        
        /// <summary>
        /// A test outcome of Pending.
        /// </summary>
        Pending,
    }
}