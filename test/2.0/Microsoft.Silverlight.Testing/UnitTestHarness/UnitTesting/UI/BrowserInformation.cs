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
    /// Information about the web browser.
    /// </summary>
    public class BrowserInformation
    {
        /// <summary>
        /// Construct a new browser information instance.
        /// </summary>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "Method not available in the Core CLR")]
        internal BrowserInformation()
        {
            string b = HtmlPage.BrowserInformation.UserAgent.ToLower(CultureInfo.InvariantCulture);

            _firefox = b.Contains("firefox");
            _ie = b.Contains("msie") && !_firefox;
            _safari = b.Contains("safari");
        }

        /// <summary>
        /// Value indicating whether Firefox is being used.
        /// </summary>
        private bool _firefox;

        /// <summary>
        /// Value indicating whether Windows Internet Explorer is being used.
        /// </summary>
        private bool _ie;

        /// <summary>
        /// Value indicating whether Apple Safari is being used.
        /// </summary>
        private bool _safari;

        /// <summary>
        /// Gets a value indicating whether Firefox is being used.
        /// </summary>
        public bool IsFirefox
        {
            get { return _firefox; }
        }

        /// <summary>
        /// Gets a value indicating whether Internet Explorer is in use.
        /// </summary>
        public bool IsIE
        {
            get { return _ie; }
        }

        /// <summary>
        /// Gets a value indicating whether Safari is in use.
        /// </summary>
        public bool IsSafari
        {
            get { return _safari; }
        }

        /// <summary>
        /// Gets a value indicating whether a non-IE browser is in use.
        /// </summary>
        public bool IsNotIE
        {
            get { return !IsIE; }
        }

        /// <summary>
        /// Gets a value indicating whether the current platform is Windows.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Improves usability of helper.")]
        public bool IsWindows
        {
            get { return (Environment.OSVersion.Platform == PlatformID.Win32NT); }
        }

        /// <summary>
        /// Gets a value indicating whether the current platform is Apple OS X.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Improves usability of helper.")]
        public bool IsMac
        {
            get { return Environment.OSVersion.Platform == PlatformID.MacOSX; }
        }
    }
}