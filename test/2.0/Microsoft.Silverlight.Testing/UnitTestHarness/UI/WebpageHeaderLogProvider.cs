// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// A log provider that operates on the live HtmlPage by logging very 
    /// high-level pass/fail status to the screen.
    /// </summary>
    public class WebpageHeaderLogProvider : WebpageLogProviderBase
    {
        /// <summary>
        /// Name to display in the header for the test harness or test product 
        /// name.
        /// </summary>
        private const string DefaultDisplayName = "Microsoft.Silverlight.Testing Framework";

        /// <summary>
        /// Gets or sets the display name.
        /// </summary>
        private string DisplayName { get; set; }

        /// <summary>
        /// The test column header control.
        /// </summary>
        private TestColumnHeader _control;

        /// <summary>
        /// Whether any failures have been found.
        /// </summary>
        private bool _hasFailed;

        /// <summary>
        /// Whether we've had any results at all.
        /// </summary>
        private bool _hasResults;

        /// <summary>
        /// Has a granular TestGroup completion been processed.
        /// </summary>
        private bool _hasFinished;

        /// <summary>
        /// Creates a new web page header provider.
        /// </summary>
        public WebpageHeaderLogProvider() : this(DefaultDisplayName) { }

        /// <summary>
        /// Creates a new web page header provider.
        /// </summary>
        /// <param name="displayName">The display name for the harness.</param>
        public WebpageHeaderLogProvider(string displayName) : base()
        {
            DisplayName = displayName;
        }

        /// <summary>
        /// Initializes the test harness.
        /// </summary>
        /// <param name="settings">The test harness settings object.</param>
        public override void Initialize(TestHarnessSettings settings)
        {
            base.Initialize(settings);

            RegisterLoggingHandlers();

            // Initializes the static provider control if it does not yet exist
            WebBrowserTestPage page = WebpageLogProviderBase.TestPage;

            // Creates and appends the header control
            _control = new TestColumnHeader(DisplayName);
            page.TestColumn.Controls.Add(_control);
        }

        /// <summary>
        /// Registers the conditional delegates.
        /// </summary>
        private void RegisterLoggingHandlers()
        {
            // Check for and result to results
            RegisterConditionalHandler(
                NotFailedButWithResults, 
                delegate(LogMessage message)
            {
                TestOutcome outcome = (TestOutcome)message[LogDecorator.TestOutcome];
                _hasResults = true;
                if (outcome != TestOutcome.Passed)
                {
                    _hasFailed = true;
                    UpdateIndicator();
                }
                else if (_hasFailed == false)
                {
                    UpdateIndicator();
                }
            });

            // Record the completion of the test group

            // NOTE: This could be done at the harness level, but logs may not 
            // be processed by the harness provider post-completion, so a single
            // test group at least improves the experience some.
            RegisterConditionalHandler(
                MarksTestGroupCompletion, 
                delegate(LogMessage message)
            {
                _hasFinished = true;
                UpdateIndicator();
            });
        }

        /// <summary>
        /// Conditional check, if the message has results and the current state 
        /// of the run, as tracked by this provider, is "pass".
        /// </summary>
        /// <param name="message">The log message.</param>
        /// <returns>Returns true when the conditions are met.</returns>
        private bool NotFailedButWithResults(LogMessage message)
        {
            // A. Must be in a passing state
            // B. Must have results
            return (!_hasFailed && message.HasDecorator(LogDecorator.TestOutcome));
        }

        /// <summary>
        /// Conditional check, if the message is of a granular nature, a test 
        /// group, and in the finishing stage.
        /// </summary>
        /// <param name="message">The log message.</param>
        /// <returns>Returns true when the conditions are met.</returns>
        private bool MarksTestGroupCompletion(LogMessage message)
        {
            if (message.HasDecorators(LogDecorator.TestGranularity, LogDecorator.TestStage))
            {
                TestGranularity granularity = (TestGranularity) message[LogDecorator.TestGranularity];
                TestStage stage = (TestStage) message[LogDecorator.TestStage];

                bool ok = granularity == TestGranularity.TestGroup || granularity == TestGranularity.Harness;
                return ok && stage == TestStage.Finishing;
            }
            return false;
        }

        /// <summary>
        /// Alter the coloring to indicate the status.
        /// </summary>
        private void UpdateIndicator()
        {
            if (_hasResults)
            {
                _control.UpdateIndicatorColoring(!_hasFinished, ! _hasFailed);
            }
        }
    }
}