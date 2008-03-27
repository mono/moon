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
		void RequestShutdown ();
	}

	public class LoggingServer : ITestLogger {

		private class TestLogData {

			private List<string> log_lines;
			private bool result_set;
			private TestResult result;
		
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
		}

		private object lock_object = new Object ();
		private Thread dbus_thread;

		private Dictionary<string, TestLogData> test_logs = new Dictionary<string, TestLogData> ();
		private bool shutdown_requested;

		public LoggingServer ()
		{
		}

		public void Start ()
		{
			dbus_thread = new Thread (new ThreadStart (DbusThreadWorker));
			dbus_thread.Start ();
		}

		public void Stop ()
		{
			dbus_thread.Abort ();
		}

		public void Log (string test, string level, string message)
		{
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

		public void RequestShutdown ()
		{
			lock (lock_object) {
				shutdown_requested = true;
			}
		}

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

		private void DbusThreadWorker ()
		{
			Bus bus = Bus.Session;

			string bus_name = "mono.moonlight.tests";
			ObjectPath path = new ObjectPath ("/mono/moonlight/tests/logger");

			if (!(bus.RequestName (bus_name) == RequestNameReply.PrimaryOwner)) {
				Console.Error.WriteLine ("Unable to request dbus bus name, results will not be logged correctly.");
				return;
			}

			Bus.Session.Register (bus_name, path, this);

			while (true)
				bus.Iterate ();	
		}

	
#if LOGGING_SERVER_STANDALONE
		public static void Main ()
		{
			Bus bus = Bus.Session;

			string bus_name = "mono.moonlight.tests";
			ObjectPath path = new ObjectPath ("/mono/moonlight/tests/logger");

			if (!(bus.RequestName (bus_name) == RequestNameReply.PrimaryOwner)) {
				Console.Error.WriteLine ("Unable to request bus name.");
				return;
			}


			LoggingServer ls = new LoggingServer ();
			Console.WriteLine ("UNIQUE NAME:  {0}", bus.UniqueName);
			Bus.Session.Register (bus_name, path, ls);

			Console.WriteLine ("name has owner:  {0}", bus.NameHasOwner (bus_name));
			Console.WriteLine ("getting object:  {0}", bus.GetObject <ILogger> (bus_name, path));

			while (true)
				bus.Iterate ();		
		}
#endif
	
	}

}
