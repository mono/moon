// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A web page log provider that is specialized for the unit test framework.
    /// </summary>
    public class UnitTestWebpageLog : WebpageLogProvider
    {
        /// <summary>
        /// String ellipsis.
        /// </summary>
        private const string Ellipsis = "...";

        /// <summary>
        /// The maximum number of characters to display in a name without using 
        /// an ellipsis.
        /// </summary>
        private const int MaximumNameDisplayLength = 52;

        /// <summary>
        /// The associated start element for a test method.
        /// </summary>
        private IDictionary<ITestMethod, HtmlContainerControl> _startElements;

        /// <summary>
        /// Html control that shows the current test method name and other 
        /// information specific to unit testing.
        /// </summary>
        private CurrentTestStatus _statusDisplay;

        /// <summary>
        /// A web page control that displays a summary and links to failures.
        /// </summary>
        private FailureSummaryControl _failureSummary;

        /// <summary>
        /// The number of scenarios encountered.
        /// </summary>
        private int _scenarioCounter;

        /// <summary>
        /// A value indicating whether the copy results link has been added 
        /// to the page yet.
        /// </summary>
        private bool _copyResultsVisible;

        /// <summary>
        /// The most recent harness object.
        /// </summary>
        private HtmlContainerControl _mostRecentHarness;

        /// <summary>
        /// The test run's progress bar.
        /// </summary>
        private TestRunProgress _progressBar;

        /// <summary>
        /// Display progress information and/or elapsed time.
        /// </summary>
        private HtmlDiv _progressText;

        /// <summary>
        /// The unit test harness.
        /// </summary>
        private UnitTestHarness _unitTestHarness;

        /// <summary>
        /// When the test assembly starts.
        /// </summary>
        private DateTime _start;

        /// <summary>
        /// When the test assembly finishes.
        /// </summary>
        private DateTime _end;

        /// <summary>
        /// Creates a new provider for displaying results on the web page, with 
        /// specialized knowledge of the unit test system.
        /// </summary>
        public UnitTestWebpageLog() 
            : base() 
        {
        }

        /// <summary>
        /// Initializes the test harness.
        /// </summary>
        /// <param name="settings">The test harness settings object.</param>
        public override void Initialize(TestHarnessSettings settings)
        {
            base.Initialize(settings);

            InitializeComponent();
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            _startElements = new Dictionary<ITestMethod, HtmlContainerControl>();
            _statusDisplay = new CurrentTestStatus();
            _statusDisplay.Fail = _statusDisplay.Total = 0;
            _failureSummary = new FailureSummaryControl();
            _progressText = new HtmlDiv();
            _progressText.Margin.Bottom = 4;

            LogContainer.Controls.Add(_statusDisplay);
            PlaceProgressBar();
            LogContainer.Controls.Add(_progressText);
            LogContainer.Controls.Add(_failureSummary);

            RegisterLogHandlers();
        }

        /// <summary>
        /// Places the progress bar.  If we can attach inside the existing 
        /// header, then insert inside that - otherwise, the progress bar 
        /// is just inserted inside the test column.
        /// </summary>
        private void PlaceProgressBar()
        {
            _progressBar = new TestRunProgress();
            _progressBar.Height = 12;

            HtmlElement hookupElement = null;
            ScriptObjectCollection hcc = HtmlPage.Document.GetElementsByTagName("div");
            for (int i = 0; i < hcc.Count; ++i)
            {
                HtmlElement htmlElement = hcc[i] as HtmlElement;
                if (htmlElement != null)
                {
                    if (htmlElement.CssClass == TestColumnHeader.TestColumnIndicator)
                    {
                        hookupElement = htmlElement;
                        break;
                    }
                }
            }
            if (hookupElement == null)
            {
                LogContainer.Controls.Add(_progressBar);
            }
            else
            {
                hookupElement.AppendChild(_progressBar);
            }
        }

        /// <summary>
        /// Register the handling conditions for log messages in the unit test 
        /// web page's log provider.
        /// </summary>
        private void RegisterLogHandlers()
        {
            ClearConditionalHandlers();
            ClearMessageTypeHandlers();

            // From the parent
            RegisterWarningHandler();
            RegisterInformationHandler();

            // Our specific handlers
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestEndMessage, ProcessEndMessage);
            RegisterConditionalHandler(UnitTestMessageConditional.IsExceptionLogMessage, ProcessException);
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestStartMessage(UnitTestLogDecorator.UnitTestHarness), UpdateAndStart(UnitTestHarnessStart));
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestStartMessage(UnitTestLogDecorator.TestAssemblyMetadata), UpdateAndStart(TestAssemblyStart));
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestStartMessage(UnitTestLogDecorator.TestClassMetadata), UpdateAndStart(TestClassStart));
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestStartMessage(UnitTestLogDecorator.TestMethodMetadata), UpdateAndStart(TestMethodStart));
    
            RegisterConditionalHandler(UnitTestMessageConditional.HasUnitTestOutcome, ProcessResult);
            RegisterConditionalHandler(UnitTestMessageConditional.IsIgnoreMessage, ProcessIgnoreMessage);
            RegisterConditionalHandler(UnitTestMessageConditional.IsIncorrectExceptionLogMessage, ProcessIncorrectException);
            RegisterConditionalHandler(UnitTestMessageConditional.IsKnownBug, ProcessBug);
            RegisterMessageTypeHandler(
                LogMessageType.TestInfrastructure, 
                delegate(LogMessage message)
                {
                    // Ignore any infrastructure messages
                });
        }

        /// <summary>
        /// Process the log of an exception.
        /// </summary>
        /// <param name="logMessage">The log message.</param>
        protected virtual void ProcessException(LogMessage logMessage)
        {
        }

        /// <summary>
        /// Process the end message.
        /// </summary>
        /// <param name="logMessage">The log message object.</param>
        private void ProcessEndMessage(LogMessage logMessage)
        {
            //// TestStage stage = (TestStage)logMessage[LogDecorator.TestStage];
            if (!logMessage.HasDecorator(LogDecorator.TestGranularity))
            {
                return;
            }

            TestGranularity gran = (TestGranularity) logMessage[LogDecorator.TestGranularity];
            if (gran == TestGranularity.TestGroup)
            {
                CompleteTestGroup();
            }
        }

        /// <summary>
        /// Method to show the closure of a test group at runtime.
        /// </summary>
        private void CompleteTestGroup()
        {
            _end = DateTime.Now;
            _statusDisplay.UpdateDetails(String.Empty);
            _progressBar.PercentComplete = 0;

            if (_scenarioCounter > 0)
            {
                TimeSpan diff = new TimeSpan(_end.Ticks - _start.Ticks);
                int average = (int)diff.TotalMilliseconds / _scenarioCounter;
                TimeSpan tsd = new TimeSpan(0, 0, 0, 0, average);
                string averagePerTest = ElapsedReadableTime(tsd);
                _progressText.InnerHtml = String.Format(CultureInfo.CurrentCulture, "Running {0} " + "test".Plural(_scenarioCounter) + " took " + ElapsedReadableTime(_start, _end) + "<br />({1} per test method on average)", _scenarioCounter, averagePerTest);
            }
            _scenarioCounter = 0;

            HtmlContainerControl done = new HtmlDiv();
            HtmlAnchor close = new HtmlAnchor(
                "(Close)",
                delegate(object sender, HtmlEventArgs e)
                {
                    HtmlPage.Window.Invoke("close");
                });
            close.ForegroundColor = Color.LightGray;

            done.InnerHtml = "Test run complete ";
            done.Controls.Add(close);

            if (!_copyResultsVisible && _statusDisplay.Fail > 0)
            {
                _copyResultsVisible = true;

                // Attempt to see if the provider is setup
                foreach (LogProvider lp in _unitTestHarness.LogProviders)
                {
                    TextFailuresLogProvider txt = lp as TextFailuresLogProvider;
                    if (txt != null)
                    {
                        HtmlAnchor copy = new HtmlAnchor("(Copy all failing results)", (sender, args) => HtmlPage.Window.Alert(txt.GetFailuresLog()));
                        copy.Margin.Left = 8;
                        copy.ForegroundColor = Color.LightGray;
                        done.Controls.Add(copy);
                        break;
                    }
                }
            }

            done.Padding.All = 3;
            done.BackgroundColor = Color.DarkGray;
            done.ForegroundColor = Color.VeryLightGray;
            done.SetStyleAttribute(CssAttribute.MarginTop, 4);

            _statusDisplay.ChangeToFinalStyle();
            LogContainer.Controls.Add(done);
        }

        /// <summary>
        /// Returns a human-readable formatting of the time different between 
        /// two DateTime instances.
        /// </summary>
        /// <param name="start">The starting time.</param>
        /// <param name="finish">The finishing time.</param>
        /// <returns>Returns a human-readable string.</returns>
        public static string ElapsedReadableTime(DateTime start, DateTime finish)
        {
            TimeSpan ts = new TimeSpan(finish.Ticks - start.Ticks);
            return ElapsedReadableTime(ts);
        }

        /// <summary>
        /// Returns a human-readable formatting of the time different between 
        /// two DateTime instances.
        /// </summary>
        /// <param name="ts">The time span instance.</param>
        /// <returns>Returns a human-readable string.</returns>
        public static string ElapsedReadableTime(TimeSpan ts)
        {
            List<string> parts = new List<string>();

            if (ts.Milliseconds > 0 && ts.Days == 0 && ts.Hours == 0 && ts.Minutes == 0)
            {
                parts.Add(ts.Milliseconds.ToString(CultureInfo.CurrentCulture) + " ms");
            }
            if (ts.Seconds > 0 && ts.Days == 0 && ts.Hours == 0)
            {
                parts.Add(ts.Seconds.ToString(CultureInfo.CurrentCulture) + " second".Plural(ts.Seconds));
            }
            if (ts.Minutes > 0)
            {
                parts.Add(ts.Minutes.ToString(CultureInfo.CurrentCulture) + " minute".Plural(ts.Minutes));
            }
            if (ts.Hours > 0)
            {
                parts.Add(ts.Hours.ToString(CultureInfo.CurrentCulture) + " hour".Plural(ts.Hours));
            }
            if (ts.Days > 0)
            {
                parts.Add(ts.Days.ToString(CultureInfo.CurrentCulture) + " day".Plural(ts.Days));
            }
            parts.Reverse();
            return String.Join(" ", parts.ToArray());
        }

        /// <summary>
        /// Assembly start code.
        /// </summary>
        /// <param name="l">The log message.</param>
        private void TestAssemblyStart(LogMessage l)
        {
            _statusDisplay.RevertFinalStyle();
            _start = DateTime.Now;

            HtmlPage.Document.SetProperty("title", String.Format(CultureInfo.InvariantCulture, "{0} - Test Run", l[LogDecorator.NameProperty]));
            if (_mostRecentHarness != null)
            {
                _mostRecentHarness.InnerHtml = "&nbsp;<strong>" + l[LogDecorator.NameProperty] + "</strong>";
            }
        }

        /// <summary>
        /// Test method start code.
        /// </summary>
        /// <param name="l">The log message.</param>
        private void TestMethodStart(LogMessage l)
        {
            ITestMethod method = (ITestMethod)l[UnitTestLogDecorator.TestMethodMetadata];
            string name = TruncateNameIfNeeded(l.Message);
            string details = string.IsNullOrEmpty(method.Description) ? l.Message : l.Message + ": " + method.Description;

            HtmlContainerControl elem = CreateText();
            HtmlAnchor anchor = new HtmlAnchor();
            anchor.InnerHtml = name;
            anchor.Title = details;
            anchor.Font.Underline = false;
            anchor.SetStyleAttribute(CssAttribute.BorderBottom, "1px dotted #aaa");
            elem.Controls.Add(anchor);
            // elem.BackgroundColor = Color.White;
            _startElements[method] = elem;

            LogContainer.Controls.Add(elem);
        }

        /// <summary>
        /// Returns either the full string, or a truncated version with an 
        /// ellipsis, if needed.
        /// </summary>
        /// <param name="name">The name string.</param>
        /// <returns>Returns the updated string.</returns>
        private static string TruncateNameIfNeeded(string name)
        {
            return name.Length > MaximumNameDisplayLength ? name.Substring(0, MaximumNameDisplayLength) + Ellipsis : name;
        }

        /// <summary>
        /// Test class start code.
        /// </summary>
        /// <param name="l">Log message.</param>
        private void TestClassStart(LogMessage l)
        {
            //// ITestClass tclass = (ITestClass)l[UnitTestLogDecorator.TestClassMetadata];
            HtmlContainerControl e = CreateText();
            e.InnerHtml = "<em>" + l.Message + "</em>";
            e.BackgroundColor = Color.Manila;
            e.SetStyleAttribute(CssAttribute.Border, "1px solid " + Color.ManilaBorder);
            e.Font.Bold = true;
            e.ForegroundColor = Color.DarkGray;
            e.SetStyleAttribute("borderBottom", "2px solid " + Color.Tan);
            e.Margin.Top = 3;
            LogContainer.Controls.Add(e);

            _statusDisplay.UpdateDetails(TruncateNameIfNeeded(l.Message));
        }

        /// <summary>
        /// Unit test harness starting message.
        /// </summary>
        /// <param name="l">Log message object.</param>
        private void UnitTestHarnessStart(LogMessage l)
        {
            _unitTestHarness = (UnitTestHarness) l[UnitTestLogDecorator.UnitTestHarness];
            string displayName = "Unit Test Run for ";
            HtmlContainerControl harness = new HtmlDiv();
            harness.InnerHtml = displayName;
            harness.Padding.All = 3;
            harness.BackgroundColor = Color.DarkGray;
            harness.ForegroundColor = Color.VeryLightGray;
            _mostRecentHarness = new HtmlSpan();
            _mostRecentHarness.ForegroundColor = Color.White;
            harness.Controls.Add(_mostRecentHarness);
            LogContainer.Controls.Add(harness);
        }

        /// <summary>
        /// Updates the status display for any log message that has a 
        /// name decorator.
        /// </summary>
        /// <param name="l">The log message object.</param>
        private void UpdateStatusDisplay(LogMessage l)
        {
            if (l.HasDecorator(LogDecorator.NameProperty))
            {
                string name = (string)l[LogDecorator.NameProperty];
                _statusDisplay.UpdateStatus(TruncateNameIfNeeded(name));
            }
        }

        /// <summary>
        /// A wrapper that both updates the status and then calls the start 
        /// message handler.
        /// </summary>
        /// <param name="action">The customized handler for the start message.</param>
        /// <returns>Returns an aggregate action that performs both actions.</returns>
        private Action<LogMessage> UpdateAndStart(Action<LogMessage> action)
        {
            return delegate(LogMessage l)
            {
                UpdateStatusDisplay(l);
                action(l);
            };
        }

        /// <summary>
        /// Increments the scenario counter, which includes ignored tests.
        /// </summary>
        private void IncrementScenarioCounter()
        {
            ++_scenarioCounter;
            if (_unitTestHarness != null)
            {
                _progressBar.PercentComplete = _unitTestHarness.TestMethodCount == 0 ? 100 : Convert.ToInt32(_scenarioCounter * 100 / _unitTestHarness.TestMethodCount);
                _progressText.InnerText = String.Format(System.Globalization.CultureInfo.CurrentCulture, "{0} of {1} test methods complete", _scenarioCounter.ToString(CultureInfo.CurrentCulture), _unitTestHarness.TestMethodCount.ToString(CultureInfo.CurrentCulture));
            }
        }

        /// <summary>
        /// Search in a container control for the first child element of a 
        /// particular type.
        /// </summary>
        /// <param name="container">The container control.</param>
        /// <typeparam name="CONTROL_TYPE">The control type to search for.</typeparam>
        /// <returns>Returns null or the first child element that matches.</returns>
        private static CONTROL_TYPE FindFirstChildElement<CONTROL_TYPE>(HtmlContainerControl container) 
            where CONTROL_TYPE : HtmlControlBase
        {
            Type type = typeof(CONTROL_TYPE);
            foreach (HtmlControlBase control in container.Controls)
            {
                if (type.IsInstanceOfType(control))
                {
                    return (CONTROL_TYPE)control;
                }
            }

            //return null;	
            //workaround for bug #448560:
            object dummy = null;
            return (CONTROL_TYPE) dummy;
        }

        /// <summary>
        /// Process a UTF result message.
        /// </summary>
        /// <param name="logMessage">The log message object.</param>
        private void ProcessResult(LogMessage logMessage)
        {
            if (logMessage.HasDecorator(UnitTestLogDecorator.TestMethodMetadata))
            {
                ScenarioResult result = (ScenarioResult) logMessage[UnitTestLogDecorator.ScenarioResult];
                TestOutcome outcome = result.Result;
                ITestMethod method = result.TestMethod;
                ITestClass test = result.TestClass;

                if (_startElements.ContainsKey(method))
                {
                    HtmlContainerControl elem = _startElements[method];
                    ++_statusDisplay.Total;
                    IncrementScenarioCounter();

                    switch (outcome)
                    {
                        case TestOutcome.Passed:
                        case TestOutcome.PassedButRunAborted:
                            break;
                        case TestOutcome.Inconclusive:
                        case TestOutcome.Failed:
                            elem.BackgroundColor = Color.Red;
                            elem.ForegroundColor = Color.White;
                            elem.Font.Bold = true;
                            ++_statusDisplay.Fail;

                            HtmlContainerControl nextSpan = CreateText(/* isDiv */ false);
                            nextSpan.SetStyleAttribute(CssAttribute.Position, "absolute");
                            nextSpan.SetStyleAttribute(CssAttribute.Right, new Unit(10));
                            elem.Controls.Add(nextSpan);

                            _failureSummary.AddFailure(test, method, nextSpan);
                            break;
                        default:
                            break;
                    }
                    _startElements.Remove(method);

                    // Hook up the details inspection control to the name
                    HtmlAnchor anchor = FindFirstChildElement<HtmlAnchor>(elem);
                    if (anchor != null)
                    {
                        ResultInspector inspector = new ResultInspector(result);
                        LogContainer.Controls.Add(inspector);
                        anchor.Click += (sender, args) => inspector.ToggleDropDown();
                        anchor.SetStyleAttribute(CssAttribute.Cursor, "hand");
                    }
                }
            }
        }

        /// <summary>
        /// Process [Bug(...)].
        /// </summary>
        /// <param name="l">A KnownBugLogMessage object.</param>
        private void ProcessBug(LogMessage l)
        {
            HtmlContainerControl container = new HtmlDiv();
            container.Padding.All = 4;
            container.BackgroundColor = Color.AnotherLightGray;

            HtmlContainerControl header = new HtmlDiv();
            header.InnerHtml = "This test has a known issue:";

            HtmlContainerControl message = CreateText();
            message.BorderStyle = BorderStyle.Solid;
            message.BorderWidth = 1;
            message.BorderColor = Color.VeryLightGray;
            message.BackgroundColor = Color.White;
            message.InnerHtml = l.Message;
            message.Padding.All = 2;
            container.Padding.All = 2;
            message.Margin.All = 2;
            
            container.Controls.Add(header);
            container.Controls.Add(message);
            LogContainer.Controls.Add(container);
        }

        /// <summary>
        /// Process [Ignore].
        /// </summary>
        /// <param name="l">The Ignore message.</param>
        private void ProcessIgnoreMessage(LogMessage l)
        {
            IncrementScenarioCounter();

            string name = (string)l[LogDecorator.NameProperty];
            TestGranularity gran = (TestGranularity)l[LogDecorator.TestGranularity];
            string extra = gran == TestGranularity.Test ? " (test class)" : String.Empty;

            HtmlContainerControl ignore = CreateText();
            ignore.InnerHtml = name + extra;
            ignore.ForegroundColor = Color.LightGray;
            ignore.Font.Strikeout = true;
            LogContainer.Controls.Add(ignore);
        }

        /// <summary>
        /// Process an Exception that was not the expected Exception type.
        /// </summary>
        /// <param name="l">The log message.</param>
        private void ProcessIncorrectException(LogMessage l)
        {
            string html = l.ToString();

            HtmlContainerControl text = CreateText();
            text.InnerHtml = html;
            text.BackgroundColor = Color.Manila;
            text.SetStyleAttribute(CssAttribute.Border, "1px solid " + Color.ManilaBorder);
            text.ForegroundColor = Color.DarkGray;
            LogContainer.Controls.Add(text);
        }

        /// <summary>
        /// Creates a new container control for text.
        /// </summary>
        /// <returns>Returns a new container control.</returns>
        private static HtmlContainerControl CreateText()
        {
            return CreateText(/* isDiv */ true);
        }

        /// <summary>
        /// Creates a new container control for text.
        /// </summary>
        /// <param name="isDiv">A value indicating whether this is a div or a 
        /// span (div is for true values).</param>
        /// <returns>Returns a new container control.</returns>
        private static HtmlContainerControl CreateText(bool isDiv)
        {
            HtmlContainerControl control;
            if (isDiv)
            {
                control = new HtmlDiv();
            }
            else
            {
                control = new HtmlSpan();
            }
            control.Margin.Bottom = 1;
            control.Padding.Top = control.Padding.Left = control.Padding.Bottom = 1;
            return control;
        }

        /// <summary>
        /// Remainder objects are just appended.
        /// </summary>
        /// <param name="message">The log message object.</param>
        protected override void ProcessRemainder(LogMessage message)
        {
            AppendFixedText(message.ToString());
        }
    }
}
