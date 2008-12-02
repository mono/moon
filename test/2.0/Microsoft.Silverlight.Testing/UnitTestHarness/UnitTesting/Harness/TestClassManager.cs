// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Test class manager.
    /// </summary>
    public class TestClassManager : UnitTestCompositeWorkItem
    {
        /// <summary>
        /// Test run filter object.
        /// </summary>
        private TestRunFilter _filter;

        /// <summary>
        /// Reference to the class and its metadata.
        /// </summary>
        private ITestClass _testClass;

        /// <summary>
        /// Queue of any tests to run.
        /// </summary>
        private CompositeWorkItem _testExecutionQueue;

        /// <summary>
        /// Class instance.
        /// </summary>
        private object _instance;

        /// <summary>
        /// A container type that handles an entire test class throughout the 
        /// test run.
        /// </summary>
        /// <param name="filter">Test run filter object.</param>
        /// <param name="testHarness">The unit test harness.</param>
        /// <param name="testClass">The test class metadata interface.</param>
        /// <param name="instance">The object instance.</param>
        /// <param name="provider">The unit test provider.</param>
        public TestClassManager(TestRunFilter filter, UnitTestHarness testHarness, ITestClass testClass, object instance, IUnitTestProvider provider) : base(testHarness, provider)
        {
            _filter = filter;
            _testClass = testClass;
            _testExecutionQueue = new CompositeWorkItem();
            _instance = instance;
        }

        /// <summary>
        /// Code run the first time this container is invoked.
        /// </summary>
        protected override void FirstInvoke()
        {
            if (Provider.HasCapability(UnitTestProviderCapabilities.ClassCanIgnore) && _testClass.Ignore) 
            {
                LogWriter.Ignore(TestGranularity.Test, _testClass.Name, _testClass);
                return;
            }

            PrepareInstance();
            PopulateTestMethods();

            Enqueue(() => LogWriter.TestClassStage(_testClass, TestStage.Starting));
            EnqueueTestClassInitialize();
            Enqueue(_testExecutionQueue);
            EnqueueTestClassCleanup();
            Enqueue(() => LogWriter.TestClassStage(_testClass, TestStage.Finishing));
        }

        /// <summary>
        /// Custom tests need to be prepared before use.
        /// </summary>
        private void PrepareInstance()
        {
            if (_instance != null & TestHarness is UnitTestHarness && _instance is CustomFrameworkUnitTest)
            {
                ((UnitTestHarness)TestHarness).PrepareCustomTestInstance((CustomFrameworkUnitTest)_instance);
            }
        }

        /// <summary>
        /// The test initialize method.
        /// </summary>
        private void EnqueueTestClassInitialize()
        {
            if (_testClass.ClassInitializeMethod != null)
            {
                EnqueueMethodDispatcher(_testClass.ClassInitializeMethod);
            }
        }

        /// <summary>
        /// The test cleanup method.
        /// </summary>
        private void EnqueueTestClassCleanup()
        {
            if (_testClass.ClassCleanupMethod != null)
            {
                EnqueueMethodDispatcher(_testClass.ClassCleanupMethod);
            }
        }

        /// <summary>
        /// Reflect through the class to find any test methods, and add them to 
        /// the list of queued methods.  Also, sorts the methods if appropriate 
        /// based on the settings file.
        /// </summary>
        private void PopulateTestMethods()
        {
            List<ITestMethod> methods = _filter.GetTestMethods(_testClass, _instance);

            foreach (ITestMethod test in methods)
            {
                _testExecutionQueue.Enqueue(new TestMethodManager(TestHarness, _testClass, test, _instance, Provider));
            }
        }

        /// <summary>
        /// Add a new method dispatcher to the test work item queue.  This is a 
        /// work item container which is able to manage its own internal test 
        /// work item queue.
        /// </summary>
        /// <param name="method">The method reflection object.</param>
        private void EnqueueMethodDispatcher(MethodInfo method)
        {
            IWorkItem task = new UnitTestMethodContainer(TestHarness, _instance, method, null, TestGranularity.TestScenario);
            Enqueue(task);
        }
    }
}