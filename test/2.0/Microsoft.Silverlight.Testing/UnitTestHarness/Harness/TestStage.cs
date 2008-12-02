// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// The stage of a test scenario, case or system running.
    /// </summary>
    public enum TestStage
    {
        /// <summary>
        /// In the process of starting.
        /// </summary>
        Starting,

        /// <summary>
        /// Currently running.
        /// </summary>
        Running,

        /// <summary>
        /// Finishing up.
        /// </summary>
        Finishing,

        /// <summary>
        /// The test is canceling.
        /// </summary>
        Canceling,
    }
}