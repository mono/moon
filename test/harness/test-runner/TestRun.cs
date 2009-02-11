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
using System.Threading;
using System.Collections;
using System.Collections.Generic;

namespace MoonlightTests {

	public class TestRun {

		private DateTime start_time;

		private string base_dir;
		private VerboseLevel verbose_level;
		private LoggingServer logging_server;
		private TestRunner runner;

		private Thread worker_thread;
		private AutoResetEvent tests_pending_event = new AutoResetEvent (false);
		private ManualResetEvent tests_finished_event = new ManualResetEvent (false);

		private Queue<Test> process_tests_queue = new Queue<Test> ();
		
		private List<Test> tests;
		private List<IReport> reports;

		private List<Test> executed_tests = new List<Test> ();
		private List<Test> passed_tests = new List<Test> ();
		private List<Test> ignored_tests = new List<Test> ();
		private List<Test> failed_tests = new List<Test> ();
		private List<Test> known_failures = new List<Test> ();

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

		public List<Test> Tests {
			get { return tests; }
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

		public List<Test> ExecutedTests {
			get { return executed_tests; }
		}

		public List<Test> PassedTests {
			get { return passed_tests; }
		}

		public List<Test> IgnoredTests {
			get { return ignored_tests; }
		}

		public List<Test> FailedTests {
			get { return failed_tests; }
		}

		public List<Test> KnownFailures {
			get { return known_failures; }
		}

		public TestRunner Runner {
			get {
				return runner;
			}
		}
		
		public int Run ()
		{
			runner.TestBeginEvent += new TestBeginEventHandler (TestBegin);
			runner.TestCompleteEvent += new TestCompleteEventHandler (TestComplete);

			reports.ForEach (delegate (IReport report) { report.BeginRun (this); });

			worker_thread = new Thread (ProcessTestsWorker);
			worker_thread.IsBackground = true;
			worker_thread.Start ();

			runner.Start ();

			Log.WriteLine ("TestRun.Run (): runner started, waiting for finished event.");
			
			tests_finished_event.WaitOne ();

			Log.WriteLine ("TestRun.Run (): got finished event.");
			
			reports.ForEach (delegate (IReport report) { report.EndRun (); });

			return FailedTests.Count;
		}

		private void TestBegin (Test test)
		{
			reports.ForEach (delegate (IReport report) { report.Executing (test); });
		}

		private void TestComplete (Test test, TestCompleteReason reason)
		{
			QueueTestForProcessing (test);
		}

		private void QueueTestForProcessing (Test test)
		{
			lock (process_tests_queue) {
				process_tests_queue.Enqueue (test);
			}

			tests_pending_event.Set ();
		}

		private void ProcessTestsWorker ()
		{
			while (true) {
				tests_pending_event.WaitOne ();

				while (true) {

					Test test = null;
					lock (process_tests_queue) {
						if (process_tests_queue.Count == 0) {
							Log.WriteLine ("TestRun.ProcessTestsWorker (): no more tests.");
							break;
						}
						test = process_tests_queue.Dequeue ();
					}

					tests_finished_event.Reset ();

					Log.WriteLine ("TestRun.ProcessTestsWorker (): processing test {0}", test.Id);
					
					ProcessTest (test);
				}

				Log.WriteLine ("TestRun.ProcessTestsWorker (): Notify the main thread that we are out of tests");
				tests_finished_event.Set ();
			}
		}

		private void ProcessTest (Test test)
		{
			TestResult result = TestResult.Pass;

			if (test.CompleteReason == TestCompleteReason.Finished) {

				if (logging_server.IsTestResultSet (test.InputFileName)) {
					result = logging_server.GetTestResult (test.InputFileName);
					if (result == TestResult.Fail)
						test.SetFailedReason ("Test LogResult set to FAIL");
				}
				
				if (result == TestResult.Pass)
					result = test.ComputeImageCompareResult ();
			} else {
				test.SetFailedReason (String.Format ("Test did not complete properly ({0})", test.CompleteReason));
				result = TestResult.Fail;
			}
			
			if (result == TestResult.Pass && test.UnitTest)
				result = test.ProcessUnitTestResult ();
			
			if (test.IsKnownFailure)
				result = (result == TestResult.Pass) ? TestResult.UnexpectedPass : TestResult.KnownFailure;

			RecordResult (test, result);
			reports.ForEach (delegate (IReport report) { report.AddResult (test, result); });

			test.Teardown ();
		}

		public void RecordResult (Test test, TestResult result)
		{
			switch (result) {
			case TestResult.Pass:
			case TestResult.UnexpectedPass:
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
			test.Result = result;
		}

	}
}

