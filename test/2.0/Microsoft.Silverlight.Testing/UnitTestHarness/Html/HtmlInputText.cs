// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A HTML text input control.
    /// </summary>
    public class HtmlInputText : HtmlInputControl
    {
        /// <summary>
        /// Constant string, "text".
        /// </summary>
        private const string TextValue = "text";

        /// <summary>
        /// Initializes a new text input control instance.
        /// </summary>
        public HtmlInputText() : base(TextValue)
        {
            AttachEvent(HtmlEvent.Change, Changed);
        }

        /// <summary>
        /// Gets or sets the maximum length of text for the input control.
        /// </summary>
        public int MaxLength
        {
            get
            {
                string i = GetAttributeOrNull(HtmlAttribute.Maxlength);
                if (i == null)
                {
                    return -1;
                }
                return int.Parse(i, CultureInfo.InvariantCulture);
            }

            set
            {
                SetAttribute(HtmlAttribute.Maxlength, value.ToString(CultureInfo.InvariantCulture));
            }
        }

        /// <summary>
        /// Gets or sets the size of the input control.
        /// </summary>
        public int Size
        {
            get
            {
                string i = GetAttributeOrNull(HtmlAttribute.Size);
                if (i == null)
                {
                    return -1;
                }
                return int.Parse(i, CultureInfo.InvariantCulture);
            }

            set
            {
                SetAttribute(HtmlAttribute.Size, value.ToString(CultureInfo.InvariantCulture));
            }
        }

        /// <summary>
        /// The changed event for the input control.
        /// </summary>
        public event EventHandler Changed;
    }
}