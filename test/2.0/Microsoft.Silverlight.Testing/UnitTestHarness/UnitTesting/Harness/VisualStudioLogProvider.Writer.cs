// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

/*
 * Moonlight: Need System.Xml.Linq
 */

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Xml.Linq;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A log provider that outputs in a simple custom test format that Visual 
    /// Studio recognizes.
    /// </summary>
    public partial class VisualStudioLogProvider : LogProvider
    {
        /// <summary>
        /// A log provider that outputs in a simple custom test format that 
        /// Visual Studio recognizes. This is a simple, subset writer.
        /// </summary>
        private class Writer
        {
            /// <summary>
            /// Date time format string representing round-trip date/time 
            /// pattern.
            /// </summary>
            private const string DateTimeFormat = "o";

            /// <summary>
            /// A unique ID representing the Visual Studio unit test type ID.
            /// </summary>
            private const string DefaultTestTypeId = "13cdc9d9-ddb5-4fa4-a97d-d965ccfc6d4b";

            /// <summary>
            /// The namespace for Visual Studio team test results.
            /// </summary>
            private readonly XNamespace ResultsXmlNamespace = "http://microsoft.com/schemas/VisualStudio/TeamTest/2006";

            /// <summary>
            /// Gets or sets the time that the test run started.
            /// </summary>
            public DateTime Started { get; set; }

            /// <summary>
            /// Gets or sets the time that the writer object and/or test run was created.
            /// </summary>
            public DateTime Created { get; set; }
            
            /// <summary>
            /// Gets or sets the time that the test run was finished.
            /// </summary>
            public DateTime Finished { get; set; }

            /// <summary>
            /// Gets the root XML node for the test run and its results.
            /// </summary>
            protected XElement TestRun { get; private set; }

            /// <summary>
            /// Gets or sets the test type ID used for new results.
            /// </summary>
            public string UnitTestTestTypeId { get; set; }

            /// <summary>
            /// Gets the TestRunConfiguration element.
            /// </summary>
            protected XElement TestRunConfiguration { get; private set; }

            /// <summary>
            /// Gets the Counters element.
            /// </summary>
            protected XElement Counters { get; private set; }

            /// <summary>
            /// Gets the TestDefinitions element.
            /// </summary>
            protected XElement TestDefinitions { get; private set; }

            /// <summary>
            /// Gets the TestLists element.
            /// </summary>
            protected XElement TestLists { get; private set; }

            /// <summary>
            /// Gets the TestEntries element.
            /// </summary>
            protected XElement TestEntries { get; private set; }

            /// <summary>
            /// Gets the Times element.
            /// </summary>
            protected XElement Times { get; private set; }

            /// <summary>
            /// Gets the ResultSummary element.
            /// </summary>
            protected XElement ResultSummary { get; private set; }

            /// <summary>
            /// Gets the Results element.
            /// </summary>
            protected XElement Results { get; private set; }

            /// <summary>
            /// A dictionary containing test list names and guids.
            /// </summary>
            private Dictionary<string, Guid> _testLists;

            /// <summary>
            /// Stores temporary, pending elements for the next result.
            /// </summary>
            private List<XElement> _pendingElements;

            /// <summary>
            /// Initializes a new Writer object.
            /// </summary>
            public Writer()
            {
                _pendingElements = new List<XElement>();

                _testLists = new Dictionary<string, Guid>();

                UnitTestTestTypeId = DefaultTestTypeId;
                Created = DateTime.Now;
                CreateInitialDocument();
            }

            /// <summary>
            /// Stores property values in the respective elements, clears any 
            /// lookup dictionaries.
            /// </summary>
            private void FinalizeContent()
            {
                Times.SetAttributeValue("creation", ToDateString(Created));
                Times.SetAttributeValue("queuing", ToDateString(Created));
                Times.SetAttributeValue("start", ToDateString(Started));
                Times.SetAttributeValue("finish", ToDateString(Finished));
                
                // Create test lists
                foreach (string list in _testLists.Keys)
                {
                    XElement test = CreateElement("TestList");
                    test.SetAttributeValue("name", list);
                    test.SetAttributeValue("id", _testLists[list].ToString());
                    TestLists.Add(test);
                }

                // Reclaim some of the memory used for element lookups
                _testLists.Clear();
            }

            /// <summary>
            /// Returns a string value of the DateTime object.
            /// </summary>
            /// <param name="dateTime">The DateTime object.</param>
            /// <returns>Returns the formatted string.</returns>
            private static string ToDateString(DateTime dateTime)
            {
                return dateTime.ToString(DateTimeFormat, CultureInfo.InvariantCulture);
            }

            /// <summary>
            /// Returns the XML log file as a string.
            /// </summary>
            /// <returns>The XML value.</returns>
            public string GetXmlAsString()
            {
                FinalizeContent();
                return TestRun.ToString();
            }

            /// <summary>
            /// Creates the initial results document and its XElements.
            /// </summary>
            private void CreateInitialDocument()
            {
                TestRun = CreateElement("TestRun");

                TestRunConfiguration = CreateElement("TestRunConfiguration");
                TestRun.Add(TestRunConfiguration);

                ResultSummary = CreateElement("ResultSummary");
                Counters = CreateElement("Counters");
                ResultSummary.Add(Counters);
                TestRun.Add(ResultSummary);

                Times = CreateElement("Times");
                TestRun.Add(Times);

                TestDefinitions = CreateElement("TestDefinitions");
                TestRun.Add(TestDefinitions);

                TestLists = CreateElement("TestLists");
                TestRun.Add(TestLists);

                TestEntries = CreateElement("TestEntries");
                TestRun.Add(TestEntries);

                Results = CreateElement("Results");
                TestRun.Add(Results);

                RunOutcome = TestOutcome.NotExecuted;
            }

            /// <summary>
            /// Creates a new XElement within the results XML namespace.
            /// </summary>
            /// <param name="name">The element name.</param>
            /// <returns>Returns a new named element.</returns>
            private XElement CreateElement(string name)
            {
                return new XElement(ResultsXmlNamespace + name);
            }

            // Things to track:
            // - writer created
            // - run started
            // - run ended [when save is called?]

            // Need very clear way to store time data for results (start, end; calc. elapsed)
            // Need to be able to tie in the result with the log system for dig into data correctly
            // 
            // Can the TestRunFilter set or provide an "execution Id"?

            /// <summary>
            /// Sets the TestRunId.
            /// </summary>
            public string TestRunId { set { TestRun.SetAttributeValue("id", value); } }

            /// <summary>
            /// Sets the TestRunName.
            /// </summary>
            public string TestRunName { set { TestRun.SetAttributeValue("name", value); } }

            /// <summary>
            /// Sets the TestRunUser.
            /// </summary>
            public string TestRunUser { set { TestRun.SetAttributeValue("runUser", value); } }

            /// <summary>
            /// Sets the TestRunConfigurationName.
            /// </summary>
            public string TestRunConfigurationName { set { TestRunConfiguration.SetAttributeValue("name", value); } }

            /// <summary>
            /// Sets the TestRunConfigurationId.
            /// </summary>
            public string TestRunConfigurationId { set { TestRunConfiguration.SetAttributeValue("id", value); } }

            /// <summary>
            /// Sets the overall run outcome value.
            /// </summary>
            public TestOutcome RunOutcome { set { ResultSummary.SetAttributeValue("outcome", value.ToString()); } }

            /// <summary>
            /// The total number of scenarios.
            /// </summary>
            private int _total;

            /// <summary>
            /// The set of outcomes and counts.
            /// </summary>
            private Dictionary<TestOutcome, int> _outcomes = new Dictionary<TestOutcome, int>();

            /// <summary>
            /// Increment the number of passing results.
            /// </summary>
            /// <param name="outcome">The test outcome.</param>
            public void IncrementResults(TestOutcome outcome)
            {
                if (_total == 0)
                {
                    RunOutcome = outcome;
                }

                ++_total;
                Counters.SetAttributeValue("total", _total.ToString(CultureInfo.InvariantCulture));

                if (!_outcomes.ContainsKey(outcome))
                {
                    _outcomes[outcome] = 0;
                }
                ++_outcomes[outcome];

                string value = outcome.ToString();
                string outcomeName = value.Substring(0, 1).ToLower(CultureInfo.InvariantCulture) + value.Substring(1);
                Counters.SetAttributeValue(outcomeName, _outcomes[outcome].ToString(CultureInfo.InvariantCulture));
            }

            /// <summary>
            /// Sets the TestRunConfigurationDescription.
            /// </summary>
            public string TestRunConfigurationDescription
            {
                set
                {
                    TestRunConfiguration.SetElementValue(ResultsXmlNamespace + "Description", value);
                }
            }

            /// <summary>
            /// Adds the result of a test method into the log.
            /// </summary>
            /// <param name="test">The test metadata.</param>
            /// <param name="storage">The storage value.</param>
            /// <param name="codeBase">The code base value.</param>
            /// <param name="adapterTypeName">The adapter type name.</param>
            /// <param name="className">The class name.</param>
            /// <param name="testListName">The test list name.</param>
            /// <param name="computerName">The computer name.</param>
            /// <param name="startTime">The start time.</param>
            /// <param name="endTime">The end time.</param>
            /// <param name="outcome">The outcome.</param>
            public void AddTestMethodResult(
                ITestMethod test,
                string storage,
                string codeBase,
                string adapterTypeName,
                string className,
                string testListName,
                string computerName,
                DateTime startTime,
                DateTime endTime,
                TestOutcome outcome)
            {
                if (test == null)
                {
                    throw new ArgumentNullException("test");
                }

                // Friendly name of the test
                string name = test.Name;

                // Generate GUIDs.
                string testId = Guid.NewGuid().ToString();
                string executionId = Guid.NewGuid().ToString();
                string testListId = GetTestListGuid(testListName);

                // UnitTest element.
                XElement unitTest = CreateElement("UnitTest");
                unitTest.SetAttributeValue("name", name);
                unitTest.SetAttributeValue("storage", storage);
                unitTest.SetAttributeValue("id", testId);

                XElement owners = CreateElement("Owners");
                XElement owner = CreateElement("Owner");
                string ownerString = test.Owner ?? string.Empty;
                owner.SetAttributeValue("name", ownerString);
                owners.Add(owner);
                unitTest.Add(owners);

                if (!string.IsNullOrEmpty(test.Description))
                {
                    XElement description = CreateElement("Description");
                    description.SetValue(test.Description);
                    unitTest.Add(description);
                }

                XElement execution = CreateElement("Execution");
                execution.SetAttributeValue("id", executionId);
                unitTest.Add(execution);

                // TestMethod element.
                XElement testMethod = CreateElement("TestMethod");
                testMethod.SetAttributeValue("codeBase", codeBase);
                testMethod.SetAttributeValue("adapterTypeName", adapterTypeName);
                testMethod.SetAttributeValue("className", className);
                testMethod.SetAttributeValue("name", name);
                unitTest.Add(testMethod);

                TestDefinitions.Add(unitTest);

                // TestEntry element.
                XElement testEntry = CreateElement("TestEntry");
                testEntry.SetAttributeValue("testId", testId);
                testEntry.SetAttributeValue("executionId", executionId);
                testEntry.SetAttributeValue("testListId", testListId);
                TestEntries.Add(testEntry);

                // UnitTestResult element.
                XElement unitTestResult = CreateElement("UnitTestResult");
                unitTestResult.SetAttributeValue("executionId", executionId);
                unitTestResult.SetAttributeValue("testId", testId);
                unitTestResult.SetAttributeValue("testName", name);
                unitTestResult.SetAttributeValue("computerName", computerName);

                TimeSpan duration = endTime.Subtract(startTime);
                unitTestResult.SetAttributeValue("duration", duration.ToString());

                unitTestResult.SetAttributeValue("startTime", ToDateString(startTime));
                unitTestResult.SetAttributeValue("endTime", ToDateString(endTime));
                unitTestResult.SetAttributeValue("testType", UnitTestTestTypeId);
                unitTestResult.SetAttributeValue("outcome", outcome.ToString());
                unitTestResult.SetAttributeValue("testListId", testListId.ToString());

                // Add any pending items
                foreach (XElement pending in _pendingElements)
                {
                    unitTestResult.Add(pending);
                }
                _pendingElements.Clear();

                Results.Add(unitTestResult);
            }

            /// <summary>
            /// Adds a WriteLine to the next result to be processed.
            /// </summary>
            /// <param name="line">The text to output.</param>
            public void AddPendingWriteLine(string line)
            {
                XElement xe = CreateElement("StdOut");
		if (line != null)
			xe.SetValue(line);
                AddPendingOutput(xe);
            }

            /// <summary>
            /// Adds an error message to the next result to be processed.
            /// </summary>
            /// <param name="message">The message.</param>
            public void AddPendingErrorMessage(string message)
            {
                XElement ei = CreateElement("ErrorInfo");
                XElement me = CreateElement("Message");
                me.SetValue(message);
                ei.Add(me);
                AddPendingOutput(ei);
            }

            /// <summary>
            /// Adds an Exception to the next result to be processed.
            /// </summary>
            /// <param name="e">The Exception object.</param>
            public void AddPendingException(Exception e)
            {
                XElement ei = CreateElement("ErrorInfo");
                XElement me = CreateElement("Message");
                me.SetValue(e.Message);
                ei.Add(me);

                XElement st = CreateElement("StackTrace");
                st.SetValue(e.StackTrace);
                ei.Add(st);

                AddPendingOutput(ei);
            }

            /// <summary>
            /// Adds pending output for the next result.
            /// </summary>
            /// <param name="element">The element to wrap in an Output element.</param>
            private void AddPendingOutput(XElement element)
            {
                XElement output = CreateElement("Output");
                output.Add(element);
                _pendingElements.Add(output);
            }

            /// <summary>
            /// Returns the GUID for a test list name. The result is stored 
            /// in memory.
            /// </summary>
            /// <param name="testListName">The test list name.</param>
            /// <returns>Returns the test list name guid.</returns>
            private string GetTestListGuid(string testListName)
            {
                Guid guid;
                if (!_testLists.TryGetValue(testListName, out guid))
                {
                    guid = Guid.NewGuid();
                    _testLists[testListName] = guid;
                }
                return guid.ToString();
            }
        }
    }
}
