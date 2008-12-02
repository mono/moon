// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.IO;
using System.Net;
using System.Reflection;

#if SILVERLIGHT
using System.Windows;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.UI;
#endif

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// Utilities, helpers, and other code designed for use when testing HTML 
    /// bridge (and other browser pieces) using the Silverlight product and 
    /// test framework.
    /// </summary>
    public static class Browser
    {
        /// <summary>
        /// Information about the current browser.
        /// </summary>
        private static BrowserInformation _browserInformation = new BrowserInformation();

        /// <summary>
        /// Gets Information about the current browser.
        /// </summary>
        public static BrowserInformation Information
        {
            get { return _browserInformation; }
        }

        /// <summary>
        /// Create a new HtmlElement which represents JavaScript.
        /// </summary>
        /// <param name="javaScriptContents">JavaScript code to include.</param>
        /// <returns>New HTML element representing the JavaScript script 
        /// element.</returns>
        public static HtmlElement CreateJavaScriptElement(string javaScriptContents)
        {
            HtmlElement js = HtmlPage.Document.CreateElement("script");
            js.SetAttribute("type", "text/javascript");
            js.SetAttribute("language", "JavaScript");
            js.SetProperty("text", javaScriptContents);

            return js;
        }

        /// <summary>
        /// Simulate clicking on an HTML element using the HTML DOM bridge to 
        /// invoke the "click" method on the element.
        /// </summary>
        /// <param name="htmlElement">HTML element reference.</param>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "HtmlElements have innerHTML properties, ScriptObjects do not")]
        public static void ClickElement(HtmlElement htmlElement)
        {
            htmlElement.Invoke("click", new object[0]);
        }

        /// <summary>
        /// Simulate clicking on an HTML element using the HTML DOM bridge to 
        /// invoke the "click" method on the element.
        /// </summary>
        /// <param name="elementId">ID of the HTML element to click.</param>
        public static void ClickElement(string elementId)
        {
            ClickElement(HtmlPage.Document.GetElementById(elementId));
        }

        /// <summary>
        /// Append HTML, using innerHTML, inside an element in the 
        /// ScratchElement.
        /// </summary>
        /// <param name="htmlContent">HTML content to append to the scratch 
        /// element.</param>
        public static void AppendScratchHtml(string htmlContent)
        {
            HtmlElement div = HtmlPage.Document.CreateElement("div");
            ScratchElement.AppendChild(div);
            div.SetProperty("innerHTML", htmlContent);
        }

        /// <summary>
        /// Gets a "scratch" element on the page which can be used for standard 
        /// HTML DOM bridge tests and other uses.  
        /// 
        /// Since the lifetime of the SilverlightTest is only that of a single 
        /// TestMethod and its execution, calling ScratchElement in a test 
        /// method guarantees that you will have a fresh element with no 
        /// siblings barring any browser or product bugs.
        /// </summary>
        public static HtmlElement ScratchElement
        {
            get { return TestHtmlElementManager.Instance.ScratchElement; }
        }
    }
}