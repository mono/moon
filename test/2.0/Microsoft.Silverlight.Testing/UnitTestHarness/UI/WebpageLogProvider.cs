// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// A log provider that operates on the live HtmlPage by logging test 
    /// harness information to the screen.
    /// </summary>
    public class WebpageLogProvider : WebpageLogProviderBase
    {
        /// <summary>
        /// Constant, the right angle quote in HTML.
        /// </summary>
        private const string RightAngleQuote = "&raquo;";

        /// <summary>
        /// Div used for the log of this instance.
        /// </summary>
        private HtmlDiv _myLog;

        /// <summary>
        /// Instantiate a new WebpageLogProvider class.
        /// </summary>
        public WebpageLogProvider()
        {
        }

        /// <summary>
        /// Initializes the test harness.
        /// </summary>
        /// <param name="settings">The test harness settings object.</param>
        public override void Initialize(TestHarnessSettings settings)
        {
            base.Initialize(settings);

            RegisterLogHandlers();

            // Initializes the static provider control if it does not yet exist
            WebBrowserTestPage page = WebpageLogProviderBase.TestPage;

            // Simple div for logging
            _myLog = new HtmlDiv();
            _myLog.Padding.All = 0;
            page.TestColumn.Controls.Add(_myLog);
        }

        /// <summary>
        /// Gets a reference to the HtmlElement that contains the running log 
        /// for this instance.
        /// </summary>
        protected HtmlDiv LogContainer
        {
            get
            {
                return _myLog;
            }
        }

        /// <summary>
        /// Register the log handlers for this provider.
        /// </summary>
        private void RegisterLogHandlers()
        {
            RegisterGranularHandler();
            RegisterInformationHandler();
            RegisterWarningHandler();
        }

        /// <summary>
        /// Register an informational message handler.
        /// </summary>
        protected void RegisterInformationHandler()
        {
            RegisterMessageTypeHandler(LogMessageType.Information, ProcessInformation);
        }

        /// <summary>
        /// Register a warning handler.
        /// </summary>
        protected void RegisterWarningHandler()
        {
            RegisterMessageTypeHandler(LogMessageType.Warning, ProcessWarning);
        }

        /// <summary>
        /// Register a granular handler.
        /// </summary>
        protected void RegisterGranularHandler()
        {
            RegisterConditionalHandler(
                IsGranularMessage, 
                delegate(LogMessage message)
            {
                ProcessGranularEvent(message);
            });
        }

        /// <summary>
        /// Conditional check for the granularity decorator.
        /// </summary>
        /// <param name="message">The log message.</param>
        /// <returns>Returns true if the condition is met.</returns>
        private bool IsGranularMessage(LogMessage message)
        {
            return (message.HasDecorator(LogDecorator.TestGranularity));
        }

        /// <summary>
        /// How to process any remaining messages.
        /// </summary>
        /// <param name="message">The log message.</param>
        protected override void ProcessRemainder(LogMessage message)
        {
            AppendFixedText(message.ToString());
        }

        /// <summary>
        /// Appends text to the log, in a fixed-width font.
        /// </summary>
        /// <param name="value">The string to append.</param>
        protected void AppendFixedText(string value)
        {
            HtmlSpan span = new HtmlSpan();
            WebBrowserTestPage.SetDefaultFixedFont(span);
            WebBrowserTestPage.SetDefaultFontSize(span);
            span.ForegroundColor = Color.LightGray;
            span.InnerHtml = value;

            _myLog.Controls.Add(span);
            _myLog.Controls.Add(new HtmlLineBreak());
        }

        /// <summary>
        /// Display the information associated with a warning message.
        /// </summary>
        /// <remarks>This method does not do anything with an associated 
        /// Exception object stored in the message.</remarks>
        /// <param name="logMessage">Message to process.</param>
        protected virtual void ProcessWarning(LogMessage logMessage)
        {
            AddSimpleMessage(RightAngleQuote + " " + logMessage.Message, Color.Black, Color.Yellow);
        }

        /// <summary>
        /// Display simple information in the log.
        /// </summary>
        /// <param name="logMessage">Message to process.</param>
        protected virtual void ProcessInformation(LogMessage logMessage)
        {
            AddSimpleMessage(RightAngleQuote + " " + logMessage.Message, Color.DarkGray);
        }

        /// <summary>
        /// Appends simple text to the log.
        /// </summary>
        /// <param name="message">The message, HTML permitted.</param>
        protected void AddSimpleMessage(string message)
        {
            AddSimpleMessage(message, null, null);
        }

        /// <summary>
        /// Appends simple text to the log.
        /// </summary>
        /// <param name="message">The message, HTML permitted.</param>
        /// <param name="foregroundColor">The optional foreground color, as a 
        /// string value.</param>
        protected void AddSimpleMessage(string message, string foregroundColor)
        {
            AddSimpleMessage(message, foregroundColor, null);
        }

        /// <summary>
        /// Appends simple text to the log.
        /// </summary>
        /// <param name="message">The message, HTML permitted.</param>
        /// <param name="foregroundColor">The optional foreground color, as a 
        /// string value.</param>
        /// <param name="backgroundColor">The optional background color, as a 
        /// string value.</param>
        protected void AddSimpleMessage(string message, string foregroundColor, string backgroundColor)
        {
            HtmlDiv div = new HtmlDiv();
            div.InnerHtml = message;
            div.Padding.All = 3;
            if (backgroundColor != null)
            {
                div.BackgroundColor = backgroundColor;
            }
            if (foregroundColor != null)
            {
                div.ForegroundColor = foregroundColor;
            }
            _myLog.Controls.Add(div);
        }

        /// <summary>
        /// Process an event with granular properties.
        /// </summary>
        /// <param name="l">The log message object.</param>
        private void ProcessGranularEvent(LogMessage l)
        {
            TestGranularity granularity = (TestGranularity)l[LogDecorator.TestGranularity];
            HtmlDiv div = new HtmlDiv();
            div.InnerHtml = l.Message;
            div.Padding.All = 2;

            switch (granularity)
            {
                case TestGranularity.Harness:
                    div.BackgroundColor = Color.DarkGray;
                    div.ForegroundColor = Color.VeryLightGray;
                    break;

                case TestGranularity.TestScenario:
                case TestGranularity.Test:
                    break;

                case TestGranularity.TestGroup:
                    div.Font.Bold = true;
                    break;

                default:
                    div = null;
                    AppendFixedText(l.ToString());
                    break;
            }
            if (div != null)
            {
                _myLog.Controls.Add(div);
            }
        }
    }
}