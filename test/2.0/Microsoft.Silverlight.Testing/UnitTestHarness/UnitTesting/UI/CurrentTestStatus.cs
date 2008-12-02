// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// Webpage control that tracks the counts for the total scenarios run, 
    /// failing scenarios, and also provides information about currently 
    /// executing work items.
    /// </summary>
    public class CurrentTestStatus : HtmlDiv
    {
        /// <summary>
        /// Element that is the total counter.
        /// </summary>
        private HtmlDiv _total;

        /// <summary>
        /// Element that is the failure counter.
        /// </summary>
        private HtmlDiv _fail;

        /// <summary>
        /// Primary status element.
        /// </summary>
        private HtmlSpan _status;

        /// <summary>
        /// Secondary status element.
        /// </summary>
        private HtmlSpan _details;

        /// <summary>
        /// Number of scenarios run.
        /// </summary>
        private int _totalNumber;

        /// <summary>
        /// Number of failing scenarios.
        /// </summary>
        private int _failNumber;

        /// <summary>
        /// Creates a new test status control on the webpage, within the 
        /// parentElement.
        /// </summary>
        public CurrentTestStatus()
        {
            SetStyleAttribute(CssAttribute.Position, "relative");
            Height = 40;
            SetStyleAttribute(CssAttribute.MarginBottom, new Unit(6));
            BorderColor = Color.VeryLightGray;
            BorderStyle = BorderStyle.Solid;
            BorderWidth = 1;

            _status = new HtmlSpan();
            _status.Margin.All = 0;
            _status.Margin.Left = 4;
            _details = new HtmlSpan();
            _details.Margin.All = 0;
            _details.Margin.Left = 4;
            RevertFinalStyle();

            _total = CreateCounterDiv();
            _fail = CreateCounterDiv();
            _fail.SetStyleAttribute(CssAttribute.Right, new Unit(35, UnitType.Pixel));

            Controls.Add(_status);
            Controls.Add(new HtmlLineBreak());
            Controls.Add(_details);
            Controls.Add(_total);
            Controls.Add(_fail);
        }

        /// <summary>
        /// Update the status text indicating the current work item being 
        /// executed.
        /// </summary>
        /// <param name="text">Text to place in the status area.</param>
        public void UpdateStatus(string text)
        {
            _status.InnerHtml = text;
        }

        /// <summary>
        /// Update the secondary status text.
        /// </summary>
        /// <param name="detailsText">Text to set in the detailed status area.</param>
        public void UpdateDetails(string detailsText)
        {
            _details.InnerHtml = detailsText;
        }

        /// <summary>
        /// Reverts the styling from the test complete look.
        /// </summary>
        public void RevertFinalStyle()
        {
            UpdateStatus(String.Empty);
            _status.Font.Size = new FontUnit(14, UnitType.Pixel);
            _details.Font.Size = new FontUnit(12, UnitType.Pixel);
            if (_fail != null)
            {
                _fail.Visible = true;
            }
        }

        /// <summary>
        /// Alter the styling to indicate that the test run is complete.
        /// </summary>
        public void ChangeToFinalStyle()
        {
            UpdateStatus("finished");
            _status.Font.Bold = true;
            _fail.BorderColor = Color.White;
            _status.Font.Size = FontUnit.XLarge;
            string title = _failNumber == 0 ? "Successful Test Run" : String.Format(CultureInfo.InvariantCulture, "{0} failures", _failNumber);
            if (_failNumber == 0)
            {
                _fail.Visible = false;
            }

            HtmlPage.Document.SetProperty("title", title);
        }

        /// <summary>
        /// Creates a div that will contain a number counter.
        /// </summary>
        /// <returns>The HTML element of the counter.</returns>
        private static HtmlDiv CreateCounterDiv()
        {
            HtmlDiv elem = new HtmlDiv();
            elem.SetStyleAttribute(CssAttribute.Position, "absolute");
            elem.SetStyleAttribute(CssAttribute.Display, CssDisplay.Block);
            elem.Margin.Top = new Unit(4, UnitType.Pixel);
            elem.Position.Right = 1;
            elem.Position.Top = 1;
            elem.Padding.Top = 3;
            elem.Font.Size = new FontUnit(14, UnitType.Pixel);
            elem.SetStyleAttribute(CssAttribute.TextAlign, "center");
            elem.BorderColor = Color.White;
            elem.BorderStyle = BorderStyle.Solid;
            elem.BorderWidth = 1;
            elem.Width = 29;
            elem.Height = 23;
            elem.InnerHtml = String.Empty;
            return elem;
        }

        /// <summary>
        /// Gets or sets the number of scenarios run.
        /// </summary>
        public int Total
        {
            get { return _totalNumber; }

            set 
            {
                _totalNumber = value;
                _total.InnerHtml = value.ToString(CultureInfo.InvariantCulture);
                _fail.BorderColor = Color.White;
            }
        }

        /// <summary>
        /// Gets or sets the number of scenarios that have failed.
        /// </summary>
        public int Fail
        {
            get { return _failNumber; }

            set
            {
                _failNumber = value;

                _fail.InnerHtml = value.ToString(CultureInfo.InvariantCulture);
                if (_totalNumber > 0)
                {
                    _fail.ForegroundColor = Color.Red;
                    _fail.BorderColor = Color.Red;
                }
            }
        }
    }
}