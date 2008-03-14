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

namespace MoonlightTests {

	public class ConsoleReport : IReport {

		private TestRun run;

		public void BeginRun (TestRun run)
		{
			this.run = run;

			Console.WriteLine ("Running tests...");
		}

		public void Executing (Test test)
		{
			if (run.VerboseLevel != VerboseLevel.None)
				Console.Write ("{0}:  ", test.InputFile);
		}

		public void EndRun ()
		{
			Console.WriteLine ("\n");

			ReportIgnoredTests ();
			ReportFailedTests ();
			ReportPassedTests ();

			ReportSummary ();
		}

		public void AddResult (Test test, TestResult result)
		{
			switch (result) {
			case TestResult.Pass:
				Console.Write (run.VerboseLevel != VerboseLevel.None ? "P" : ".");
				break;
			case TestResult.Ignore:
				Console.Write ("I");
				break;
			case TestResult.Fail:
				Console.Write ("F");
				break;
			case TestResult.KnownFailure:
				Console.Write ("K");
				break;
			}

			if (run.VerboseLevel != VerboseLevel.None)
				Console.WriteLine ();
		}

		private void ReportIgnoredTests ()
		{
			if (run.IgnoredTests.Count < 1)
				return;

			Console.WriteLine ("========================= Ignored Tests =========================");
			foreach (Test t in run.IgnoredTests) {
				Console.WriteLine ("Ignored:  {0} -- {1}", t.InputFile, t.IgnoreReason);
			}

			Console.WriteLine ("\n");
		}

		private void ReportFailedTests ()
		{
			if (run.FailedTests.Count < 1)
				return;

			Console.WriteLine ("========================= Failed Tests ==========================");
			foreach (Test t in run.FailedTests) {
				Console.WriteLine ("Failed:  {0} -- {1}", t.InputFile, t.FailedReason);
				if (run.VerboseLevel >= VerboseLevel.ShowShockerLines) {
					Console.WriteLine ("  Log Lines:");
					foreach (string str in run.LoggingServer.GetTestLogLines (t.InputFile)) {
						Console.WriteLine ("    {0}", str);
					}
				}
				if (run.VerboseLevel >= VerboseLevel.ShowStderr) {
					Console.WriteLine ("  Stderr:");
					foreach (string str in t.Stderr.Split (Environment.NewLine [0])) {
						Console.WriteLine ("    {0}", str);
					}
				}
				if (run.VerboseLevel >= VerboseLevel.ShowStdout) {
					Console.WriteLine ("  Stdout:");
					foreach (string str in t.Stdout.Split (Environment.NewLine [0])) {
						Console.WriteLine ("    {0}", str);
					}
				}
			}

			Console.WriteLine ("\n");
		}

		private void ReportPassedTests ()
		{
			// TODO:  This should only happen when we run-known-failures-only
		}

		private void ReportSummary ()
		{
			Console.WriteLine ("Tests run:  {0}  {1} Pass, {2} Ignored,  {3} Failures,  {4} Known Failures",
					run.ExecutedTests.Count, run.PassedTests.Count, run.IgnoredTests.Count, run.FailedTests.Count, run.KnownFailures.Count);
		}

	}
}

