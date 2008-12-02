// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// An attached, 4-component unit box for CSS values.
    /// </summary>
    public class CssBox : Box
    {
        /// <summary>
        /// The managed control.
        /// </summary>
        private HtmlControlBase _mhe;

        /// <summary>
        /// The enum property names.
        /// </summary>
        private CssAttribute[] _cssProperties;

        /// <summary>
        /// Initializes a new CssBox type with the specified CSS property names.
        /// </summary>
        /// <param name="element">The parent managed HTML element.</param>
        /// <param name="top">The top property.</param>
        /// <param name="right">The right property.</param>
        /// <param name="bottom">The bottom property.</param>
        /// <param name="left">The left property.</param>
        public CssBox(HtmlControlBase element, CssAttribute top, CssAttribute right, CssAttribute bottom, CssAttribute left) : base()
        {
            _mhe = element;
            _cssProperties = new CssAttribute[] 
            {
                top,
                right,
                bottom,
                left
            };
        }

        /// <summary>
        /// Gets the managed HTML element.
        /// </summary>
        protected HtmlControlBase ManagedHtmlElement
        {
            get
            {
                return _mhe;
            }
        }

        /// <summary>
        /// Sets a style property.
        /// </summary>
        /// <param name="style">The style name.</param>
        /// <param name="value">The value to set.</param>
        protected void SetStyleProperty(CssAttribute style, Unit value)
        {
            if (value == null || value.IsEmpty)
            {
                _mhe.RemoveStyleAttribute(style);
            }
            else
            {
                _mhe.SetStyleAttribute(style, value);
            }
        }

        /// <summary>
        /// Gets or sets the Top value.
        /// </summary>
        public override Unit Top
        {
            get
            {
                return base.Top;
            }
            set
            {
                SetStyleProperty(_cssProperties[0], value);
                base.Top = value;
            }
        }

        /// <summary>
        /// Gets or sets the Right value.
        /// </summary>
        public override Unit Right
        {
            get
            {
                return base.Right;
            }
            set
            {
                SetStyleProperty(_cssProperties[1], value);
                base.Right = value;
            }
        }

        /// <summary>
        /// Gets or sets the Bottom value.
        /// </summary>
        public override Unit Bottom
        {
            get
            {
                return base.Bottom;
            }
            set
            {
                SetStyleProperty(_cssProperties[2], value);
                base.Bottom = value;
            }
        }

        /// <summary>
        /// Gets or sets the left value.
        /// </summary>
        public override Unit Left
        {
            get
            {
                return base.Left;
            }
            set
            {
                SetStyleProperty(_cssProperties[3], value);
                base.Left = value;
            }
        }
    }
}