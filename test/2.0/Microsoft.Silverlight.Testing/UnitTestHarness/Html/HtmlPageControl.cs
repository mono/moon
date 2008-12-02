// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A control for interacting with the header on an HTML page.
    /// </summary>
    public class HtmlPageControl : HtmlContainerControl
    {
        /// <summary>
        /// The header control.
        /// </summary>
        private HtmlHeaderControl _header;

        /// <summary>
        /// The body control.
        /// </summary>
        private HtmlContainerControl _body;

        /// <summary>
        /// Initializes a new HtmlPageControl object.
        /// </summary>
        /// <param name="existingElement">A reference to the existing page 
        /// element on the page.</param>
        [SuppressMessage("Microsoft.Performance", "CA1804:RemoveUnusedLocals", MessageId = "body", Justification = "This helps initialize the property at construction.")]
        [SuppressMessage("Microsoft.Performance", "CA1804:RemoveUnusedLocals", MessageId = "head", Justification = "This helps initialize the property at construction.")]
        public HtmlPageControl(HtmlElement existingElement)
            : base(existingElement) 
        {
            // Attempt to load these immediately 
            try
            {
                HtmlControlBase body = Body;
                HtmlControlBase head = Header;
            }
            catch (InvalidOperationException) 
            { 
            }
        }

        /// <summary>
        /// Gets the body control.
        /// </summary>
        public HtmlContainerControl Body
        {
            get
            {
                if (_body == null)
                {
                    _body = new HtmlContainerControl(HtmlPage.Document.Body);
                }
                return _body;
            }
        }

        /// <summary>
        /// Gets the header control for the page.
        /// </summary>
        public HtmlHeaderControl Header
        {
            get
            {
                if (_header == null)
                {
                    ScriptObjectCollection col = HtmlPage.Document.GetElementsByTagName("head");
                    if (col.Count == 0)
                    {
                        throw new InvalidOperationException();
                    }
                    HtmlElement head = col[0] as HtmlElement;
                    if (head != null)
                    {
                        _header = new HtmlHeaderControl(head);
                    }
                }
                return _header;
            }
        }

        /// <summary>
        /// Gets or sets the page title.
        /// </summary>
        public string Title
        {
            get { return GetProperty(HtmlProperty.Title) as string; }
            set { SetProperty(HtmlProperty.Title, value); }
        }
    }
}