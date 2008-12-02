// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Provides screen information about the browser.
    /// </summary>
    /// <remarks>A simple proxy to JavaScript window and screen variables,
    /// abstracts away common web browser differences.</remarks>
    public static class BrowserScreenInformation
    {
        /// <summary>
        /// Flag indicating Navigator/Firefox/Safari or Internet Explorer.
        /// </summary>
        private readonly static bool _isNavigator = HtmlPage.BrowserInformation.Name.Contains("Netscape");

        /// <summary>
        /// Gets the window.screen ScriptObject.
        /// </summary>
        private static ScriptObject Screen
        {
            get
            {
                ScriptObject screen = (ScriptObject)HtmlPage.Window.GetProperty("screen");

                if (screen == null)
                {
                    throw new InvalidOperationException();
                }

                return screen;
            }
        }

        /// <summary>
        /// Gets the window object's client width.
        /// </summary>
        public static double ClientWidth
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("innerWidth")
                    : (double)HtmlPage.Document.Body.GetProperty("clientWidth");
            }
        }

        /// <summary>
        /// Gets the window object's client height.
        /// </summary>
        public static double ClientHeight
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("innerHeight")
                    : (double)HtmlPage.Document.Body.GetProperty("clientHeight");
            }
        }

        /// <summary>
        /// Gets the current horizontal scrolling offset.
        /// </summary>
        public static double ScrollLeft
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("pageXOffset")
                    : (double)HtmlPage.Document.Body.GetProperty("scrollLeft");
            }
        }

        /// <summary>
        /// Gets the current vertical scrolling offset.
        /// </summary>
        public static double ScrollTop
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("pageYOffset")
                    : (double)HtmlPage.Document.Body.GetProperty("scrollHeight");
            }
        }

        /// <summary>
        /// Gets the width of the entire display.
        /// </summary>
        public static double ScreenWidth
        {
            get
            {
                return (double)Screen.GetProperty("width");
            }
        }

        /// <summary>
        /// Gets the height of the entire display.
        /// </summary>
        public static double ScreenHeight
        {
            get
            {
                return (double)Screen.GetProperty("height");
            }
        }

        /// <summary>
        /// Gets the width of the available screen real estate, excluding the 
        /// dock or task bar.
        /// </summary>
        public static double AvailableScreenWidth
        {
            get
            {
                return (double)Screen.GetProperty("availWidth");
            }
        }

        /// <summary>
        /// Gets the height of the available screen real estate, excluding the 
        /// dock or task bar.
        /// </summary>
        public static double AvailableScreenHeight
        {
            get
            {
                return (double)Screen.GetProperty("availHeight");
            }
        }

        /// <summary>
        /// Gets the absolute left pixel position of the window in display 
        /// coordinates.
        /// </summary>
        public static double ScreenPositionLeft
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("screenX")
                    : (double)HtmlPage.Window.GetProperty("screenLeft");
            }
        }

        /// <summary>
        /// Gets the absolute top pixel position of the window in display 
        /// coordinates.
        /// </summary>
        public static double ScreenPositionTop
        {
            get
            {
                return _isNavigator ? (double)HtmlPage.Window.GetProperty("screenY")
                    : (double)HtmlPage.Window.GetProperty("screenTop");
            }
        }
    }
}