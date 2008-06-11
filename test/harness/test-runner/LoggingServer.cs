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
using System.Xml;
using System.Threading;
using System.Diagnostics;
using System.Collections.Generic;

using NDesk.DBus;
using org.freedesktop.DBus;


namespace MoonlightTests {

	[Interface ("mono.moonlight.tests.logger.ITestLogger")]
	public interface ITestLogger {
		void Log (string test, string level, string message);
		void LogResult (string test, int result);
	}

#if !LOGGING_SERVER_STANDALONE
	public class LoggingServer : ITestLogger, IDbusService {
		StandaloneServer standalone = new StandaloneServer ();

		private class TestLogData {

			private List<string> log_lines;
			private bool result_set;
			private TestResult result;
			private bool test_complete;
		
			public bool IsResultSet {
				get { return result_set; }
			}

			public TestResult Result {
				get {
					if (!result_set)
						throw new Exception ("Result not set");
					return result;
				}
			}

			public bool IsTestComplete {
				get { return test_complete; }
			}

			public void AddLogLine (string level, string message)
			{
				if (log_lines == null)
					log_lines = new List<string> ();
				log_lines.Add (String.Concat (level, ": ", message));
			}

			public string [] GetLogLines ()
			{
				if (log_lines == null)
					return new string [0];
				return log_lines.ToArray ();
			}

			public void SetTestResult (TestResult result)
			{
				if (result == TestResult.Pass && result_set && this.result == TestResult.Fail) {
					AddLogLine ("Error", "You can not change the result of a test from fail to pass.");
					return;
				}

				this.result = result;
				result_set = true;
			}

			public void TestComplete (bool successful)
			{
				test_complete = true;
				if (!successful)
					SetTestResult (TestResult.Fail);
			}
		}

		private object lock_object = new Object ();
		private Thread dbus_thread;

		private Dictionary<string, TestLogData> test_logs = new Dictionary<string, TestLogData> ();
		private bool shutdown_requested;

		private static AutoResetEvent test_complete_event = new AutoResetEvent (false);
		private static AutoResetEvent next_test_ready_event = new AutoResetEvent (false);

		public LoggingServer ()
		{
		}

		ObjectPath IDbusService.GetObjectPath ()
		{
			return new ObjectPath ("/mono/moonlight/tests/logger");
		}

		public void Log (string test, string level, string message)
		{
			if (MoonlightTests.Driver.LogToStdout)
				standalone.Log (test, level, message);

			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else {
					tld = new TestLogData ();
					test_logs [test] = tld;
				}

				tld.AddLogLine (level, message);
			}
		}

		public void LogResult (string test, int result)
		{
			if (Driver.LogToStdout)
				standalone.LogResult (test, result);

			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else {
					tld = new TestLogData ();
					test_logs [test] = tld;
				}

				
				tld.SetTestResult (TestResultFromInt (result));
			}
		}

		public void TestComplete (string test, bool successful)
		{
			if (Driver.LogToStdout)
				standalone.TestComplete (test, successful);

			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else {
					tld = new TestLogData ();
					test_logs [test] = tld;
				}
				tld.TestComplete (successful);

				Console.WriteLine ("test complete:  {0}", test);
				test_complete_event.Set ();
			}
		}

		public bool IsTestComplete (string test)
		{
			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else
					return false;

				return tld.IsTestComplete;
			}
		}

		/*
		internal bool WaitForTestToComplete (string test, ExternalProcess process, int timeout)
		{
			while (!IsTestComplete (test)) {
				WaitHandle [] handles = new WaitHandle [2];
				handles [0] = process.ExitedEvent;
				handles [1] = test_complete_event;

				if (WaitHandle.WaitAny (handles, timeout, false) == WaitHandle.WaitTimeout) {
					Console.WriteLine ("test did not complete correctly.  We timed out waiting for it to complete ({0}ms).", timeout);
					return false;
				}
				Console.WriteLine ("got reset event");
			}

			return true;
		}
		*/

		public bool IsTestResultSet (string test)
		{
			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else
					return false;

				return tld.IsResultSet;
			}
		}

		public TestResult GetTestResult (string test)
		{
			lock (lock_object) {
				TestLogData tld = null;
				if (test_logs.ContainsKey (test))
					tld = test_logs [test] as TestLogData;
				else
					throw new Exception ("Attempting to get log result for test that does not exist.  Call IsTestResultSet first!");
				return tld.Result;
			}
		}

		public string [] GetTestLogLines (string test)
		{
			lock (lock_object) {
				if (!test_logs.ContainsKey (test))
					return new string [0];
				TestLogData tld = test_logs [test] as TestLogData;
				return tld.GetLogLines ();
			}
		}

		public bool HasShutdownBeenRequested ()
		{
			lock (lock_object) {
				return shutdown_requested;
			}
		}

		private TestResult TestResultFromInt (int value)
		{
			if (value == 1)
				return TestResult.Pass;
			return TestResult.Fail;
		}
	}
#endif

		public class StandaloneServer : ITestLogger {
			public void Log (string test, string level, string message)
			{
				if (level == "Error") {
					Console.ForegroundColor = ConsoleColor.Red;
				} else if (level == "Warning") {
					Console.ForegroundColor = ConsoleColor.Yellow;
				} else {
					Console.ForegroundColor = ConsoleColor.Blue;
				}
				Console.WriteLine ("{0}: {1}, {2}", test, level, message);
				Console.ResetColor ();
			}

			public void LogResult (string test, int result)
			{
				if (result == -1) {
					Console.ForegroundColor = ConsoleColor.Red;
					Console.WriteLine ("{0}: Test result set to Failed", test);
				} else if (result == 1) {
					Console.ForegroundColor = ConsoleColor.White;
					Console.WriteLine ("{0}: Test result set to Passed", test);
				} else {
					Console.WriteLine ("{0}: Test result set to {1}", test, result);
				}
				Console.ResetColor ();
			}

			public void RequestShutdown ()
			{
				Console.WriteLine ("RequestShutdown ()");
			}
			public void TestComplete (string test, bool successful)
			{
				Console.WriteLine ("{0}: TestComplete");
			}

	
#if LOGGING_SERVER_STANDALONE
		public static void Main ()
		{
			Bus bus = Bus.Session;

			string bus_name = "mono.moonlight.tests";
			ObjectPath path = new ObjectPath ("/mono/moonlight/tests/logger");

			if (!(bus.RequestName (bus_name, NameFlag.ReplaceExisting | NameFlag.AllowReplacement) == RequestNameReply.PrimaryOwner)) {
				Console.Error.WriteLine ("Unable to request bus name.");
				return;
			}


			StandaloneServer ls = new StandaloneServer ();
			Console.WriteLine ("UNIQUE NAME:  {0}", bus.UniqueName);
			Bus.Session.Register (bus_name, path, ls);

			Console.WriteLine ("name has owner:  {0}", bus.NameHasOwner (bus_name));
			Console.WriteLine ("getting object:  {0}", bus.GetObject <ITestLogger> (bus_name, path));

			while (true)
				bus.Iterate ();		
		}
#endif
	
	}

}
