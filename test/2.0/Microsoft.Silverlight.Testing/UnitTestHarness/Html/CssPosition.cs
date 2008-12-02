// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// The position type for an element.
    /// </summary>
    public class CssPosition : CssBox
    {
        /// <summary>
        /// Initializes a new position type.
        /// </summary>
        /// <param name="element">The owning element.</param>
        public CssPosition(HtmlControlBase element) : base(element, CssAttribute.Top, CssAttribute.Right, CssAttribute.Bottom, CssAttribute.Left) { }
    }
}