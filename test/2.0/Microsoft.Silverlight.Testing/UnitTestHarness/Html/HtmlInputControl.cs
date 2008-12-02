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
    /// A HTML input control.
    /// </summary>
    public class HtmlInputControl : HtmlContainerControl
    {
        /// <summary>
        /// The type of input control.
        /// </summary>
        private string _type;

        /// <summary>
        /// Initializes a new HtmlInputControl object.
        /// </summary>
        /// <param name="type">The type of input control.</param>
        public HtmlInputControl(string type) : base(HtmlTag.Input)
        {
            _type = type;
            SetAttribute(HtmlAttribute.Type, _type);
        }

        /// <summary>
        /// Gets or sets the value of the input control.
        /// </summary>
        public string Value
        {
            get
            {
                return GetAttributeOrNull(HtmlAttribute.Value);
            }

            set
            {
                SetAttribute(HtmlAttribute.Value, value);
            }
        }

        /// <summary>
        /// Gets or sets the name attribute for the control.
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
        /// Gets the input control's type string.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods", Justification = "This property name is familiar to HTML and ASP.NET developers.")]
        public string Type
        {
            get
            {
                return _type;
            }
        }
    }
}