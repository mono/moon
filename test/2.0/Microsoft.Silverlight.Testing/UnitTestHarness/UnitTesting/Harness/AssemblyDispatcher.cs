//------------------------------------------------------------------------------
// <copyright file="AssemblyDispatcher.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AssemblyDispatcher
//     : ITestTaskDispatcher
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;
    using ITestHarness = Microsoft.Silverlight.Testing.TestHarness.ITestHarness;
    using Microsoft.Silverlight.Testing.TestHarness.LogMessageTypes;
    using Microsoft.Silverlight.Testing.UnitTesting.Harness.LogMessages;

    internal class AssemblyDispatcher : UTDispatcher
    {

        public AssemblyDispatcher(UnitTestHarness testHarness, IUnitTestProvider provider, 
            IAssembly testAssembly) : base(testHarness, provider)
        {
            _assembly = testAssembly;
            _testClasses = new TestWorkItemDispatcher();
            _classInstances = new TestClassInstanceCollection();
        }

        protected override void FirstInvoke()
        {
            // Init loggers
            //TODO: Use correct logger
            LogMessage(new StartMessage(TestGranularity.TestGroup, _assembly.Name));
            
            if (Provider.HasCapability(UnitTestProviderCapabilities.AssemblySupportsInitializeMethod)
                && _assembly.AssemblyInitializeMethod != null) 
            {
                EnqueueMethodDispatcher(_assembly.AssemblyInitializeMethod);
            }

            // Add all test classes
            foreach (ITestClass testClass in _assembly.GetTestClasses())
            {
                ITestWorkItem test = new TestClassDispatcher(TestHarness, testClass, 
                    _classInstances.GetInstance(testClass.Type), Provider);
                _testClasses.Enqueue(test);
            }

            Enqueue(_testClasses);

            // Assembly cleanup
            if (Provider.HasCapability(UnitTestProviderCapabilities.AssemblySupportsCleanupMethod)
                && _assembly.AssemblyCleanupMethod != null) 
            {
                EnqueueMethodDispatcher(_assembly.AssemblyCleanupMethod);
            }

            Complete += delegate(object sender, EventArgs e)
            {
                LogMessage(new EndMessage(TestGranularity.TestGroup, _assembly.Name));
            };
        }

        private IAssembly _assembly;

        private TestWorkItemDispatcher _testClasses;

        private TestClassInstanceCollection _classInstances;

        internal TestWorkItemDispatcher TestClasses
        {
            get { return _testClasses; }
        }

        private void EnqueueMethodDispatcher(MethodInfo method)
        {
            object o = _classInstances.GetInstance(method.ReflectedType);

            if (o == null) {
                throw new NullReferenceException();
            }

            ITestWorkItem task = new UtMethodDispatcher(TestHarness, o, method, TestGranularity.TestScenario); // ??
            Enqueue(task);
        }

    }
}
