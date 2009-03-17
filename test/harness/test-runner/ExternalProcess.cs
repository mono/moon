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
using System.Text;

namespace MoonlightTests {

	internal class ExternalProcess : IDisposable {

		private Process process;
		private bool process_running;
		private Thread stderr_thread;
		private bool redirect_stdout;

		private string process_path;
		private string arguments;
		private int timeout;

		private StringBuilder stdout;
		private string stderr;
		private int exit_code = -1;

		private bool process_timed_out;

		public AutoResetEvent ExitedEvent = new AutoResetEvent (false);

		public System.Collections.Specialized.StringDictionary EnvironmentVariables {
			get {
				if (process == null)
					process = new Process ();
				return process.StartInfo.EnvironmentVariables;
			}
		}
		
		public void Dispose ()
		{
			if (process != null)
				process.Dispose ();
		}
		
		public ExternalProcess (string process_path, string arguments, int timeout)
		{
			this.process_path = process_path;
			this.arguments = arguments;
			this.timeout = timeout;
		}

		public bool RedirectStdOut {
			get { return redirect_stdout; }
			set { redirect_stdout = value; }
		}
		
		public string Stdout {
			get { return stdout == null ? null : stdout.ToString (); }
		}

		public string Stderr {
			get { return stderr; }
		}

		public int ExitCode {
			get { return exit_code; }
		}

		public bool ProcessTimedOut {
			get { return process_timed_out; }
		}

		public bool IsRunning {
			get { return process_running; }
		}

		public void Run (bool wait)
		{
			if (process == null)
				process = new Process ();

			process.StartInfo.FileName = process_path;
			process.StartInfo.Arguments = arguments;

			process.StartInfo.CreateNoWindow = true;
			process.StartInfo.UseShellExecute = false;

			if (Driver.SwallowStreams) {
				process.StartInfo.RedirectStandardError = true;
	
				stderr_thread = new Thread (delegate () { stderr = process.StandardError.ReadToEnd (); });
				stderr_thread.IsBackground = true;
			}

			if (Driver.SwallowStreams || RedirectStdOut) {
				process.StartInfo.RedirectStandardOutput = true;
				stdout = new StringBuilder ();
				process.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) {
					try {
						if (e.Data == null)
							return;
						stdout.AppendLine (e.Data);
						Console.WriteLine (e.Data);
					} catch (Exception ex) {
						Console.WriteLine ("Stdout tee for '{0}' threw an exception: {1}", process_path, ex.Message);
						Console.WriteLine (ex.StackTrace);
					}
				};
			}
			
			try {
				process.EnableRaisingEvents = true;
				process_running = process.Start ();

				process.Exited += delegate (object sender, EventArgs e)
				{
					exit_code = process.ExitCode;
					process_running = false;
					ExitedEvent.Set ();
				}; 

				if (Driver.SwallowStreams)
					stderr_thread.Start ();

				if (Driver.SwallowStreams || RedirectStdOut)
					process.BeginOutputReadLine ();

				if (wait && !process.WaitForExit (timeout))
					process_timed_out = true;
				if (wait && !process_timed_out)
					exit_code = process.ExitCode;
			} catch (Exception e) {
				Console.Error.WriteLine ("Unable to start {0} process:", process_path);
				Console.Error.WriteLine (e);

				process_running = false;
			} finally {
				if (wait)
					Kill ();
			}
		}

		public void Kill ()
		{
			if (process_running && !process.HasExited)
				process.Kill ();

			process.Dispose ();

			if (Driver.SwallowStreams) {
				stderr_thread.Abort ();
			}
		}

		public void ResetIO ()
		{
			stdout = null;
			stderr = null;
		}
	}
}


