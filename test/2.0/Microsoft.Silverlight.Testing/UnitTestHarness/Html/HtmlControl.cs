// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using System.Collections.Generic;
using System.Globalization;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A managed HTML control that provides many of the same services as 
    /// HtmlElement, plus style, property and basic extensions.
    /// 
    /// Any HtmlElement can be provided in the constructor to get access to the 
    /// helper methods, strong type properties, and other tools.
    /// </summary>
    public class HtmlControl : HtmlControlBase
    {
        /// <summary>
        /// The shared style provider.
        /// </summary>
        private static StyleProvider _styleProvider;

        /// <summary>
        /// The border helper object.
        /// </summary>
        private CssBorder _advancedBorder;

        /// <summary>
        /// The padding helper object.
        /// </summary>
        private CssPadding _padding;

        /// <summary>
        /// The margin helper object.
        /// </summary>
        private CssMargin _margin;

        /// <summary>
        /// The position helper object.
        /// </summary>
        private CssPosition _position;

        /// <summary>
        /// The FontInfo helper object.
        /// </summary>
        private FontInfo _fontInfo;

        /// <summary>
        /// Initializes a new HtmlControl object.  Takes the parent managed 
        /// control, and the TagName to create for this managed control. 
        /// Automatically the object is inserted into the page and the parent 
        /// Control.
        /// </summary>
        /// <param name="tag">The TagName to create.</param>
        public HtmlControl(string tag) : base(tag) 
        {
            ApplyStyles();
        }

        /// <summary>
        /// Initializes a new HtmlControl object.  Takes the parent managed 
        /// control, and the TagName to create for this managed control. 
        /// Automatically the object is inserted into the page and the parent 
        /// Control.
        /// </summary>
        /// <param name="tag">The TagName to create.</param>
        public HtmlControl(HtmlTag tag) : base(tag) 
        {
            ApplyStyles();
        }

        /// <summary>
        /// Initializes a new HtmlControl object.  The HtmlElement will become 
        /// the root of this particular HTML element tree island.
        /// </summary>
        /// <param name="root">The root, already-existing HTML element to turn 
        /// into a managed HtmlControl.</param>
        public HtmlControl(HtmlElement root) : base(root) 
        {
            ApplyStyles();
        }

        /// <summary>
        /// Uses the shared style provider to apply defaults to the object.
        /// </summary>
        private void ApplyStyles()
        {
            if (_styleProvider != null)
            {
                _styleProvider.ApplyStyle(this);
            }
        }

        /// <summary>
        /// Gets or sets the style provider used for all new HtmlControl 
        /// objects.
        /// </summary>
        public static StyleProvider StyleProvider
        {
            get { return _styleProvider; }

            set { _styleProvider = value; }
        }

        /// <summary>
        /// Gets or sets the background color of the control.
        /// </summary>
        public string BackgroundColor
        {
            get
            {
                return GetStyleAttribute(CssAttribute.BackgroundColor);
            }

            set
            {
                SetStyleAttribute(CssAttribute.BackgroundColor, value);
            }
        }

        /// <summary>
        /// Gets or sets the text (foreground) color of the element.
        /// </summary>
        public string ForegroundColor
        {
            get
            {
                return GetStyleAttribute(CssAttribute.Color);
            }

            set
            {
                SetStyleAttribute(CssAttribute.Color, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether the control is Enabled.
        /// </summary>
        public bool Enabled
        {
            get
            {
                bool disabled = false;
                object obj = GetProperty(HtmlProperty.Disabled);
                if (obj != null)
                {
                    disabled = (bool)obj;
                }
                return !disabled;
            }

            set
            {
                value = !value;
                SetProperty(HtmlProperty.Disabled, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether the element is visible on 
        /// the page.
        /// </summary>
        public bool Visible
        {
            get
            {
                string s = GetStyleAttribute(CssAttribute.Display);
                return s != CssDisplay.None.ScriptPropertyName();
            }

            set
            {
                if (value)
                {
                    SetStyleAttribute(CssAttribute.Display, CssDisplay.Block);
                }
                else
                {
                    SetStyleAttribute(CssAttribute.Display, CssDisplay.None);
                }
            }
        }

        /// <summary>
        /// Gets or sets the border color of the element.
        /// </summary>
        public string BorderColor
        {
            get
            {
                return GetStyleAttribute(CssAttribute.BorderColor);
            }

            set
            {
                SetStyleAttribute(CssAttribute.BorderColor, value);
            }
        }

        /// <summary>
        /// Sets a number of border properties for the element.
        /// </summary>
        /// <param name="width">The width of the border.</param>
        /// <param name="color">The border color.</param>
        public void SetBorder(Unit width, string color)
        {
            SetBorder(width, color, BorderStyle.Solid);
        }

        /// <summary>
        /// Sets a number of border properties for the element.
        /// </summary>
        /// <param name="width">The width of the border.</param>
        /// <param name="color">The border color.</param>
        /// <param name="style">The border style.</param>
        public void SetBorder(Unit width, string color, BorderStyle style)
        {
            BorderWidth = width;
            BorderColor = color;
            BorderStyle = style;
        }

        /// <summary>
        /// Gets or sets the tax index of the control.
        /// </summary>
        public int TaxIndex
        {
            get
            {
                string v = GetAttribute(HtmlAttribute.Tabindex);
                return int.Parse(v, CultureInfo.InvariantCulture);
            }

            set
            {
                SetAttribute(HtmlAttribute.Tabindex, value.ToString(CultureInfo.InvariantCulture));
            }
        }

        /// <summary>
        /// Gets or sets the Display property of the control (CSS).
        /// </summary>
        public CssDisplay Display
        {
            get
            {
                string s = GetStyleAttribute(CssAttribute.Display);
                return (CssDisplay)Enum.Parse(typeof(CssDisplay), s, true);
            }

            set
            {
                SetStyleAttribute(CssAttribute.Display, value);
            }
        }

        /// <summary>
        /// Gets or sets the border style of the element.
        /// </summary>
        public BorderStyle BorderStyle
        {
            get
            {
                BorderStyle style;
                string property = GetStyleAttribute(CssAttribute.BorderStyle);
                style = (BorderStyle)Enum.Parse(typeof(BorderStyle), property, true);
                return style;
            }

            set
            {
                SetStyleAttribute(CssAttribute.BorderStyle, value);
            }
        }

        /// <summary>
        /// Gets or sets the position of the control.
        /// </summary>
        public CssPosition Position
        {
            get
            {
                if (_position == null)
                {
                    _position = new CssPosition(this);
                }
                return _position;
            }

            set
            {
                _position.Bottom = value.Bottom;
                _position.Left = value.Left;
                _position.Right = value.Right;
                _position.Top = value.Top;
            }
        }

        /// <summary>
        /// Gets or sets the padding of the element.
        /// </summary>
        public CssPadding Padding
        {
            get
            {
                if (_padding == null)
                {
                    _padding = new CssPadding(this);
                }
                return _padding;
            }

            set
            {
                _padding.Bottom = value.Bottom;
                _padding.Left = value.Left;
                _padding.Right = value.Right;
                _padding.Top = value.Top;
            }
        }

        /// <summary>
        /// Gets or sets the margin of the element.
        /// </summary>
        public CssMargin Margin
        {
            get
            {
                if (_margin == null)
                {
                    _margin = new CssMargin(this);
                }
                return _margin;
            }

            set
            {
                _margin.Bottom = value.Bottom;
                _margin.Left = value.Left;
                _margin.Right = value.Right;
                _margin.Top = value.Top;
            }
        }

        /// <summary>
        /// Gets or sets the font of the element.
        /// </summary>
        public FontInfo Font
        {
            get
            {
                if (_fontInfo == null)
                {
                    _fontInfo = new FontInfo(this);
                }
                return _fontInfo;
            }

            set
            {
                // one-way setter
                Font.CopyFontNamesAndSizeFrom(value);
            }
        }

        /// <summary>
        /// Gets or sets the width of the element.
        /// </summary>
        public Unit Width
        {
            get
            {
                string val = GetStyleAttribute(CssAttribute.Width);
                return new Unit(val, CultureInfo.InvariantCulture);
            }

            set
            {
                SetStyleAttribute(CssAttribute.Width, value);
            }
        }

        /// <summary>
        /// Gets or sets the height of the element.
        /// </summary>
        public Unit Height
        {
            get
            {
                string val = GetStyleAttribute(CssAttribute.Height);
                return new Unit(val, CultureInfo.InvariantCulture);
            }

            set
            {
                SetStyleAttribute(CssAttribute.Height, value);
            }
        }
        
        /// <summary>
        /// Gets or sets the width of the border.
        /// </summary>
        public Unit BorderWidth
        {
            get
            {
                string val = GetStyleAttribute(CssAttribute.BorderWidth);
                return new Unit(val, CultureInfo.InvariantCulture);
            }

            set
            {
                SetStyleAttribute(CssAttribute.BorderWidth, value);
            }
        }

        /// <summary>
        /// Gets or sets the advanced border properties, enabling specific 
        /// left/right/bottom/top widths.
        /// </summary>
        public CssBorder BorderWidthAdvanced
        {
            get
            {
                if (_advancedBorder == null)
                {
                    _advancedBorder = new CssBorder(this);
                }

                return _advancedBorder;
            }

            set
            {
                CssBorder thisBorder = BorderWidthAdvanced; // get

                // COPY VALUES ONLY!
                thisBorder.Bottom = value.Bottom;
                thisBorder.Left = value.Left;
                thisBorder.Right = value.Right;
                thisBorder.Top = value.Top;
            }
        }

        /// <summary>
        /// Focuses the browser on the control.
        /// </summary>
        public void Focus()
        {
            RequireInitialization();
            HtmlElement.Focus();
        }
    }
}