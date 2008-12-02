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
    public class WebBrowserTestPage
    {
        /// <summary>
        /// The default minimum size of the plugin width.
        /// </summary>
        internal const int DefaultMinimumPluginWidth = 400;

        /// <summary>
        /// The default minimum size of the plugin height.
        /// </summary>
        internal const int DefaultMinimumPluginHeight = 280;

        /// <summary>
        /// The default width of the test column.
        /// </summary>
        internal const int DefaultTestColumnWidth = 370;

        /// <summary>
        /// Amount of padding to have between the column and the plugin. 
        /// Not adjustable.
        /// </summary>
        private const int ImplicitPadding = 10;

        /// <summary>
        /// The default font size to use in logging operations.
        /// </summary>
        public readonly static Unit DefaultFontSize = new Unit(12, UnitType.Pixel);

        /// <summary>
        /// Font used for text.
        /// </summary>
        public const string DefaultFontFamily = "Calibri, Arial, sans-serif";

        /// <summary>
        /// Font used to fixed width text.
        /// </summary>
        public const string DefaultFixedFontFamily = "Consolas, Courier New, courier";

        /// <summary>
        /// The set of default fonts.
        /// </summary>
        private static string[] _defaultFontFamilies;

        /// <summary>
        /// The set of default fixed fonts.
        /// </summary>
        private static string[] _defaultFixedFontFamilies;

        /// <summary>
        /// The Silverlight plugin HTML element.
        /// </summary>
        private HtmlControl _plugin;

        /// <summary>
        /// The container of the Silverlight plugin.
        /// </summary>
        private HtmlControl _pluginContainer;

        /// <summary>
        /// The primary test column.
        /// </summary>
        private HtmlTestColumn _testColumn;
        
        /// <summary>
        /// The minimum size of the plugin.
        /// </summary>
        private Size<int> _minimumPluginSize;

        /// <summary>
        /// The known size of the client.
        /// </summary>
        private Size<int> _browserClientSize;

        /// <summary>
        /// The current size allocated and calculated for the plugin.
        /// </summary>
        private Size<int> _pluginSize;

        /// <summary>
        /// The width allocated for the test column.
        /// </summary>
        private int _testColumnWidth;

        /// <summary>
        /// Initializes the HtmlTestPage Control.
        /// </summary>
        /// <param name="permitResizing">A value indicating whether to allow 
        /// the plugin and page to resize the contents.</param>
        public WebBrowserTestPage(bool permitResizing) : this(DefaultMinimumPluginWidth, DefaultMinimumPluginHeight, DefaultTestColumnWidth, permitResizing) { }

        /// <summary>
        /// Initializes the HtmlTestPage Control with a specific minimum plugin 
        /// size requirement.
        /// </summary>
        /// <param name="minimumPluginWidth">The minimum width that the plugin 
        /// can be sized to.</param>
        /// <param name="minimumPluginHeight">The minimum height that the plugin
        /// can be sized to.</param>
        /// <param name="testColumnWidth">The width to allocate for the test 
        /// column.</param>
        /// <param name="permitResizing">A value indicating whether to allow 
        /// the plugin and page to resize the contents.</param>
        public WebBrowserTestPage(int minimumPluginWidth, int minimumPluginHeight, int testColumnWidth, bool permitResizing)
        {
            PrepareApplicationFonts();

            _testColumnWidth = testColumnWidth;
            _minimumPluginSize = new Size<int>(minimumPluginWidth, minimumPluginHeight);
            _pluginContainer = new HtmlControl(HtmlPage.Plugin.Parent);
            _plugin = new HtmlControl(HtmlPage.Plugin);

            CalculatePreferredPluginSize();

            if (permitResizing)
            {
                ResizeSilverlightPlugin();
            }
            
            // Appended directly into the document body
            _testColumn = new HtmlTestColumn(_testColumnWidth, ImplicitPadding, _browserClientSize.Height);
            HtmlPage.Document.Body.AppendChild(_testColumn);
        }

        /// <summary>
        /// Sets the font of the control to be the default font for the test 
        /// framework.
        /// </summary>
        /// <param name="control">The managed HTML control.</param>
        public static void SetDefaultFont(HtmlControl control)
        {
            control.Font.Names = _defaultFontFamilies;
        }

        /// <summary>
        /// Sets the font of the control to be the default fixed-size font for 
        /// the test framework.
        /// </summary>
        /// <param name="control">The managed HTML control.</param>
        public static void SetDefaultFixedFont(HtmlControl control)
        {
            control.Font.Names = _defaultFixedFontFamilies;
        }

        /// <summary>
        /// Sets the font size of the control to be the default size for the 
        /// test framework.
        /// </summary>
        /// <param name="control">The managed HTML control.</param>
        public static void SetDefaultFontSize(HtmlControl control)
        {
            control.Font.Size = new FontUnit(DefaultFontSize);
        }

        /// <summary>
        /// Prepares the fonts for use in the HTML-side of the test framework's
        /// application.
        /// </summary>
        private static void PrepareApplicationFonts()
        {
            FontNamesConverter fonts = new FontNamesConverter();
            _defaultFontFamilies = (string[])fonts.ConvertFromString(DefaultFontFamily);
            _defaultFixedFontFamilies = (string[])fonts.ConvertFromString(DefaultFixedFontFamily);

            HtmlControl.StyleProvider = new TestStyleProvider();
        }

        /// <summary>
        /// Calculate the size of the Silverlight control to meet a set of 
        /// hard-coded dimensions and resize the plugin and plugin element.
        /// </summary>
        private void CalculatePreferredPluginSize()
        {
            _browserClientSize = new Size<int> 
            { 
                Width = (int) BrowserScreenInformation.ClientWidth, 
                Height = (int) BrowserScreenInformation.ClientHeight 
            };
            
            int actualWidth = _browserClientSize.Width - ImplicitPadding - _testColumnWidth;
            int controlWidth = actualWidth < _minimumPluginSize.Width ? _minimumPluginSize.Width : actualWidth;
            int actualHeight = _browserClientSize.Height - ImplicitPadding;
            int controlHeight = actualHeight < _minimumPluginSize.Height ? _minimumPluginSize.Height : actualHeight;
            _pluginSize = new Size<int>(controlWidth, controlHeight);
            
            int columnSizing = _browserClientSize.Width - controlWidth - ImplicitPadding;
            if (columnSizing < _testColumnWidth)
            {
                // Shrink the test column
                _testColumnWidth = columnSizing < 0 ? 0 : columnSizing;
            }
        }

        /// <summary>
        /// Gets the plugin element.
        /// </summary>
        public HtmlControl Plugin
        {
            get
            {
                return _plugin;
            }
        }

        /// <summary>
        /// Gets the container of the plugin object.
        /// </summary>
        public HtmlControl PluginContainer
        {
            get
            {
                return _pluginContainer;
            }
        }

        /// <summary>
        /// Gets the test column.
        /// </summary>
        public HtmlTestColumn TestColumn
        {
            get
            {
                return _testColumn;
            }
        }

        /// <summary>
        /// Resize the plugin using the preferred sizes.
        /// </summary>
        private void ResizeSilverlightPlugin()
        {
            Plugin.Width = _pluginSize.Width;
            Plugin.Height = _pluginSize.Height;
            PluginContainer.Width = Plugin.Width;
            PluginContainer.Height = Plugin.Height;

            // TODO: update the attach and onresize code here [see: below]
        }

        ////HtmlPage.Window.AttachEvent(
        ////    "onresize",
        ////    delegate(object sender, HtmlEventArgs ea)
        ////    {
        ////        // previous size
        ////        int oldWidth = _clientSpace.Width;
        ////        int newWidth = (int)((double)HtmlPage.Document.Body.GetProperty("clientWidth"));

        ////        if (newWidth < oldWidth)
        ////        {
        ////            int change = oldWidth - newWidth;

        ////            // set plugin size
        ////            _pluginSpace.Width = Math.Max(0, _pluginSpace.Width - change);
        ////            pluginElement.SetStyleAttribute("width", AsPixels(_pluginSpace.Width));
        ////            _clientSpace.Width = newWidth;
        ////        }
        ////    });
    }
}