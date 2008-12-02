// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// Failure information.
    /// </summary>
    public class FailureControl : HtmlDiv
    {
        /// <summary>
        /// The prefix identifier.
        /// </summary>
        private const string PrefixIdentifier = "_utf_f_";

        /// <summary>
        /// The failure summary control.  Also the Parent, but this is 
        /// strongly typed.
        /// </summary>
        private FailureSummaryControl _summary;

        /// <summary>
        /// Initializes a new failure web control.
        /// </summary>
        /// <param name="testClass">The test class metadata object.</param>
        /// <param name="summaryControl">The overall summary control.</param>
        public FailureControl(ITestClass testClass, FailureSummaryControl summaryControl) : base()
        {
            _summary = summaryControl;

            Margin.All = 0;
            Margin.Top = 4;
            Margin.Bottom = 2;

            CreateClassHeader(testClass.Name);
        }

        /// <summary>
        /// Creates the visual header for the class.
        /// </summary>
        /// <param name="name">The test class name.</param>
        private void CreateClassHeader(string name)
        {
            Paragraph para = new Paragraph(/* html */ name);
            para.Font.Bold = true;
            para.BackgroundColor = Color.Manila;
            para.Margin.All = 0;
            para.Padding.All = 2;
            para.BorderColor = Color.ManilaBorder;
            para.BorderStyle = BorderStyle.Solid;
            para.BorderWidthAdvanced.All = 1;
            para.BorderWidthAdvanced.Bottom = 2;
            para.SetStyleAttribute(CssAttribute.BorderBottomColor, Color.Tan);

            Controls.Add(para);
        }

        /// <summary>
        /// Styles the anchors used for this control.
        /// </summary>
        /// <param name="anchor">The anchor control.</param>
        private static void StyleAnchor(HtmlAnchor anchor)
        {
            anchor.ForegroundColor = Color.DarkGray;
        }

        /// <summary>
        /// Records a failure and modifies the web page to allow linking between
        /// the failure summary, and also next and previous failure 
        /// functionality.
        /// </summary>
        /// <param name="method">Unit test provider's method reference.</param>
        /// <param name="methodNextSpan">Span that will contain the next button.</param>
        public void AddMethodFailure(ITestMethod method, HtmlControl methodNextSpan)
        {
            string id = PrefixIdentifier + _summary.GenerateNextFailureId().ToString(CultureInfo.InvariantCulture);

            HtmlAnchor anchor = new HtmlAnchor(id);
            StyleAnchor(anchor);
            anchor.Id = id;
            methodNextSpan.Controls.Add(anchor);

            // Link to the previous failure
            //---
            if (_summary.FirstFailureEntry != null)
            {
                HtmlAnchor prev = new HtmlAnchor();
                StyleAnchor(prev);
                prev.Href = "#" + _summary.LastFailureEntry.Id;
                prev.Title = "Previous Failure";
                prev.Font.Underline = false;
                prev.ForegroundColor = Color.White;
                prev.InnerHtml = "&uarr;";

                methodNextSpan.Controls.Add(prev);
            }

            // Link to the next failure
            //---
            if (_summary.LastFailureEntry != null)
            {
                HtmlAnchor next = new HtmlAnchor();
                StyleAnchor(next);
                next.Href = "#" + id;
                next.Title = "Next Failure";
                next.Font.Underline = false;
                next.ForegroundColor = Color.White;
                next.InnerHtml = "&nbsp; &darr;";
                
                _summary.LastFailureEntry.Element.Controls.Add(next);
            }

            // The failure summary link
            //---
            HtmlAnchor link = new HtmlAnchor();
            StyleAnchor(link);
            link.Href = "#" + id;
            link.Display = CssDisplay.Block;
            link.InnerHtml = method.Name;
            Controls.Add(link);

            // Store this most recent failure's data
            //---
            _summary.LastFailureEntry = new FailureEntry
            {
                Id = id,
                Element = methodNextSpan
            };

            // Set the first entry if this is the first
            //---
            if (_summary.FirstFailureEntry == null)
            {
                _summary.FirstFailureEntry = _summary.LastFailureEntry;
            }
        }
    }
}