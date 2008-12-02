// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using System.Globalization;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A &lt;span /&gt;-based container control for content, provides inner 
    /// HTML and text values.
    /// </summary>
    public class HtmlSpan : HtmlContainerControl
    {
        /// <summary>
        /// Initializes a new span (container) element.
        /// </summary>
        public HtmlSpan() : base(HtmlTag.Span) { }
    }
}