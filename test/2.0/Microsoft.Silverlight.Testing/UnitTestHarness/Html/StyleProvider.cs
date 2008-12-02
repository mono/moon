// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Used for applying a style to all new controls.
    /// </summary>
    public abstract class StyleProvider
    {
        /// <summary>
        /// Applies the styles to the control.  Derived classes can customize 
        /// the type, tag, or other properties of the controls to limit when the 
        /// styles are applied.
        /// </summary>
        /// <param name="control">The control reference.</param>
        public virtual void ApplyStyle(HtmlControl control)
        {
            // The default implementation does nothing
        }
    }
}