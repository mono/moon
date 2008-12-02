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
    /// A HTML anchor control.
    /// </summary>
    public class HtmlAnchor : HtmlContainerControl
    {
        /// <summary>
        /// JavaScript code that will cause a link to not navigate anywhere, but 
        /// the OnClick event will still fire.
        /// </summary>
        private const string DoNothingJavaScript = "javascript:void(0)";

        /// <summary>
        /// Initializes a new managed anchor control inside the parent control.
        /// </summary>
        public HtmlAnchor() : base(HtmlTag.A) 
        {
            AttachEvent(HtmlEvent.Click, new EventHandler<HtmlEventArgs> (OnClick));
        }

        /// <summary>
        /// Initializes a new managed anchor control.
        /// </summary>
        /// <param name="innerText">The inner text of the anchor/link.</param>
        /// <param name="clickHandler">The click handler for the button.</param>
        public HtmlAnchor(string innerText, EventHandler<HtmlEventArgs> clickHandler) : this()
        {
            InnerText = innerText;
            Href = null;
            Click += clickHandler;
        }

        /// <summary>
        /// Initializes a new HtmlAnchor object in the parent control.
        /// </summary>
        /// <param name="nameOfLink">The name of the link to set.</param>
        public HtmlAnchor(string nameOfLink) : this()
        {
            InitializeNamedLink(nameOfLink);
        }

        /// <summary>
        /// Gets or sets the hypertext reference for the anchor.
        /// 
        /// Set to null to include JavaScript that does nothing, to prevent a 
        /// click-through 
        /// or page error.
        /// </summary>
        public string Href
        {
            get
            {
                return GetAttributeOrNull(HtmlAttribute.Href);
            }

            set
            {
                if (value == null)
                {
                    value = DoNothingJavaScript;
                }
                SetAttribute(HtmlAttribute.Href, value);
            }
        }

        /// <summary>
        /// Removes the HREF attribute.
        /// </summary>
        public void ClearHref()
        {
            RemoveAttribute(HtmlAttribute.Href);
        }

        /// <summary>
        /// Initializes a named link anchor - no href, just a name.
        /// </summary>
        /// <param name="thisLinkName">The name attribute's value.</param>
        public void InitializeNamedLink(string thisLinkName)
        {
            ClearHref();
            Name = thisLinkName;
        }

        /// <summary>
        /// Gets or sets the name of the anchor.
        /// </summary>
        public string Name
        {
            get
            {
                return GetAttributeOrNull(HtmlAttribute.Name);
            }

            set
            {
                SetAttribute(HtmlAttribute.Name, value);
            }
        }

        /// <summary>
        /// Gets or sets the target.
        /// </summary>
        public string Target
        {
            get
            {
                return GetAttributeOrNull(HtmlAttribute.Target);
            }

            set
            {
                SetAttribute(HtmlAttribute.Target, value);
            }
        }

        /// <summary>
        /// Gets or sets the title.
        /// </summary>
        public string Title
        {
            get
            {
                return GetAttributeOrNull(HtmlAttribute.Title);
            }

            set
            {
                SetAttribute(HtmlAttribute.Title, value);
            }
        }

        /// <summary>
        /// The target of the Click will be the managed HtmlAnchor object 
        /// instance and NOT the actual HtmlElement.
        /// </summary>
        public event EventHandler<HtmlEventArgs> Click;

        /// <summary>
        /// This calls the Click event handler.
        /// </summary>
        /// <remarks>
        /// The target of the Click will be the managed HtmlAnchor object 
        /// instance and NOT the actual HtmlElement.
        /// </remarks>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event arguments.</param>
        private void OnClick(object sender, HtmlEventArgs e)
        {
            if (Click != null)
            {
                Click(this, e);
            }
        }
    }
}