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

namespace MoonlightTests {

	internal class ExternalProcess {

		private Process process;
		private bool process_running;
		private Thread stdout_thread;
		private Thread stderr_thread;

		private string process_path;
		private string arguments;
		private int timeout;

		private string stdout;
		private string stderr;
		private int exit_code = -1;

		private bool process_timed_out;

		public AutoResetEvent ExitedEvent = new AutoResetEvent (false);

		public ExternalProcess (string process_path, string arguments, int timeout)
		{
			this.process_path = process_path;
			this.arguments = arguments;
			this.timeout = timeout;
		}

		public string Stdout {
			get { return stdout; }
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
			process = new Process ();

			process.StartInfo.FileName = process_path;
			process.StartInfo.Arguments = arguments;

			process.StartInfo.CreateNoWindow = true;
			process.StartInfo.UseShellExecute = false;
			process.StartInfo.RedirectStandardOutput = true;
			process.StartInfo.RedirectStandardError = true;

			stdout_thread = new Thread (delegate () { stdout = process.StandardOutput.ReadToEnd (); });
			stderr_thread = new Thread (delegate () { stderr = process.StandardError.ReadToEnd (); });
			stdout_thread.IsBackground = true;
			stderr_thread.IsBackground = true;

			try {
				process.EnableRaisingEvents = true;
				process_running = process.Start ();

				process.Exited += delegate (object sender, EventArgs e)
				{
					exit_code = process.ExitCode;
					process_running = false;
					ExitedEvent.Set ();
				}; 

				stdout_thread.Start ();
				stderr_thread.Start ();

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

			stdout_thread.Abort ();
			stderr_thread.Abort ();
		}

		public void ResetIO ()
		{
			stdout = null;
			stderr = null;
		}
	}
}


