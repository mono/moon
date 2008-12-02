// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A &lt;div /&gt;-based container control for content, provides inner HTML
    /// and text values.
    /// </summary>
    public class HtmlDiv : HtmlContainerControl
    {
        /// <summary>
        /// Initializes a new DIV.
        /// </summary>
        /// <param name="root">The existing DIV element.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "The ToUpperInvariant is not in Silverlight's BCL.")]
        public HtmlDiv(HtmlElement root) : base(root) 
        {
            if (root.TagName != HtmlTag.Div.ToString().ToLower(CultureInfo.InvariantCulture))
            {
                throw new ArgumentException("The HtmlElement must be a division (div) tag.", "root");
            }
        }

        /// <summary>
        /// Initializes a new div (container) element.
        /// </summary>
        public HtmlDiv() : base(HtmlTag.Div) { }
    }
}