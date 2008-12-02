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
    /// A container control for content, provides inner HTML and text values.
    /// </summary>
    public class HtmlContainerControl : HtmlControl
    {
        /// <summary>
        /// Initializes a new root managed container control.
        /// </summary>
        /// <param name="root">The new root HtmlElement.</param>
        public HtmlContainerControl(HtmlElement root) : base(root) { }

        /// <summary>
        /// Initializes a new HtmlContainerControl.
        /// </summary>
        /// <param name="tag">The tag name to use to create the control element.</param>
        public HtmlContainerControl(string tag) : base(tag) { }

        /// <summary>
        /// Initializes a new HtmlContainerControl.
        /// </summary>
        /// <param name="tag">The tag name to use to create the control element.</param>
        public HtmlContainerControl(HtmlTag tag) : base(tag) { }

        /// <summary>
        /// Gets or sets the inner HTML content.
        /// </summary>
        public string InnerHtml
        {
            get
            {
                return (string)GetProperty(HtmlProperty.InnerHTML);
            }

            set
            {
                SetProperty(HtmlProperty.InnerHTML, value);
            }
        }

        /// <summary>
        /// Gets or sets the inner text content.
        /// </summary>
        public string InnerText
        {
            get
            {
                return (string)GetProperty(HtmlProperty.InnerText);
            }

            set
            {
                SetProperty(HtmlProperty.InnerText, value);
            }
        }
    }
}