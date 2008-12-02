// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// An attached box type that represents the four border dimensions for an 
    /// HTML element's border.
    /// </summary>
    public class CssBorder : CssBox
    {
        /// <summary>
        /// Initializes a new attached CSS border for a managed HtmlElement.
        /// </summary>
        /// <param name="element">The owner of the border.</param>
        public CssBorder(HtmlControlBase element) : base(element, CssAttribute.BorderTop, CssAttribute.BorderRight, CssAttribute.BorderBottom, CssAttribute.BorderLeft) { }
    }
}