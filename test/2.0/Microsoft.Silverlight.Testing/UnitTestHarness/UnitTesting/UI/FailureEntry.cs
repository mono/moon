// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// An entry that tracks failure elements and IDs to setup link associations
    /// in the interface.
    /// </summary>
    public class FailureEntry
    {
        /// <summary>
        /// Gets or sets the HTML control element.
        /// </summary>
        public HtmlControl Element
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the ID string.
        /// </summary>
        public string Id
        {
            get;
            set;
        }
    }
}