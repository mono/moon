// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A control for interacting with the header on an HTML page.
    /// </summary>
    public class HtmlHeaderControl : HtmlContainerControl
    {
        /// <summary>
        /// Initializes a new HtmlHeaderControl object.
        /// </summary>
        public HtmlHeaderControl() : base(HtmlTag.Head) { }

        /// <summary>
        /// Initializes a new HtmlHeaderControl object.
        /// </summary>
        /// <param name="existingElement">A reference to the existing header 
        /// element on the page.</param>
        public HtmlHeaderControl(HtmlElement existingElement) : base(existingElement) { }
    }
}