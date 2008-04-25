// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2007-2008 Novell, Inc.
//
// Authors:
//	Jackson Harper (jackson@ximian.com)
//


using System;
using System.Collections;
using System.Collections.Generic;

namespace MoonlightTests {

	public class TestRun {

		private DateTime start_time;

		private string base_dir;
		private VerboseLevel verbose_level;
		private LoggingServer logging_server;
		private TestRunner runner;

		private List<Test> tests;
		private List<IReport> reports;

		private ArrayList executed_tests = new ArrayList ();
		private ArrayList passed_tests = new ArrayList ();
		private ArrayList ignored_tests = new ArrayList ();
		private ArrayList failed_tests = new ArrayList ();
		private ArrayList known_failures = new ArrayList ();

		public TestRun (string base_dir, VerboseLevel verbose_level, List<Test> tests, List<IReport> reports, LoggingServer logging_server, TestRunner runner)
		{
			start_time = DateTime.Now;

			this.base_dir = base_dir;
			this.verbose_level = verbose_level;

			this.tests = tests;
			this.reports = reports;

			this.logging_server = logging_server;
			this.runner = runner;
		}

		public string BaseDirectory {
			get { return base_dir; }
		}

		public VerboseLevel VerboseLevel {
			get { return verbose_level; }
		}

		public LoggingServer LoggingServer {
			get { return logging_server; }
		}

		public DateTime StartTime {
			get { return start_time; }
		}

		public IList ExecutedTests {
			get { return executed_tests; }
		}

		public IList PassedTests {
			get { return passed_tests; }
		}

		public IList IgnoredTests {
			get { return ignored_tests; }
		}

		public IList FailedTests {
			get { return failed_tests; }
		}

		public IList KnownFailures {
			get { return known_failures; }
		}

		public int Run ()
		{
			runner.TestBeginEvent += new TestBeginEventHandler (TestBegin);
			runner.TestCompleteEvent += new TestCompleteEventHandler (TestComplete);

			reports.ForEach (delegate (IReport report) { report.BeginRun (this); });
			runner.Start ();
			reports.ForEach (delegate (IReport report) { report.EndRun (); });

			return 0;
		}

		private void TestBegin (Test test)
		{
			test.Setup ();

			reports.ForEach (delegate (IReport report) { report.Executing (test); });
		}

		private void TestComplete (Test test, TestCompleteReason reason)
		{
			TestResult result = TestResult.Pass;

			if (reason == TestCompleteReason.Finished) {

				if (logging_server.IsTestResultSet (test.InputFileName)) {
					result = logging_server.GetTestResult (test.InputFileName);
					if (result == TestResult.Fail)
						test.SetFailedReason ("Test LogResult set to FAIL");
				}
				
				if (result == TestResult.Pass)
					result = test.ComputeImageCompareResult ();
			} else {
				test.SetFailedReason (String.Format ("Test did not complete properly ({0})", reason));
				result = TestResult.Fail;
			}

			if (test.IsKnownFailure)
				result = TestResult.KnownFailure;

			RecordResult (test, result);
			reports.ForEach (delegate (IReport report) { report.AddResult (test, result); });

			test.Teardown ();
		}

		public void RecordResult (Test test, TestResult result)
		{
			switch (result) {
			case TestResult.Pass:
				PassedTests.Add (test);
				break;
			case TestResult.Ignore:
				IgnoredTests.Add (test);
				break;
			case TestResult.Fail:
				FailedTests.Add (test);
				break;
			case TestResult.KnownFailure:
				KnownFailures.Add (test);
				break;
			}

			ExecutedTests.Add (test);
		}

	}
}

