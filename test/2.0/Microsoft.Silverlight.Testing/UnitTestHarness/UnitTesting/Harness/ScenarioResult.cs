// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Text;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A result from a test scenario.
    /// </summary>
    public class ScenarioResult
    {
        /// <summary>
        /// Gets or sets the scenario's started time.
        /// </summary>
        public DateTime Started { get; set; }

        /// <summary>
        /// Gets or sets the scenario's finishing time.
        /// </summary>
        public DateTime Finished { get; set; }

        /// <summary>
        /// Gets the test method metadata.
        /// </summary>
        public ITestMethod TestMethod { get; private set; }

        /// <summary>
        /// Gets the test class metadata.
        /// </summary>
        public ITestClass TestClass { get; private set; }

        /// <summary>
        /// Gets or sets the Result object.
        /// </summary>
        public TestOutcome Result { get; set; }

        /// <summary>
        /// Gets the Exception instance, if any.
        /// </summary>
        public Exception Exception { get; private set; }

        /// <summary>
        /// Creates a result record.
        /// </summary>
        /// <param name="method">Test method metadata object.</param>
        /// <param name="testClass">Test class metadata object.</param>
        /// <param name="result">Test result object.</param>
        /// <param name="exception">Exception instance, if any.</param>
        public ScenarioResult(ITestMethod method, ITestClass testClass, TestOutcome result, Exception exception)
        {
            TestClass = testClass;
            TestMethod = method;
            Exception = exception;
            Result = result;
        }

        /// <summary>
        /// The string representation of the ScenarioResult.
        /// </summary>
        /// <returns>Returns a verbose string representation of the result.</returns>
        public override string ToString()
        {
            StringBuilder s = new StringBuilder();
            s.AppendLine("Started: " + Started.ToString());
            s.AppendLine("Finished: " + Finished.ToString());
            TimeSpan duration = Finished.Subtract(Started);
            s.AppendLine("Duration: " + duration.ToString());
            s.AppendLine("TestMethod: " + TestMethod.ToString());
            s.AppendLine("TestClass: " + TestClass.ToString());
            s.AppendLine("Result: " + Result.ToString());
            if (Exception != null)
            {
                s.AppendLine();
                s.AppendLine("Exception:");
                s.AppendLine(Exception.Message);
                s.AppendLine(Exception.StackTrace);
            }
            return s.ToString();
        }
    }
}