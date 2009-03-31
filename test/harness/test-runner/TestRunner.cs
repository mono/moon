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
using System.IO;
using System.Threading;
using System.Reflection;
using System.Diagnostics;
using System.Collections;

using NDesk.DBus;
using org.freedesktop.DBus;


namespace MoonlightTests {

	//
	// Feed the server a list of tests and it will execute them all, raising
	// the test completed event each time a test completes.
	//

	// The test enumerator is moved from two places.  The Start method will enumerator through tests but
	// the actual agviewer process will also enumerate through tests by calling the GetNextTest method.
	// The only time that tests will be enumerated through Start's loop is during the first test, or if
	// agviewer crashes/is killed.

	[Interface ("mono.moonlight.tests.runner.ITestRunner")]
	public interface ITestRunner {
		void GetNextTest (out bool available, out string test_path, out int timeout);
		void TestComplete (string test, bool successful);

		void MarkTestAsCompleteAndGetNextTest (string test, bool successful, out bool next_available, out string next_test_path, out int timeout);

		void RequestShutdown ();
	}

	public class TestRunner : ITestRunner, IDbusService {
		private Driver driver;
		private IList tests;
		private bool run_complete;
		private IEnumerator tests_enumerator;
		private object tests_lock;

		private string working_dir;
		private string process_path;
		private ExternalProcess agviewer_process;

		private Test current_test;

		public TestRunner (IList tests, string working_dir, Driver driver)
		{
			this.driver = driver;
			this.tests = tests;
			this.working_dir = working_dir;

			tests_enumerator = tests.GetEnumerator ();
			tests_lock = new object ();
		}

		ObjectPath IDbusService.GetObjectPath ()
		{
			return new ObjectPath ("/mono/moonlight/tests/runner");
		}

		public void MarkTestAsCompleteAndGetNextTest (string test, bool successful, out bool next_available, out string next_test_path, out int timeout)
		{
			Log.WriteLine ("TestRunner.MarkTestAsCompleteAndGetNextTest ({0}, {1})", test, successful);
			TestComplete (test, successful);
			GetNextTest (out next_available, out next_test_path, out timeout);
			Log.WriteLine ("TestRunner.MarkTestAsCompleteAndGetNextTest () completed");
		}

		public void GetNextTest (out bool available, out string test_path, out int timeout)
		{
			Log.WriteLine ("TestRunner.GetNextTest () entering lock");
			lock (tests_lock) {
				if (!tests_enumerator.MoveNext ()) {
					Log.WriteLine ("TestRunner.GetNextTest (): run complete");
					run_complete = true;
					test_path = String.Empty;
					timeout = -1;
					available = false;
					return;
				}

				current_test = (Test) tests_enumerator.Current;
				Log.WriteLine ("TestRunner.GetNextTest () Calling OnBeginTest");
				OnBeginTest (current_test);
				Log.WriteLine ("TestRunner.GetNextTest () OnBeginTest complete");
				
				test_path = current_test.InputFile;
				if (!current_test.Remote)
					test_path = Path.GetFullPath (test_path);
				timeout = current_test.Timeout;

				available = true;
			}
			Log.WriteLine ("TestRunner.GetNextTest () done");
		}

		//
		// This will be called from the agviewer process when a test is completed
		//
		public void TestComplete (string test_path, bool successful)
		{
			if (test_path != Path.GetFileName (current_test.InputFile))
				Console.WriteLine ("Test complete path does not match current test path.  ({0} vs {1})", test_path, current_test.InputFile);
			Log.WriteLine ("TestRunner.TestComplete ({0}, {1})", test_path, successful);
			OnTestComplete (current_test, successful ? TestCompleteReason.Finished : TestCompleteReason.Timedout);
		}

		public void RequestShutdown ()
		{
			Log.WriteLine ("TestRunner.RequestShutdown ()");
			run_complete = true;
		}

		public void Start ()
		{
			 do {
				 EnsureAgviewerProcess ();

				// If agviewer process dies and the tests aren't finished 
				// mark the current test as a failure (since it somehow crashed the viewer)
				// and move to the next test in the queue.
				if (agviewer_process.ExitedEvent.WaitOne ()) {
					if (run_complete)
						break;
					if (current_test != null) {
						int exit_code = 256; 
						try {
							exit_code = agviewer_process.ExitCode;
						} catch (Exception ex) {
							Log.WriteLine ("Exception (ignored) while trying to get exit code: {0}", ex);
						}
						if (exit_code == 0) {
							Log.WriteLine ("Start (): agviewer decided to exit.");
							OnTestComplete (current_test, TestCompleteReason.Finished);
						} else {
							Log.WriteLine ("Start (): agviewer crashed, exit code: {0}", exit_code);
							OnTestComplete (current_test, TestCompleteReason.Crashed);
						}
					}
					agviewer_process.Kill ();
				}
			 } while (!run_complete);
		}

		public void Stop ()
		{
			agviewer_process.Kill ();
		}

		private void OnBeginTest (Test test)
		{
			test.Setup ();

			if (TestBeginEvent != null)
				TestBeginEvent (test);
		}

		private void OnTestComplete (Test test, TestCompleteReason reason)
		{
			if (test.CompleteReason != TestCompleteReason.Timedout)
				test.CompleteReason = reason;
			if (TestCompleteEvent != null)
				TestCompleteEvent (test, reason);
		}

		private void EnsureAgviewerProcess ()
		{
			if (agviewer_process != null && !agviewer_process.IsRunning) {
				agviewer_process.Kill ();
				agviewer_process = null;
			}

			if (agviewer_process == null) {
				string args = String.Format ("-working-dir {0}", Path.GetFullPath (working_dir));
				string valgrind_args;
				string gdb_args;
				
				if (driver != null && driver.UseValgrind) {
					valgrind_args = string.Format ("{0} {1} {2}", 
					                               System.Environment.GetEnvironmentVariable ("MOONLIGHT_VALGRIND_OPTIONS"), 
					                               GetProcessPath (), args);
					
					agviewer_process = new ExternalProcess ("valgrind", valgrind_args, -1);
				} else if (driver != null && driver.UseGdb) {
					gdb_args = string.Format ("--batch --eval-command=run --args {0} {1}", 
					                               GetProcessPath (), args);
					
					agviewer_process = new ExternalProcess ("gdb", gdb_args, -1);
				} else {
					agviewer_process = new ExternalProcess (GetProcessPath (), args, -1);
				}
				if (!string.IsNullOrEmpty (Environment.GetEnvironmentVariable ("MOON_PATH")))
					agviewer_process.EnvironmentVariables ["MONO_PATH"] = Environment.GetEnvironmentVariable ("MOON_PATH") + ":" + Environment.GetEnvironmentVariable ("MONO_PATH");
				agviewer_process.Run (false);
			}
		}

		private string GetProcessPath ()
		{
			if (!File.Exists (process_path))
				process_path = FindAgviewerRecursive (Directory.GetCurrentDirectory ());
			if (process_path == null)
				process_path = "agviewer";
			return process_path;
		}

		private string FindAgviewerRecursive (string cd)
		{
			string ag = Path.Combine (cd, "agviewer");
			if (File.Exists (ag))
				return ag;

			foreach (string dir in Directory.GetDirectories (cd)) {
				string res = FindAgviewerRecursive (dir);
				if (res != null)
					return res;
			}

			return null;
		}

		public Driver Driver {
			get {
				return driver;
			}
		}
		
		public TestCompleteEventHandler TestCompleteEvent;
		public TestBeginEventHandler TestBeginEvent;
	}
}

