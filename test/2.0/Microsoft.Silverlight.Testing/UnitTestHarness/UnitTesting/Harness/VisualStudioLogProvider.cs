// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A log provider that outputs in a simple custom test format that Visual 
    /// Studio recognizes.
    /// </summary>
    public partial class VisualStudioLogProvider : LogProvider, ITestSettingsLogProvider
    {
        #region Default string constants

        /// <summary>
        /// The filename to use for saving test results.
        /// </summary>
        private const string DefaultTestResultsFilename = "TestResults.trx";

        /// <summary>
        /// The default test adapter type name.
        /// </summary>
        private const string DefaultTestAdapterTypeName = "Microsoft.VisualStudio.TestTools.TestTypes.Unit.UnitTestAdapter, Microsoft.VisualStudio.QualityTools.Tips.UnitTest.Adapter, Version=9.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a";

        /// <summary>
        /// The default test list name.
        /// </summary>
        private const string DefaultTestListName = "Unit tests";

        /// <summary>
        /// The default computer name.
        /// </summary>
        private const string DefaultComputerName = "Unknown";

        /// <summary>
        /// The default user name.
        /// </summary>
        private const string DefaultUserName = "Unknown";

        /// <summary>
        /// The default configuration name.
        /// </summary>
        private const string DefaultTestRunConfigurationName = "Local Test Run";

        /// <summary>
        /// The default configuration description.
        /// </summary>
        private const string DefaultTestRunConfigurationDescription = "This is a default test run configuration for a local test run.";

        #endregion

        /// <summary>
        /// Gets or sets the test results filename.
        /// </summary>
        public string TestResultsFileName { get; set; }

        /// <summary>
        /// Gets or sets the test adapter type name reported in the Visual 
        /// Studio results log file.
        /// </summary>
        public string TestAdapterTypeName { get; set; }

        /// <summary>
        /// Gets or sets the TestRunId.
        /// </summary>
        public string TestRunId { get; set; }

        /// <summary>
        /// Gets or sets the test list name.
        /// </summary>
        public string TestListName { get; set; }

        /// <summary>
        /// Gets or sets the computer name that is reported in the results
        /// log file.
        /// </summary>
        public string ComputerName { get; set; }

        /// <summary>
        /// Gets or sets the user name that is reported in the results file.
        /// </summary>
        public string UserName { get; set; }

        /// <summary>
        /// Gets or sets the TestRunConfigurationName.
        /// </summary>
        public string TestRunConfigurationName { get; set; }

        /// <summary>
        /// Gets or sets the TestRunConfigurationDescription.
        /// </summary>
        public string TestRunConfigurationDescription { get; set; }

        /// <summary>
        /// Gets or sets a prefix for use in the test run name.
        /// </summary>
        public string TestRunNamePrefix { get; set; }

        /// <summary>
        /// Gets or sets the TestRunConfigurationId.
        /// </summary>
        public string TestRunConfigurationId { get; set; }

        /// <summary>
        /// Gets the current assembly name.
        /// </summary>
        protected string CurrentAssemblyName { get; private set; }

        /// <summary>
        /// The Visual Studio log format writer.
        /// </summary>
        private Writer _writer;

        /// <summary>
        /// Initializes a new instance of the VisualStudioLogProvider class.
        /// </summary>
        public VisualStudioLogProvider()
            : base()
        {
            TestResultsFileName = DefaultTestResultsFilename;
            TestAdapterTypeName = DefaultTestAdapterTypeName;
            TestListName = DefaultTestListName;
            ComputerName = DefaultComputerName;
            UserName = DefaultUserName;
            TestRunConfigurationName = DefaultTestRunConfigurationName;
            TestRunConfigurationDescription = DefaultTestRunConfigurationDescription;
            TestRunNamePrefix = string.Empty;

            _writer = new Writer();
            RegisterLogHandlers();
        }

        /// <summary>
        /// Register the handler conditions of interest to this log provider.
        /// </summary>
        private void RegisterLogHandlers()
        {
            RegisterConditionalHandler(UnitTestMessageConditional.IsTestRunFilterMessage, TestRunFilterSelected);
            RegisterConditionalHandler(UnitTestMessageConditional.IsUnitTestStartMessage(UnitTestLogDecorator.TestAssemblyMetadata), TestAssemblyStart);
            RegisterConditionalHandler(UnitTestMessageConditional.HasUnitTestOutcome, ProcessResult);
            RegisterConditionalHandler(UnitTestMessageConditional.IsExceptionLogMessage, ProcessException);
            RegisterConditionalHandler(UnitTestMessageConditional.IsIncorrectExceptionLogMessage, ProcessIncorrectException);
            RegisterConditionalHandler(UnitTestMessageConditional.IsKnownBug, ProcessBug);
        }

        /// <summary>
        /// Assembly start code.
        /// </summary>
        /// <param name="l">The log message.</param>
        private void TestAssemblyStart(LogMessage l)
        {
            IAssembly assembly = (IAssembly)l[UnitTestLogDecorator.TestAssemblyMetadata];
            _writer.Started = DateTime.Now;
            CurrentAssemblyName = assembly.Name;

            // For now, we assume the log works best with a single test 
            // assembly and with undefined behavior when there are more.
            SetTestRunConfiguration(assembly);
        }

        /// <summary>
        /// Process an Exception that was not the expected Exception type.
        /// </summary>
        /// <param name="l">The log message.</param>
        private void ProcessIncorrectException(LogMessage l)
        {
            _writer.AddPendingErrorMessage(l.ToString());
        }

        /// <summary>
        /// Process an Exception that is logged or stored.
        /// </summary>
        /// <param name="l">The log message object.</param>
        private void ProcessException(LogMessage l)
        {
            Exception e = (Exception)l[UnitTestLogDecorator.ActualException];
            _writer.AddPendingException(e);
        }

        /// <summary>
        /// Sets the test run configuration information when the test assembly 
        /// begins processing.
        /// </summary>
        /// <param name="assembly">The test assembly metadata object.</param>
        private void SetTestRunConfiguration(IAssembly assembly)
        {
            _writer.TestRunConfigurationDescription = TestRunConfigurationDescription;
            _writer.TestRunConfigurationId = TestRunConfigurationId ?? NewGuid();
            _writer.TestRunConfigurationName = TestRunConfigurationName;
            _writer.TestRunId = TestRunId ?? NewGuid();

            _writer.TestRunName = GenerateTestRunName(assembly);
            _writer.TestRunUser = UserName;
        }

        /// <summary>
        /// Generate a name for the test run that will be used when 
        /// displaying the result in Visual Studio.
        /// </summary>
        /// <param name="assembly">The test assembly metadata object.</param>
        /// <returns>Returns the test run name.</returns>
        protected virtual string GenerateTestRunName(IAssembly assembly)
        {
            string displayDateTime = DateTime.Now.ToString("s", CultureInfo.InvariantCulture);
            string name = TestRunNamePrefix ?? string.Empty;
            if (name.Length > 0)
            {
                name += " ";
            }
            name += displayDateTime;
            return name;
        }

        /// <summary>
        /// Generates a new Guid string value.
        /// </summary>
        /// <returns>Returns a new Guid string value.</returns>
        private static string NewGuid()
        {
            return Guid.NewGuid().ToString();
        }

        /// <summary>
        /// Process a UTF result message.
        /// </summary>
        /// <param name="logMessage">The log message object.</param>
        private void ProcessResult(LogMessage logMessage)
        {
            if (logMessage.HasDecorator(UnitTestLogDecorator.TestMethodMetadata))
            {
                TestOutcome result = (TestOutcome)logMessage[LogDecorator.TestOutcome];
                ITestMethod method = (ITestMethod)logMessage[UnitTestLogDecorator.TestMethodMetadata];
                ITestClass test = (ITestClass)logMessage[UnitTestLogDecorator.TestClassMetadata];
                ScenarioResult sr = (ScenarioResult)logMessage[UnitTestLogDecorator.ScenarioResult];

                string storage = CurrentAssemblyName;
                string codeBase = CurrentAssemblyName;
                string adapterTypeName = TestAdapterTypeName;
                string className = test.Name;
                string testListName = TestListName;
                string computerName = ComputerName;
                DateTime startTime = sr.Started;
                DateTime endTime = sr.Finished;
                _writer.AddTestMethodResult(method, storage, codeBase, adapterTypeName, className, testListName, computerName, startTime, endTime, result);
                _writer.IncrementResults(result);
            }
        }

        /// <summary>
        /// Process [Bug(...)].
        /// </summary>
        /// <param name="l">A KnownBugLogMessage object.</param>
        private void ProcessBug(LogMessage l)
        {
            _writer.AddPendingWriteLine(l.Message);
        }

        /// <summary>
        /// The run filter has been selected.
        /// </summary>
        /// <param name="logMessage">The log message object.</param>
        private void TestRunFilterSelected(LogMessage logMessage)
        {
            TestRunFilter filter = (TestRunFilter)logMessage[UnitTestLogDecorator.TestRunFilter];
            TestListName = filter.TestRunName;
        }

        /// <summary>
        /// Saves the log file data.
        /// </summary>
        /// <param name="harness">The unit test harness.</param>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "This log provider and format only applies to the unit test harness.")]
        public void WriteLogFile(UnitTestHarness harness)
        {
            _writer.Finished = DateTime.Now;
            harness.WriteLogFile(TestResultsFileName, _writer.GetXmlAsString());
        }

        /// <summary>
        /// Initializes the test harness.
        /// </summary>
        /// <param name="settings">The test harness settings.</param>
        public void Initialize(TestHarnessSettings settings)
        {
            UnitTestHarness uth = settings.TestHarness as UnitTestHarness;
            if (uth != null)
            {
                // Attach to store the log file
                uth.Publishing += (o, e) => WriteLogFile(uth);

                // Look for a unique test run ID
                if (uth.TestService != null)
                {
                    string runId = uth.TestService.UniqueTestRunIdentifier;
                    if (!string.IsNullOrEmpty(runId))
                    {
                        TestRunId = runId;
                    }
                }
            }

            string filename;
            if (settings.Parameters.TryGetValue("log", out filename))
            {
                TestResultsFileName = filename;
            }

            // Read pre-defined optional settings and environment variables
            List<string> prefix = new List<string>();
            string initialPrefix;
            if (settings.Parameters.TryGetValue("testRunNamePrefix", out initialPrefix))
            {
                prefix.Add(initialPrefix);
            }

            string userName;
            if (settings.Parameters.TryGetValue("userName", out userName))
            {
                prefix.Add(userName);
                UserName = userName;
            }

            string computerName;
            if (settings.Parameters.TryGetValue("computerName", out computerName))
            {
                prefix.Add(computerName);
                ComputerName = computerName;
            }

            for (int i = 0; i < prefix.Count; ++i)
            {
                if (TestRunNamePrefix.Length > 0)
                {
                    TestRunNamePrefix += "_";
                }
                TestRunNamePrefix += prefix[i];
            }
        }
    }
}

