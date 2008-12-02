// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// The extended writer for the unit testing harness and consumers.
    /// </summary>
    public class UnitTestLogMessageWriter : LogMessageWriter
    {
        /// <summary>
        /// Initializes the unit test log message writer helper.
        /// </summary>
        /// <param name="harness">The test harness reference.</param>
        public UnitTestLogMessageWriter(ITestHarness harness) : base(harness) 
        { 
        }

        /// <summary>
        /// Marks a message as a unit test system-specific message.
        /// </summary>
        /// <param name="message">The log message object.</param>
        private static void MarkUnitTestMessage(LogMessage message)
        {
            message[UnitTestLogDecorator.IsUnitTestMessage] = true;
        }

        /// <summary>
        /// An incorrect exception type has occurred.
        /// </summary>
        /// <param name="expectedExceptionType">The expected type.</param>
        /// <param name="actualExceptionType">The actual exception's type.</param>
        /// <param name="test">The test metadata.</param>
        /// <param name="method">The method metadata.</param>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "Clarifies API purpose.")]
        public void IncorrectException(Type expectedExceptionType, Type actualExceptionType, ITestClass test, ITestMethod method)
        {
            string message = String.Format(System.Globalization.CultureInfo.CurrentCulture, Properties.UnitTestMessage.LogIncorrectExceptionType, actualExceptionType.Name, expectedExceptionType.Name);
            LogMessage m = Create(LogMessageType.Error, message);
            MarkUnitTestMessage(m);
            DecorateTestGranularity(m, TestGranularity.TestScenario);
            m[UnitTestLogDecorator.IncorrectExceptionMessage] = true;
            m[UnitTestLogDecorator.ExpectedExceptionType] = expectedExceptionType;
            m[UnitTestLogDecorator.ActualExceptionType] = actualExceptionType;
            m[UnitTestLogDecorator.TestClassMetadata] = test;
            m[UnitTestLogDecorator.TestMethodMetadata] = method;
            Enqueue(m);
        }

        /// <summary>
        /// No Exception was intercepted, yet one was expected.
        /// </summary>
        /// <param name="expectedExceptionType">The expected exception type.</param>
        /// <param name="test">The test class metadata.</param>
        /// <param name="method">The test method metadata.</param>
        public void NoExceptionWhenExpected(Type expectedExceptionType, ITestClass test, ITestMethod method)
        {
            string message = String.Format(System.Globalization.CultureInfo.CurrentCulture, Properties.UnitTestMessage.LogNoException, expectedExceptionType.ToString());
            LogMessage m = Create(LogMessageType.Error, message);
            MarkUnitTestMessage(m);
            DecorateTestGranularity(m, TestGranularity.TestScenario);
            m[UnitTestLogDecorator.IncorrectExceptionMessage] = true;
            m[UnitTestLogDecorator.ExpectedExceptionType] = expectedExceptionType;
            m[UnitTestLogDecorator.TestClassMetadata] = test;
            m[UnitTestLogDecorator.TestMethodMetadata] = method;
            Enqueue(m);
        }

        /// <summary>
        /// Logs and Exception that was intercepted or observed.
        /// </summary>
        /// <param name="exception">The actual Exception instance.</param>
        /// <param name="test">The test class metadata.</param>
        /// <param name="method">The test method metadata.</param>
        public void LogException(Exception exception, ITestClass test, ITestMethod method)
        {
            string message = String.Format(System.Globalization.CultureInfo.CurrentCulture, Properties.UnitTestMessage.LogException, exception.GetType().ToString(), exception.Message);
            LogMessage m = Create(LogMessageType.Error, message);
            MarkUnitTestMessage(m);
            DecorateTestGranularity(m, TestGranularity.TestScenario);
            m[UnitTestLogDecorator.ActualException] = exception;
            m[UnitTestLogDecorator.TestClassMetadata] = test;
            m[UnitTestLogDecorator.TestMethodMetadata] = method;
            Enqueue(m);
        }

        /// <summary>
        /// Enqueues a Ignore message.
        /// </summary>
        /// <param name="granularity">The granularity of the ignore operation.</param>
        /// <param name="name">The name of the test skipped.</param>
        public void Ignore(TestGranularity granularity, string name, ITestMethod method)
        {
            string message = String.Format(System.Globalization.CultureInfo.CurrentCulture, Properties.UnitTestMessage.LogIgnore, name);
            LogMessage m = Create(LogMessageType.TestExecution, message);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, name);
            DecorateTestGranularity(m, granularity);
            m[UnitTestLogDecorator.IgnoreMessage] = true;
            m[UnitTestLogDecorator.TestMethodMetadata] = method;
            Enqueue(m);
        }

        /// <summary>
        /// Enqueues a Ignore message.
        /// </summary>
        /// <param name="granularity">The granularity of the ignore operation.</param>
        /// <param name="name">The name of the test skipped.</param>
        public void Ignore(TestGranularity granularity, string name, ITestClass test)
        {
            string message = String.Format(System.Globalization.CultureInfo.CurrentCulture, Properties.UnitTestMessage.LogIgnore, name);
            LogMessage m = Create(LogMessageType.TestExecution, message);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, name);
            DecorateTestGranularity(m, granularity);
            m[UnitTestLogDecorator.IgnoreMessage] = true;
            m[UnitTestLogDecorator.TestClassMetadata] = test;
            Enqueue(m);
        }

        /// <summary>
        /// Enqueues a message containing a test run filter.
        /// </summary>
        /// <param name="filter">The test run filter.</param>
        public void TestRunFilterSelected(TestRunFilter filter)
        {
            string message = filter.TestRunName;
            LogMessage m = Create(LogMessageType.TestInfrastructure, message);
            MarkUnitTestMessage(m);
            m[UnitTestLogDecorator.TestRunFilter] = filter;
            Enqueue(m);
        }

        /// <summary>
        /// Records a log message that indicates a named, granular test stage has 
        /// happened.
        /// </summary>
        /// <param name="assembly">The assembly metadata object.</param>
        /// <param name="granularity">The test granularity value.</param>
        /// <param name="stage">The test stage value.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "This API is not available in Silverlight's BCL.")]
        public void GranularAssemblyTestStage(IAssembly assembly, TestGranularity granularity, TestStage stage)
        {
            string message = granularity.ToString() + assembly.Name + " " + stage.ToString().ToLower(System.Globalization.CultureInfo.InvariantCulture);
            string name = assembly.Name;

            LogMessage m = Create(LogMessageType.TestExecution, message);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, name);
            DecorateTestGranularity(m, granularity);
            m[UnitTestLogDecorator.TestAssemblyMetadata] = assembly;
            DecorateTestStage(m, stage);
            Enqueue(m);
        }

        /// <summary>
        /// Log a test class's stage.
        /// </summary>
        /// <param name="test">The test class metadata object.</param>
        /// <param name="stage">The test stage.</param>
        public void TestClassStage(ITestClass test, TestStage stage)
        {
            TestGranularity granularity = TestGranularity.Test;
            LogMessage m = Create(LogMessageType.TestExecution, test.Name);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, test.Name);
            DecorateTestGranularity(m, granularity);
            m[UnitTestLogDecorator.TestClassMetadata] = test;
            DecorateTestStage(m, stage);
            Enqueue(m);
        }

        /// <summary>
        /// Log the result of a unit test scenario.
        /// </summary>
        /// <param name="result">The result of the test.</param>
        public void TestResult(ScenarioResult result)
        {
            TestOutcome outcome = result.Result;
            string name = result.TestClass.Name + "." + result.TestMethod.Name;
            LogMessage m = Create(LogMessageType.TestResult, name);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, result.TestMethod.Name);
            DecorateTestGranularity(m, TestGranularity.TestScenario);
            m[UnitTestLogDecorator.ScenarioResult] = result;
            m[UnitTestLogDecorator.TestMethodMetadata] = result.TestMethod;
            m[UnitTestLogDecorator.TestClassMetadata] = result.TestClass;
            DecorateTestOutcome(m, outcome);
            Enqueue(m);
        }

        /// <summary>
        /// Log a test method's stage.
        /// </summary>
        /// <param name="method">The test method metadata object.</param>
        /// <param name="stage">The test stage.</param>
        public void TestMethodStage(ITestMethod method, TestStage stage)
        {
            TestGranularity granularity = TestGranularity.TestScenario;
            LogMessage m = Create(LogMessageType.TestExecution, method.Name);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, method.Name);
            DecorateTestGranularity(m, granularity);
            m[UnitTestLogDecorator.TestMethodMetadata] = method;
            DecorateTestStage(m, stage);
            Enqueue(m);
        }

        /// <summary>
        /// Records a harness state for the unit test harness.
        /// </summary>
        /// <param name="harness">The unit test harness.</param>
        /// <param name="name">The harness name.</param>
        /// <param name="stage">The test stage.</param>
        public void UnitTestHarnessStage(UnitTestHarness harness, string name, TestStage stage)
        {
            LogMessage m = Create(LogMessageType.TestExecution, name);
            MarkUnitTestMessage(m);
            DecorateNameProperty(m, name);
            DecorateTestGranularity(m, TestGranularity.Harness);
            m[UnitTestLogDecorator.UnitTestHarness] = harness;
            DecorateTestStage(m, stage);
            Enqueue(m);
        }
    }
}
