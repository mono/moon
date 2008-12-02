//------------------------------------------------------------------------------
// <copyright file="TestClassDispatcher.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// TestClassDispatcher
//     : ITestTaskDispatcher
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;
    using Microsoft.Silverlight.Testing.Utility;
    using LogMessages;
    using ITestHarness = Microsoft.Silverlight.Testing.TestHarness.ITestHarness;
    using Microsoft.Silverlight.Testing.TestHarness.LogMessageTypes;

    internal class TestClassDispatcher : UTDispatcher
    {

        public TestClassDispatcher(
            UnitTestHarness testHarness, ITestClass testClass, 
            object instance, IUnitTestProvider provider) 
            : base(testHarness, provider)
        {
            _testClass = testClass;
            _testExecutionQueue = new TestWorkItemDispatcher();
            _instance = instance;
        }

        private ITestClass _testClass;

        private TestWorkItemDispatcher _testExecutionQueue;

        private object _instance;

        protected override void FirstInvoke()
        {
            if (Provider.HasCapability(UnitTestProviderCapabilities.ClassCanIgnore) && _testClass.Ignore) {
                LogMessage(new IgnoreMessage(TestGranularity.Test, _testClass.Name));
                return;
            }

            // Setup Silverlight tests
            if (_instance != null & TestHarness is UnitTestHarness && _instance is SilverlightTest) {
                ((UnitTestHarness)TestHarness).PrepareSilverlightTestInstance((SilverlightTest)_instance);
            }

            //TODO: Implement the BugAttribute at the class level

            if(_testClass.ClassInitializeMethod != null) {
                EnqueueMethodDispatcher(_testClass.ClassInitializeMethod);
            }

            foreach (ITestMethod test in _testClass.GetTestMethods())
            {
                ITestWorkItem scenario = new TestMethodDispatcher(
                    TestHarness, _testClass, test, _instance, Provider);
                _testExecutionQueue.Enqueue(scenario);
            }

            Enqueue(_testExecutionQueue);

            if (_testClass.ClassCleanupMethod != null) {
                EnqueueMethodDispatcher(_testClass.ClassCleanupMethod);
            }

        }

        private void EnqueueMethodDispatcher(MethodInfo method)
        {
            ITestWorkItem task = new UtMethodDispatcher(TestHarness, _instance, method, TestGranularity.TestScenario);
            Enqueue(task);
        }

    }
}
