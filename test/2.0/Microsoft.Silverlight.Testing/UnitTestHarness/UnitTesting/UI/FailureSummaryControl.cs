// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A web page control that provides a summary, and links to, any failing 
    /// tests during a test run.
    /// </summary>
    public class FailureSummaryControl : HtmlDiv
    {
        /// <summary>
        /// Trying to keep the initial dictionary size tiny.
        /// </summary>
        private const int StartingCollectionSize = 2;

        /// <summary>
        /// Collection of seen classes.
        /// </summary>
        private Dictionary<ITestClass, FailureControl> _classes;

        /// <summary>
        /// The first element to be added.
        /// </summary>
        private FailureEntry _first;

        /// <summary>
        /// The last element to be added.
        /// </summary>
        private FailureEntry _last;

        /// <summary>
        /// The last ID to be created.
        /// </summary>
        private int _lastId;

        /// <summary>
        /// Create a new FailureSummary web page control.
        /// </summary>
        public FailureSummaryControl() : base()
        {
            _classes = new Dictionary<ITestClass, FailureControl>(StartingCollectionSize);

            Margin.All = 0;
            Padding.All = 0;
        }

        /// <summary>
        /// The first failure calls this.
        /// </summary>
        private void FirstFailure()
        {
            HtmlDiv summaryHeader = new HtmlDiv();
            summaryHeader.InnerHtml = "Summary of Test Run Failures";
            summaryHeader.Padding.All = 2;
            summaryHeader.BackgroundColor = Color.DarkGray;
            summaryHeader.ForegroundColor = Color.VeryLightGray;
            Controls.Add(summaryHeader);
        }

        /// <summary>
        /// Returns the next generated failure ID.  Used to iterate through the 
        /// set of failures by anchors and links.
        /// </summary>
        /// <returns>Returns the integer ID.</returns>
        public int GenerateNextFailureId()
        {
            _lastId++;
            return _lastId;
        }

        /// <summary>
        /// Gets or sets the first failure entry.
        /// </summary>
        public FailureEntry FirstFailureEntry
        {
            get { return _first; }
            set { _first = value; }
        }

        /// <summary>
        /// Gets or sets the last failure entry.
        /// </summary>
        public FailureEntry LastFailureEntry
        {
            get { return _last; }
            set { _last = value; }
        }

        /// <summary>
        /// Adds a failure visually.
        /// </summary>
        /// <param name="testClass">Test class metadata object.</param>
        /// <param name="method">Method metadata object.</param>
        /// <param name="methodNextSpan">The next method to work with.</param>
        public void AddFailure(
            ITestClass testClass,
            ITestMethod method,
            HtmlControl methodNextSpan)
        {
            if (_lastId == 0)
            {
                FirstFailure();
            }

            if (false == _classes.ContainsKey(testClass))
            {
                _classes[testClass] = new FailureControl(testClass, this);
                Controls.Add(_classes[testClass]);
            }

            _classes[testClass].AddMethodFailure(method, methodNextSpan);
        }
    }
}