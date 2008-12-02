// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// An interactive web control for examining a test method's results and 
    /// detailed information.
    /// </summary>
    public partial class ResultInspector : HtmlDiv
    {
        /// <summary>
        /// Gets the scenario result object.
        /// </summary>
        public ScenarioResult Result { get; private set; }

        /// <summary>
        /// Details control.
        /// </summary>
        private Details _details;

        /// <summary>
        /// Toggles the details display view.
        /// </summary>
        public void ToggleDropDown()
        {
            _details.ToggleVisibility();
        }

        /// <summary>
        /// Initializes a new ResultInspector control.
        /// </summary>
        /// <param name="result">The scenario result.</param>
        public ResultInspector(ScenarioResult result)
            : base()
        {
            Result = result;
            InitializeComponent();
        }

        /// <summary>
        /// Prepare the initial visual contents of the web control.
        /// </summary>
        private void InitializeComponent()
        {
            _details = Result.Exception != null && Result.Result != TestOutcome.Passed ? new ExceptionDetails(this) : new Details(this);
            bool hasDecorators = false;
            foreach (HtmlControlBase decoration in _details.DecorateContainer())
            {
                Controls.Add(decoration);
                hasDecorators = true;
            }
            Controls.Add(_details);
            if (hasDecorators)
            {
                BackgroundColor = Color.Manila;
                SetStyleAttribute(CssAttribute.Border, "1px solid " + Color.ManilaBorder);
                ForegroundColor = Color.DarkGray;
                Padding.Bottom = 2;
                Margin.Bottom = 2;
                Padding.Top = 6;
                SetStyleAttribute(CssAttribute.Position, "relative");
            }
            SetStyleAttribute(CssAttribute.Position, "relative");
        }
    }
}