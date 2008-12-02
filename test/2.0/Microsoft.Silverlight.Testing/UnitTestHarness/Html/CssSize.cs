// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A height and width type that attaches to a control to update HtmlElement
    /// values directly.
    /// </summary>
    public class CssSize : Size<Unit>
    {
        /// <summary>
        /// The managed Control reference.
        /// </summary>
        private HtmlControlBase _mhe;

        /// <summary>
        /// Stored property enum names.
        /// </summary>
        private CssAttribute[] _cssProperties;

        /// <summary>
        /// Initializes a new CssSize.
        /// </summary>
        /// <param name="element">The element.</param>
        /// <param name="width">The width CSS property.</param>
        /// <param name="height">The height CSS property.</param>
        public CssSize(HtmlControlBase element, CssAttribute width, CssAttribute height)
            : base()
        {
            _mhe = element;
            _cssProperties = new CssAttribute[] 
            {
                width,
                height
            };
        }

        /// <summary>
        /// Sets the style property.
        /// </summary>
        /// <param name="style">The style enum name.</param>
        /// <param name="value">The new value.</param>
        private void SetStyleProperty(CssAttribute style, Unit value)
        {
            if (value == null)
            {
                _mhe.RemoveStyleAttribute(style);
            }
            else
            {
                _mhe.SetStyleAttribute(style, value.ToString());
            }
        }

        /// <summary>
        /// Gets or sets the width unit.
        /// </summary>
        public override Unit Width
        {
            get
            {
                return base.Width;
            }

            set
            {
                SetStyleProperty(_cssProperties[0], value);
                base.Width = value;
            }
        }

        /// <summary>
        /// Gets or sets the height Unit.
        /// </summary>
        public override Unit Height
        {
            get
            {
                return base.Height;
            }

            set
            {
                SetStyleProperty(_cssProperties[1], value);
                base.Width = value;
            }
        }
    }
}