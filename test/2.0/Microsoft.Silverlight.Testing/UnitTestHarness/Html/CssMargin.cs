// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// The margin type for an element.
    /// </summary>
    public class CssMargin : CssBox
    {
        /// <summary>
        /// Initializes a new margin type for the control.
        /// </summary>
        /// <param name="element">The owning element.</param>
        public CssMargin(HtmlControlBase element) : base(element, CssAttribute.MarginTop, CssAttribute.MarginRight, CssAttribute.MarginBottom, CssAttribute.MarginLeft) { }
    }
}