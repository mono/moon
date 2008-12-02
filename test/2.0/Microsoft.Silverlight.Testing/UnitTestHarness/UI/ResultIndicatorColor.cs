// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// A type that represents a web page used for test purposes of the current 
    /// Silverlight plugin.
    /// </summary>
    public static class ResultIndicatorColor
    {
        /// <summary>
        /// The color used to indicate a failed run.
        /// </summary>
        public const string FinishedFailure = "#f00";

        /// <summary>
        /// The color used to indicate that a failure has happened during an 
        /// in-progress run.
        /// </summary>
        public const string InProgressFailure = "#f99";

        /// <summary>
        /// The color used to indicate a good run.
        /// </summary>
        public const string FinishedSuccess = "#0f0";

        /// <summary>
        /// The color used to indicate that there have been no failures yet, but
        /// the run is in-progress.
        /// </summary>
        public const string InProgressSuccess = "#9f9";

        /// <summary>
        /// The background color used to indicate a failing run.
        /// </summary>
        public const string BackgroundFailure = "#fee";

        /// <summary>
        /// The background color used to indicate a passing run.
        /// </summary>
        public const string BackgroundSuccess = "#efe";
    }
}