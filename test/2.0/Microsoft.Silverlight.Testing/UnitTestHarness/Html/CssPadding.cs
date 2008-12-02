// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// The padding type for an element.
    /// </summary>
    public class CssPadding : CssBox
    {
        /// <summary>
        /// Initializes a new padding type.
        /// </summary>
        /// <param name="element">The owning element.</param>
        public CssPadding(HtmlControlBase element) : base(element, CssAttribute.PaddingTop, CssAttribute.PaddingRight, CssAttribute.PaddingBottom, CssAttribute.PaddingLeft) { }
    }
}