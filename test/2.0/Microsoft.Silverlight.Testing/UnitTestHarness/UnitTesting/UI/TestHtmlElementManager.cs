// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.IO;
using System.Net;
using System.Reflection;
using System.Windows;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A type for managing the shared 'scratch html element'.
    /// </summary>
    public class TestHtmlElementManager
    {
        /// <summary>
        /// ID that is used for the scratch element by default.
        /// </summary>
        private const string DefaultScratchElementID = "__scratchElement";

        /// <summary>
        /// HTML element manager.  Static instance.
        /// </summary>
        private static TestHtmlElementManager _instance = new TestHtmlElementManager();

        /// <summary>
        /// Gets a single instance of the scratch element manager.
        /// </summary>
        internal static TestHtmlElementManager Instance
        {
            get { return _instance; }
        }

        /// <summary>
        /// One-time constructor.
        /// </summary>
        private TestHtmlElementManager()
        {
            ClearScratchElement();
        }

        /// <summary>
        /// Scratch element reference.
        /// </summary>
        private HtmlElement _scratchElement;

        /// <summary>
        /// Value indicating whether the element is dirty.
        /// </summary>
        private bool _dirty;

        /// <summary>
        /// Gets the scratch element and marks that it has been used.  After 
        /// the test method completes, it will have its children elements 
        /// cleared automatically.
        /// </summary>
        public HtmlElement ScratchElement
        {
            get
            {
                _dirty = true;
                return _scratchElement;
            }
        }

        /// <summary>
        /// Removes the underlying children of the html element if dirty.
        /// </summary>
        internal void ClearDirtyScratchElement()
        {
            if (_dirty) 
            {
                ClearScratchElement();
            }
        }

        /// <summary>
        /// Recreates the scratch html element.
        /// </summary>
        public void ClearScratchElement()
        {
            if (_scratchElement != null)
            {
                RemoveScratchElement();
            }

            _scratchElement = HtmlPage.Document.CreateElement("div");
            _scratchElement.SetAttribute("id", DefaultScratchElementID);

            if (HtmlPage.Document.Body.Children.Count > 0)
            {
                HtmlElement first = HtmlPage.Document.Body.Children[0] as HtmlElement;
                HtmlPage.Document.Body.AppendChild(_scratchElement, first);
            }
            else
            {
                HtmlPage.Document.Body.AppendChild(_scratchElement);
            }

            _dirty = false;
        }

        /// <summary>
        /// Remove the scratch element, if any, from its parent.
        /// </summary>
        private void RemoveScratchElement()
        {
            if (_scratchElement == null)
            {
                _scratchElement = HtmlPage.Document.GetElementById(DefaultScratchElementID);
            }
            if (_scratchElement == null)
            {
                return;
            }

            HtmlElement parent = _scratchElement.Parent;
            if (parent == null)
            {
                return;
            }

            parent.RemoveChild(_scratchElement);
            _scratchElement = null;
        }
    }
}