// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A subset of the CSS property values for display.
    /// </summary>
    public enum CssDisplay
    {
        /// <summary>
        /// Do not display the element.
        /// </summary>
        None,

        /// <summary>
        /// The element will be displayed as a block-level element, with a line
        /// break before and after the element.
        /// </summary>
        Block,

        /// <summary>
        /// Default in CSS. The element will be displayed as an inline element, 
        /// with no line break before or after the element.
        /// </summary>
        Inline,

        /// <summary>
        /// Displays using the inline block box model.
        /// </summary>
        InlineBlock,
    }
}