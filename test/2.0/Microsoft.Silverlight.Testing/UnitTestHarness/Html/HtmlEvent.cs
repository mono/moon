// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// JavaScript events that can be attached to, exposed on an HtmlElement.
    /// </summary>
    public enum HtmlEvent
    {
        /// <summary>
        /// The mouse Click event.
        /// </summary>
        Click,

        /// <summary>
        /// The change event for input controls.
        /// </summary>
        Change,

        /// <summary>
        /// The mouse over event.
        /// </summary>
        Mouseover,

        /// <summary>
        /// The mouse out event.
        /// </summary>
        Mouseout,
    }
}