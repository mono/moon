// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Harness.Service;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A test harness for interacting with unit test providers such as Visual 
    /// Studio Team Test's metadata.
    /// </summary>
    public class UnitTestHarness : TestHarness
    {
        /// <summary>
        /// Display name for this harness.
        /// </summary>
        internal const string HarnessName = "Unit Testing";

        /// <summary>
        /// Manages the attachment state of a global exception handler.
        /// </summary>
        private GlobalExceptionHandler _globalExceptions;

        /// <summary>
        /// Container of all work items for the test harness.
        /// </summary>
        private ICompositeWorkItem _harnessTasks;

        /// <summary>
        /// Manager of the stack of dispatchers, so that the appropriate parent 
        /// container handles exceptions and completion events.
        /// </summary>
        private WorkItemsManager _dispatcherStack;

        /// <summary>
        /// Gets the list of results.
        /// </summary>
        public List<ScenarioResult> Results { get; private set; }

        /// <summary>
        /// Number of valid test assemblies encountered.
        /// </summary>
        private int _validTestAssemblies;

        /// <summary>
        /// The current run's known number of test methods.
        /// </summary>
        private int _knownTestMethods;

        /// <summary>
        /// Gets the log message writer instance.  This can be used to easily 
        /// post informative messages to the log message queue and providers.
        /// </summary>
        public UnitTestLogMessageWriter LogWriter { get; private set; }

        /// <summary>
        /// Gets or sets the logic factory used for instantiating the 
        /// unit test logic and management objects.
        /// </summary>
        public UnitTestLogicFactory LogicFactory { get; set; }

        /// <summary>
        /// Initiate unit test harness.
        /// </summary>
        public UnitTestHarness()
            : base()
        {
            _globalExceptions = new GlobalExceptionHandler(GlobalUnhandledExceptionListener);
            _dispatcherStack = new WorkItemsManager();
            Results = new List<ScenarioResult>();
            LogWriter = new UnitTestLogMessageWriter(this);
            LogicFactory = new UnitTestLogicFactory(this);

            // Attach to publishing event
            Publishing += (sender, args) => ReportCodeCoverage(TestService);

            // Create the initial dispatcher tasks
            CreateHarnessTasks();
        }

        /// <summary>
        /// Sets the unit test harness property for a test case that inherits 
        /// from the abstract base type 'CustomTest'.
        /// </summary>
        /// <param name="customTest">A CustomText instance.</param>
        public void PrepareCustomTestInstance(CustomFrameworkUnitTest customTest)
        {
            customTest.UnitTestHarness = this;
        }

        /// <summary>
        /// Gets the root container for test work to be completed.
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Advanced)]
        public ICompositeWorkItem RootCompositeWorkItem { get { return _harnessTasks; } }

        /// <summary>
        /// Gets the known number of test methods in the current test run.
        /// </summary>
        public int TestMethodCount { get { return _knownTestMethods; } }

        /// <summary>
        /// Gets or sets a value indicating whether to intercept exceptions at 
        /// the app domain level and funnel into the current container or not.
        /// </summary>
        public bool InterceptAllExceptions
        {
            get { return _globalExceptions.AttachGlobalHandler; }
            set { _globalExceptions.AttachGlobalHandler = value; }
        }

        /// <summary>
        /// Gets the internal DispatcherStack being used by the test harness.
        /// </summary>
        public WorkItemsManager DispatcherStack 
        { 
            get { return _dispatcherStack; }
        }

        /// <summary>
        /// Initialize the harness with a set of test assemblies.
        /// </summary>
        /// <param name="settings">Initial harness settings.</param>
        public override void Initialize(TestHarnessSettings settings)
        {
            // Let the base initialize the log providers
            base.Initialize(settings);

            // Attach to the unhandled exception handler
            InterceptAllExceptions = true;

            LogWriter.TestInfrastructure(Properties.UnitTestMessage.UnitTestHarness_Initialize_UnitTestHarnessInitialize);
            PrepareTestAssemblyTasks();
        }

        /// <summary>
        /// Restarts the run dispatcher.
        /// </summary>
        public override void RestartRunDispatcher()
        {
            if (_harnessTasks == null)
            {
                CreateHarnessTasks();
            }
            base.RestartRunDispatcher();
        }

        /// <summary>
        /// Track the results for our execution and also track the fail state.
        /// </summary>
        /// <param name="result">Scenario result to process.</param>
        public void TrackScenarioResult(ScenarioResult result)
        {
            Results.Add(result);
            State.IncrementTotalScenarios();

            if (result.Result != TestOutcome.Passed)
            {
                State.IncrementFailures();
            }
        }

        /// <summary>
        /// Attempts to report the code coverage information using the test 
        /// service provider. If there is no available coverage reporting 
        /// service, this is a silent failure. Only reports if >= 1 blocks 
        /// are hit.
        /// </summary>
        /// <param name="testService">The test service.</param>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Allows for future extensibility.")]
        private void ReportCodeCoverage(TestServiceProvider testService)
        {
            if (CodeCoverage.HitBlockCount > 0 && testService != null && testService.HasService(TestServiceFeature.CodeCoverageReporting))
            {
                CodeCoverageProvider ccp = testService.GetService<CodeCoverageProvider>(TestServiceFeature.CodeCoverageReporting);
                if (ccp != null)
                {
                    string data = CodeCoverage.GetCoverageData();
                    ccp.SaveCoverageData(data, /* no callback Action */ null);
                }
            }
        }

        /// <summary>
        /// Enqueue a test assembly from a simple Assembly reference.
        /// </summary>
        /// <param name="assembly">The test assembly.</param>
        /// <param name="runFilter">The run filter settings for the test assembly's run.</param>
        public void EnqueueTestAssembly(Assembly assembly, TestRunFilter runFilter)
        {
            IAssembly testAssembly = UnitTestProviders.GetAssemblyWrapper(this, assembly);
            if (testAssembly != null)
            {
                EnqueueTestAssembly(testAssembly, runFilter);
            }
        }

        /// <summary>
        /// Enqueues a test assembly.
        /// </summary>
        /// <param name="testAssembly">The test assembly metadata.</param>
        /// <param name="runFilter">The run filter settings for the test assembly's run.</param>
        public void EnqueueTestAssembly(IAssembly testAssembly, TestRunFilter runFilter)
        {
            AssemblyManager assemblyManager = LogicFactory.CreateAssemblyManager(testAssembly.Provider, runFilter, testAssembly);
            _harnessTasks.Enqueue(assemblyManager);
            _knownTestMethods = CalculateTotalMethods(assemblyManager, testAssembly, runFilter);
            ++_validTestAssemblies;
        }

        /// <summary>
        /// Flush the current log manager and then perform the next invoke.
        /// </summary>
        /// <returns>Returns true if work remains.</returns>
        protected override bool RunNextStep()
        {
            ProcessLogMessages();
            if (RootCompositeWorkItem == null)
            {
                throw new InvalidOperationException(Properties.UnitTestMessage.UnitTestHarness_RunNextStep_NoCompositeWorkItemsExist);
            }
            return RootCompositeWorkItem.Invoke();
        }

        /// <summary>
        /// Creates the test run filter for the initial run.
        /// </summary>
        /// <param name="settings">The unit test settings.</param>
        /// <returns>Returns a new TestRunFilter instance.</returns>
        protected virtual TestRunFilter CreateTestRunFilter(UnitTestSettings settings)
        {
            return String.IsNullOrEmpty(settings.TagExpression) ? new TestRunFilter(settings, this) : new TagTestRunFilter(settings, this, settings.TagExpression);
        }

        /// <summary>
        /// Determine what test assemblies need to be executed.  Enqueue tasks 
        /// for the unit test assembly providers to run the tests.
        /// </summary>
        private void PrepareTestAssemblyTasks()
        {
            // Regular unit test run
            UnitTestSettings settings = Settings as UnitTestSettings;
            TestRunFilter filter = CreateTestRunFilter(settings);

            // Log the filter information
            LogWriter.TestRunFilterSelected(filter);

            // Add the test assemblies
            foreach (Assembly assembly in Settings.TestAssemblies)
            {
                EnqueueTestAssembly(assembly, filter);
            }
            if (_validTestAssemblies > 0)
            {
                LogWriter.UnitTestHarnessStage(this, HarnessName, TestStage.Starting);
            }
            else
            {
                LogWriter.Warning(Properties.UnitTestMessage.UnitTestHarness_TestAssembliesNotActionable);
            }
        }

        /// <summary>
        /// Calculates the number of methods for a run.
        /// </summary>
        /// <param name="assemblyManager">The assembly manager.</param>
        /// <param name="assembly">The test assembly.</param>
        /// <param name="filter">The test run filter.</param>
        /// <returns>Returns the number of known methods returned.</returns>
        private static int CalculateTotalMethods(AssemblyManager assemblyManager, IAssembly assembly, TestRunFilter filter)
        {
            int total = 0;
            List<ITestClass> cls = filter.GetTestClasses(assembly, assemblyManager.ClassInstances);
            foreach (ITestClass test in cls)
            {
                object instance = assemblyManager.ClassInstances.GetInstance(test.Type);
                total += filter.GetTestMethods(test, instance).Count;
            }
            return total;
        }

        /// <summary>
        /// Event fired at the completion of the harness' work.
        /// </summary>
        /// <param name="sender">Sender object instance.</param>
        /// <param name="e">Event arguments.</param>
        private void HarnessComplete(object sender, EventArgs e)
        {
            LogWriter.UnitTestHarnessStage(this, HarnessName, TestStage.Finishing);
            _harnessTasks = null;
        }

        /// <summary>
        /// Listener event for any unhandled exceptions.
        /// </summary>
        /// <param name="sender">Sender object instance.</param>
        /// <param name="e">Event arguments.</param>
        private void GlobalUnhandledExceptionListener(object sender, EventArgs e)
        {
            if (DispatcherStack.CurrentCompositeWorkItem is CompositeWorkItem)
            {
                CompositeWorkItem cd = (CompositeWorkItem)DispatcherStack.CurrentCompositeWorkItem;
                Exception exception = GlobalExceptionHandler.GetExceptionObject(e);
                cd.WorkItemException(exception);
                GlobalExceptionHandler.ChangeExceptionBubbling(e, /* handled */ true);
            }
            else
            {
                GlobalExceptionHandler.ChangeExceptionBubbling(e, /* handled */ false);
            }
        }

        /// <summary>
        /// Creates the set of harness tasks to run and hooks up to the Complete event.
        /// </summary>
        private void CreateHarnessTasks()
        {
            _harnessTasks = new CompositeWorkItem();
            _harnessTasks.Complete += HarnessComplete;
        }
    }
}