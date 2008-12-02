// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// A type that represents a web page used for test purposes of the current 
    /// Silverlight plugin.
    /// </summary>
    public class HtmlTestColumn : HtmlContainerControl
    {
        /// <summary>
        /// Padding that is always present between the control and plugin.
        /// </summary>
        private const int ImplicitPadding = 10;

        /// <summary>
        /// Initializes the HtmlTestColumn control.
        /// </summary>
        /// <param name="allocatedColumnWidth">The allocated width in pixels of
        /// the test column.</param>
        /// <param name="leftMargin">The left margin size, in pixels.</param>
        /// <param name="clientHeight">The calculated height of the browser's 
        /// client space.</param>
        public HtmlTestColumn(int allocatedColumnWidth, int leftMargin, int clientHeight) : base(HtmlTag.Div) 
        {
            if (allocatedColumnWidth > ImplicitPadding)
            {
                allocatedColumnWidth -= ImplicitPadding;
            }
            Width = allocatedColumnWidth;
            SetStyleAttribute(CssAttribute.MinHeight, clientHeight);

            PreparePositioning();
            PrepareLeftEdge(leftMargin);
        }

        /// <summary>
        /// Positions the column absolutely.
        /// </summary>
        private void PreparePositioning()
        {
            SetStyleAttribute(CssAttribute.Position, "absolute");
            Position.Top = 0;
            Position.Right = 0;
        }

        /// <summary>
        /// Creates a border on the left side of the column.
        /// </summary>
        /// <param name="leftMargin">The margin on the left.</param>
        private void PrepareLeftEdge(int leftMargin)
        {
            Margin.Left = leftMargin;
            BorderWidth = 0;
            SetStyleAttribute(CssAttribute.BorderLeftStyle, BorderStyle.Solid);
            SetStyleAttribute(CssAttribute.BorderLeftColor, Color.AnotherLightGray);
            BorderWidthAdvanced.Left = 1;
        }
    }
}