// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A control to visualize a test run's progress.
    /// </summary>
    public partial class TestRunProgress : HtmlDiv
    {
        /// <summary>
        /// The percent of the run that is complete.
        /// </summary>
        private double _percent;

        /// <summary>
        /// The progress bar.
        /// </summary>
        private HtmlDiv _progress;

        /// <summary>
        /// Initializes a new progress bar control.
        /// </summary>
        public TestRunProgress() : base()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Gets or sets the percent complete on a scale from 0 to 100.
        /// </summary>
        public double PercentComplete
        {
            set
            {
                _percent = value;
                UpdateProgressBar();
            }

            get
            {
                return _percent;
            }
        }

        /// <summary>
        /// Sets the color of the progress bar.
        /// </summary>
        /// <param name="value">The color value.</param>
        public void SetProgressColor(string value)
        {
            _progress.BackgroundColor = value;
        }

        /// <summary>
        /// Updates the progress bar.
        /// </summary>
        private void UpdateProgressBar()
        {
            double width = (double)GetProperty(HtmlProperty.ClientWidth);
            if (width <= 0)
            {
                return;
            }
            int round = (int) Math.Round(_percent * (width / 100));
            _progress.Width = round;
        }

        /// <summary>
        /// Initializes the web control.
        /// </summary>
        private void InitializeComponent()
        {
            SetStyleAttribute(CssAttribute.Position, "relative");
            _progress = new HtmlDiv();
            _progress.SetStyleAttribute(CssAttribute.Position, "absolute");
            _progress.Position.Left = _progress.Position.Top = _progress.Position.Bottom = 0;
            SetProgressColor(Color.Black);
            MakeTransparent(_progress, 25);
            Controls.Add(_progress);
        }

        /// <summary>
        /// Makes a control transparent.  Works with several browsers.
        /// </summary>
        /// <param name="control">The control.</param>
        /// <param name="opacity">The opacity amount.</param>
        private static void MakeTransparent(HtmlControl control, int opacity)
        {
            control.SetStyleAttribute("filter", "alpha(opacity=" + opacity + ")");
            control.SetStyleAttribute("-moz-opacity", "." + opacity);
            control.SetStyleAttribute("opacity", "." + opacity);
        }
    }
}