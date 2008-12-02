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
    /// The test column's header control.
    /// </summary>
    public class TestColumnHeader : HtmlDiv
    {
        /// <summary>
        /// The fixed name used for test column indicator's class names.
        /// </summary>
        public const string TestColumnIndicator = "test-column-indicator";

        /// <summary>
        /// The pass/fail indicator area.
        /// </summary>
        private HtmlDiv _indicator;

        /// <summary>
        /// The framework title.
        /// </summary>
        private Paragraph _about;

        /// <summary>
        /// Initializes a new column header.
        /// </summary>
        public TestColumnHeader() : base()
        {
            StylizeHeader();
            CreateAbout();
            CreateIndicator();
        }

        /// <summary>
        /// Initializes a new column header.
        /// </summary>
        /// <param name="displayName">The display name of the framework.</param>
        public TestColumnHeader(string displayName)
            : this()
        {
            DisplayProductName = displayName;
        }

        /// <summary>
        /// Gets or sets the display name for the framework/product.
        /// </summary>
        public string DisplayProductName
        {
            get
            {
                return _about.InnerHtml;
            }

            set
            {
                _about.InnerHtml = value;
            }
        }

        /// <summary>
        /// Sets the color of the result indicator in the header.
        /// </summary>
        /// <param name="runInProgress">A value indicating whether the run is 
        /// still in progress, or has completed.</param>
        /// <param name="runIsSuccessful">A value indicating whether the run 
        /// results indicate success so far.</param>
        public void UpdateIndicatorColoring(bool runInProgress, bool runIsSuccessful)
        {
            string color = runIsSuccessful ?
                /* pass */ (runInProgress ? ResultIndicatorColor.InProgressSuccess : ResultIndicatorColor.FinishedSuccess) :
                /* fail */ (runInProgress ? ResultIndicatorColor.InProgressFailure : ResultIndicatorColor.FinishedFailure);
            string background = runIsSuccessful ? ResultIndicatorColor.BackgroundSuccess : ResultIndicatorColor.BackgroundFailure;
            
            _indicator.BackgroundColor = color;
           BackgroundColor = background;
        }

        /// <summary>
        /// Creates the section that displays that about box / framework title.
        /// </summary>
        private void CreateAbout()
        {
            _about = new Paragraph();
            _about.Font.Size = new FontUnit(16, UnitType.Pixel);
            _about.Margin.All = 0;
            _about.Padding.All = 0;
            Controls.Add(_about);
        }

        /// <summary>
        /// Creates the indicator area.
        /// </summary>
        private void CreateIndicator()
        {
            _indicator = new HtmlDiv();
            _indicator.Position.Bottom = _indicator.Position.Left = _indicator.Position.Right = 0;
            _indicator.Height = new Unit(12, UnitType.Pixel);
            _indicator.CssClass = TestColumnIndicator;
            _indicator.SetStyleAttribute(CssAttribute.Position, "absolute");
            Controls.Add(_indicator);
        }

        /// <summary>
        /// Styles the header.
        /// </summary>
        private void StylizeHeader()
        {
            Height = 34;
            SetStyleAttribute(CssAttribute.Position, "relative");
            Padding.All = 5;
            BackgroundColor = "#fff9d8";
            BorderWidth = 0;
            BorderWidthAdvanced.Bottom = 1;
            BorderStyle = BorderStyle.Dotted;
            BorderColor = "#ffe8cd";
        }
    }
}