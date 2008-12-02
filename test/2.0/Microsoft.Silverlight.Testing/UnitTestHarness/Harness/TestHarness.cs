// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using Microsoft.Silverlight.Testing.Harness.Service;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Base implementation for test harnesses, implements 
    /// <see cref="ITestHarness" /> interface.
    /// </summary>
    public abstract class TestHarness : ITestHarness
    {
        /// <summary>
        /// Gets or sets the overall harness state - overloaded types can be 
        /// used to store additional information.
        /// </summary>
        public TestHarnessState State { get; set; }

        /// <summary>
        /// Gets the log providers list.
        /// </summary>
        public IList<LogProvider> LogProviders { get; private set; }

        /// <summary>
        /// Queue of log messages awaiting processing.
        /// </summary>
        private Queue<LogMessage> _queuedLogMessages;

        /// <summary>
        /// Gets or sets the wrapper that handles calling the next Run step 
        /// method until complete; allows for a virtual Run method.
        /// </summary>
        protected RunDispatcher RunDispatcher { get; set; }

        /// <summary>
        /// Gets the dictionary of Parameters passed into the test harness.
        /// </summary>
        public IDictionary<string, string> Parameters { get; private set; }

        /// <summary>
        /// Gets or sets the settings used to initialize the test harness.
        /// </summary>
        public TestHarnessSettings Settings { get; set; }

        /// <summary>
        /// The test harness is publishing results.
        /// </summary>
        public event EventHandler Publishing;

        /// <summary>
        /// Constructor for derived classes that instantiates a TestHarnessState
        /// object.
        /// </summary>
        protected TestHarness()
        {
            State = new TestHarnessState();
            LogProviders = new List<LogProvider>();
            _queuedLogMessages = new Queue<LogMessage>();
        }

        /// <summary>
        /// Constructor for derived classes that sets an initial 
        /// TestHarnessState and.
        /// </summary>
        /// <param name="initialTestHarnessState">Initial harness object. This 
        /// allows a developer to pass in a derived state class instance for 
        /// tracking test run information beyond the default.</param>
        protected TestHarness(TestHarnessState initialTestHarnessState)
        {
            State = initialTestHarnessState;
        }

        /// <summary>
        /// Gets the TestService referenced by the test harness settings. The 
        /// test service provides advanced, optional functionality that is 
        /// useful to harness and test case developers. A typical test service 
        /// operates outside the process or security boundary.
        /// </summary>
        public TestServiceProvider TestService
        {
            get
            {
                return Settings == null ? null : Settings.TestService;
            }
        }

        /// <summary>
        /// Adds a log provider to the listening log providers group.
        /// </summary>
        /// <param name="provider">Log provider object.</param>
        public void AddLogProvider(LogProvider provider)
        {
            LogProviders.Add(provider);
        }

        /// <summary>
        /// Enqueue a log message object for processing by the log providers.
        /// </summary>
        /// <param name="message">The log message object.</param>
        public void QueueLogMessage(LogMessage message)
        {
            _queuedLogMessages.Enqueue(message);
        }

        /// <summary>
        /// Initialization method.
        /// </summary>
        /// <param name="settings">Optional, non-null settings.</param>
        public virtual void Initialize(TestHarnessSettings settings)
        {
            if (settings != null)
            {
                InitializeSettings(settings);
            }
        }

        /// <summary>
        /// Begin running the test harness.
        /// </summary>
        /// <remarks>
        /// Make sure to subscribe to the Complete event before calling this 
        /// method, in some harnesses this may be a synchronous Run followed 
        /// immediately by the Complete event being fired.
        /// </remarks>
        public virtual void Run()
        {
            // Initialize any providers that need access to the settings
            InitializeLogProviders();

            // Continue the run
            RestartRunDispatcher();
        }

        /// <summary>
        /// Restart the run method manager for the harness.
        /// </summary>
        public virtual void RestartRunDispatcher()
        {
            this.RunDispatcher = RunDispatcher.Create(RunNextStep);
            this.RunDispatcher.Complete += new EventHandler(RunDispatcherComplete);
            this.RunDispatcher.Run();
        }

        /// <summary>
        /// Complete event.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The event arguments.</param>
        [SuppressMessage("Microsoft.Security", "CA2109:ReviewVisibleEventHandlers", Justification = "The protected method makes this testable.")]
        protected void RunDispatcherComplete(object sender, EventArgs e)
        {
            RunDispatcher = null;

            OnTestHarnessCompleted();
            PublishResults();
        }

        /// <summary>
        /// Stores a log file for the test run. Depending on the execution 
        /// environment, this call may not successful.
        /// </summary>
        /// <param name="logName">The name of the log file.</param>
        /// <param name="fileContent">The log file content as a string.</param>
        public virtual void WriteLogFile(string logName, string fileContent)
        {
            if (TestService != null && TestService.HasService(TestServiceFeature.TestReporting))
            {
                TestReportingProvider trp = TestService.GetService<TestReportingProvider>(TestServiceFeature.TestReporting);
                if (trp != null)
                {
                    trp.WriteLog(null, logName, fileContent);
                }
            }
        }

        /// <summary>
        /// Publish results using the test service, if available.
        /// </summary>
        protected virtual void PublishResults()
        {
            // Fire the publishing event
            OnPublishing();

            // Final publish on the test service
            if (TestService != null && TestService.HasService(TestServiceFeature.TestReporting))
            {
                TestReportingProvider trp = TestService.GetService<TestReportingProvider>(TestServiceFeature.TestReporting);
                if (trp != null)
                {
                    int failures = State.Failures;
                    int total = State.TotalScenarios;
                    trp.ReportFinalResult(null, State.Failed, failures, total, "" /* message is not supported right now */);
                }
            }
        }

        /// <summary>
        /// Process all queued log messages.
        /// </summary>
        protected void ProcessLogMessages()
        {
            while (_queuedLogMessages.Count > 0)
            {
                LogMessage message = _queuedLogMessages.Dequeue();
                foreach (LogProvider provider in LogProviders)
                {
                    provider.Process(message);
                }
            }
        }

        /// <summary>
        /// Fill member variables with any non-null settings of the same type.
        /// </summary>
        /// <param name="settings">Settings container class.</param>
        private void InitializeSettings(TestHarnessSettings settings)
        {
            if (settings.LogProviders != null)
            {
                LogProviders = settings.LogProviders;
            }
            if (settings.Parameters != null)
            {
                Parameters = settings.Parameters;   
            }
            Settings = settings;
        }
        
        /// <summary>
        /// Initializes all log providers.
        /// </summary>
        private void InitializeLogProviders()
        {
            foreach (LogProvider provider in LogProviders)
            {
                ITestSettingsLogProvider prov = provider as ITestSettingsLogProvider;
                if (prov != null)
                {
                    prov.Initialize(Settings);
                }
            }
        }

        /// <summary>
        /// Abstract method that continues harness execution.
        /// </summary>
        /// <remarks>
        /// When implementing RunNextStep: Make sure to call LogManager.Flush 
        /// as needed (typically in this method) and convienient.
        /// </remarks>
        /// <returns>
        /// True as long as additional work remains for the harness.
        /// </returns>
        protected abstract bool RunNextStep();

        /// <summary>
        /// Complete event fired when the test harness has finished its test 
        /// run.
        /// </summary>
        public event EventHandler<TestHarnessCompletedEventArgs> TestHarnessCompleted;

        /// <summary>
        /// Call the TestHarnessCompleted event.
        /// </summary>
        protected void OnTestHarnessCompleted()
        {
            if (TestHarnessCompleted != null)
            {
                TestHarnessCompleted(this, new TestHarnessCompletedEventArgs(State));
            }
        }

        /// <summary>
        /// Call the Publishing event.
        /// </summary>
        protected void OnPublishing()
        {
            if (Publishing != null)
            {
                Publishing(this, EventArgs.Empty);
            }
        }
    }
}