//------------------------------------------------------------------------------
// <copyright file="TestMethodDispatcher.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// TestMethodDispatcher
//     : ITestTaskDispatcher
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;
    using LogMessages;
    using ITestHarness = Microsoft.Silverlight.Testing.TestHarness.ITestHarness;
    using Microsoft.Silverlight.Testing.TestHarness.LogMessageTypes;

    internal class TestMethodDispatcher : UTDispatcher
    {

        public TestMethodDispatcher(
            UnitTestHarness testHarness,
            ITestClass testClass, 
            ITestMethod testMethod, object instance, 
            IUnitTestProvider provider
            ) : base(testHarness, provider)
        {
            _testClass = testClass;
            _testMethod = testMethod;
            _instance = instance;
        }

        private ITestClass _testClass;

        private ITestMethod _testMethod;

        private object _instance;

        protected override void FirstInvoke()
        {
            if (Provider.HasCapability(UnitTestProviderCapabilities.MethodCanIgnore) && _testMethod.Ignore) {
                LogMessage(new IgnoreMessage(TestGranularity.TestScenario, _testMethod.Name));
                this.TestWorkItemComplete();
                return;
            }

            if(_testClass.TestInitializeMethod != null) {
                EnqueueMethodDispatcher(_testClass.TestInitializeMethod);
            }

            EnqueueMethodDispatcher(_testMethod.Method);
            
            if (_testClass.TestCleanupMethod != null) {
                EnqueueMethodDispatcher(_testClass.TestCleanupMethod);
            }

        }

        private void EnqueueMethodDispatcher(MethodInfo method)
        {
            ITestWorkItem task = new UtMethodDispatcher(TestHarness, _instance, method, TestGranularity.TestScenario);
            Enqueue(task);
        }

    }
}
