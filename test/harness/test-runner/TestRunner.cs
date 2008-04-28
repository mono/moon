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

		private IList tests;
		private bool run_complete;
		private IEnumerator tests_enumerator;

		private string working_dir;
		private string process_path;
		private ExternalProcess agviewer_process;

		private Test current_test;

		public TestRunner (IList tests, string working_dir)
		{
			this.tests = tests;
			this.working_dir = working_dir;

			tests_enumerator = tests.GetEnumerator ();
		}

		ObjectPath IDbusService.GetObjectPath ()
		{
			return new ObjectPath ("/mono/moonlight/tests/runner");
		}

		public void MarkTestAsCompleteAndGetNextTest (string test, bool successful, out bool next_available, out string next_test_path, out int timeout)
		{
			TestComplete (test, successful);
			GetNextTest (out next_available, out next_test_path, out timeout);
		}

		public void GetNextTest (out bool available, out string test_path, out int timeout)
		{
			lock (tests_enumerator) {
				if (!tests_enumerator.MoveNext ()) {
					run_complete = true;
					test_path = String.Empty;
					timeout = -1;
					available = false;
					return;
				}

				current_test = (Test) tests_enumerator.Current;
				OnBeginTest (current_test);

				test_path = Path.GetFullPath (current_test.InputFile);
				timeout = current_test.Timeout;

				available = true;
			}
		}

		//
		// This will be called from the agviewer process when a test is completed
		//
		public void TestComplete (string test_path, bool successful)
		{
			if (test_path != Path.GetFileName (current_test.InputFile))
				Console.WriteLine ("Test complete path does not match current test path.  ({0} vs {1})", test_path, current_test.InputFile);
			OnTestComplete (current_test, successful ? TestCompleteReason.Finished : TestCompleteReason.Timedout);
		}

		public void RequestShutdown ()
		{
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
					agviewer_process.Kill ();
					if (run_complete)
						break;
					OnTestComplete (current_test, TestCompleteReason.Crashed);
				}
			 } while (!run_complete);
		}

		public void Stop ()
		{
			agviewer_process.Kill ();
		}

		private void OnBeginTest (Test test)
		{
			if (TestBeginEvent != null)
				TestBeginEvent (test);
		}

		private void OnTestComplete (Test test, TestCompleteReason reason)
		{
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
				agviewer_process = new ExternalProcess (GetProcessPath (), String.Format ("-working-dir {0}", Path.GetFullPath (working_dir)), -1);
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

		public TestCompleteEventHandler TestCompleteEvent;
		public TestBeginEventHandler TestBeginEvent;
	}
}

