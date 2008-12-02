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
    /// Style provider for the test framework.
    /// </summary>
    public class TestStyleProvider : StyleProvider
    {
        /// <summary>
        /// Applies the test framework styles to the control.
        /// </summary>
        /// <param name="control">The control reference.</param>
        public override void ApplyStyle(HtmlControl control)
        {
            // For all controls, set the default font and size

            WebBrowserTestPage.SetDefaultFont(control);
            WebBrowserTestPage.SetDefaultFontSize(control);
        }
    }
}