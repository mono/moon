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
    /// A &lt;br /&gt; control.
    /// </summary>
    public class HtmlLineBreak : HtmlControl
    {
        /// <summary>
        /// Initializes a new line break element.
        /// </summary>
        public HtmlLineBreak() : base(HtmlTag.Br) { }
    }
}