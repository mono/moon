// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A simple managed HTML paragraph control.
    /// </summary>
    public class Paragraph : HtmlContainerControl
    {
        /// <summary>
        /// Initializes a new paragraph control.
        /// </summary>
        public Paragraph() : base(HtmlTag.P) 
        {
        }

        /// <summary>
        /// Initializes a new paragraph control with HTML content included.
        /// </summary>
        /// <param name="html">The initial HTML content.</param>
        public Paragraph(string html) : this()
        {
            InnerHtml = html;
        }
    }
}