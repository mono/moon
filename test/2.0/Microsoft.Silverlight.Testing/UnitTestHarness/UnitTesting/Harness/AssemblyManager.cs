// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Container and manager type which handles an entire test assembly; contains sub work 
    /// items that represent all actions needed to execute its tests.
    /// </summary>
    public class AssemblyManager : UnitTestCompositeWorkItem
    {
        /// <summary>
        /// The test run filter object.
        /// </summary>
        private TestRunFilter _filter;

        /// <summary>
        /// Unit test provider-agnostic assembly metadata.
        /// </summary>
        private IAssembly _assembly;

        /// <summary>
        /// Container of work items for running test classes.
        /// </summary>
        private CompositeWorkItem _testClasses;

        /// <summary>
        /// Gets the collection of all the instances of the assembly's test
        /// classes, used to keep a single reference of the type.
        /// </summary>
        internal TestClassInstanceDictionary ClassInstances { get; private set; }

        /// <summary>
        /// Create a new assembly manager, takes in the harness, provider 
        /// reference and actual IAssembly object.
        /// </summary>
        /// <param name="runFilter">The test run filter object.</param>
        /// <param name="testHarness">Harness object.</param>
        /// <param name="provider">The unit test metadata provider.</param>
        /// <param name="testAssembly">The test assembly metadata object.</param>
        public AssemblyManager(TestRunFilter runFilter, UnitTestHarness testHarness, IUnitTestProvider provider, IAssembly testAssembly) : base(testHarness, provider)
        {
            _filter = runFilter;
            _assembly = testAssembly;
            _testClasses = new CompositeWorkItem();
            ClassInstances = new TestClassInstanceDictionary();
        }

        /// <summary>
        /// When the test run is ready to test the underlying test assembly that
        /// this class manages, perform reflection and enqueue work items to run
        /// the tests.
        /// </summary>
        protected override void FirstInvoke()
        {
            LogWriter.GranularAssemblyTestStage(_assembly, TestGranularity.TestGroup, TestStage.Starting);

            EnqueueAssemblyInitialize();
            EnqueueTestClasses();
            EnqueueAssemblyCleanup();

            Complete += delegate(object sender, EventArgs e) 
            {
                LogWriter.GranularAssemblyTestStage(_assembly, TestGranularity.TestGroup, TestStage.Finishing);
            };
        }

        /// <summary>
        /// The assembly initialize method.
        /// </summary>
        private void EnqueueAssemblyInitialize()
        {
            if (Provider.HasCapability(UnitTestProviderCapabilities.AssemblySupportsInitializeMethod)
                && _assembly.AssemblyInitializeMethod != null)
            {
                EnqueueMethodDispatcher(_assembly.AssemblyInitializeMethod);
            }
        }

        /// <summary>
        /// The assembly cleanup method.
        /// </summary>
        private void EnqueueAssemblyCleanup()
        {
            if (Provider.HasCapability(UnitTestProviderCapabilities.AssemblySupportsCleanupMethod)
                && _assembly.AssemblyCleanupMethod != null)
            {
                EnqueueMethodDispatcher(_assembly.AssemblyCleanupMethod);
            }
        }

        /// <summary>
        /// Reflect over all test classes in the assembly and add any which are not 
        /// filtered out into the test work item queue.
        /// </summary>
        private void EnqueueTestClasses()
        {
            IList<ITestClass> classes = _filter.GetTestClasses(_assembly, ClassInstances);
            foreach (ITestClass testClass in classes)
            {
                _testClasses.Enqueue(TestHarness.LogicFactory.CreateTestClassManager(Provider, _filter, testClass, ClassInstances.GetInstance(testClass.Type)));
            }
            Enqueue(_testClasses);
        }

        /// <summary>
        /// Helper to enqueue a new method dispatcher.
        /// </summary>
        /// <param name="method">The method reflection object.</param>
        private void EnqueueMethodDispatcher(MethodInfo method)
        {
            object o = ClassInstances.GetInstance(method.ReflectedType);
            if (o == null)
            {
                throw new InvalidOperationException(String.Format(CultureInfo.InvariantCulture, "Object of type {0} could not be instantiated.", method.ReflectedType.ToString()));
            }

            // CustomTest was known in the first release as "SilverlightTest"
            // These are tests that are not compatible with the full framework 
            // as the properties, methods and other features exposed are custom 
            // to this harness.
            CustomFrameworkUnitTest customTest = o as CustomFrameworkUnitTest;
            if (customTest != null)
            {
                TestHarness.PrepareCustomTestInstance(customTest);
            }
            IWorkItem task = new UnitTestMethodContainer(TestHarness, o, method, null, TestGranularity.TestScenario);
            Enqueue(task);
        }
    }
}