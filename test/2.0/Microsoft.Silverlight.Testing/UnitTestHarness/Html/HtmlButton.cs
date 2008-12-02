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
    /// An HtmlButton control.
    /// </summary>
    public class HtmlButton : HtmlContainerControl
    {
        /// <summary>
        /// Initializes a new HtmlButton object.
        /// </summary>
        public HtmlButton() : base(HtmlTag.Button) 
        {
            AttachEvent(HtmlEvent.Click, new EventHandler <HtmlEventArgs>(OnClick));
        }
        
        /// <summary>
        /// The Click event handler.
        /// </summary>
        public event EventHandler<HtmlEventArgs> Click;

        /// <summary>
        /// Calls the Click event handler.  The source object is the button.
        /// </summary>
        /// <param name="sender">The event source.</param>
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