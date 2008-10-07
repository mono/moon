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
			
		}

		public void EndRun ()
		{
			Console.WriteLine ("\n");

			ReportIgnoredTests ();
			ReportFailedTests ();
			ReportPassedTests ();
			
			ReportSummary ();
			//ReportAllTests ();
		}

		public void AddResult (Test test, TestResult result)
		{
			string state = string.Empty;
			ConsoleColor color;
			
			switch (result) {
			case TestResult.Pass:
				color = ConsoleColor.Green;
				state = run.VerboseLevel != VerboseLevel.None ? "P" : ".";
				break;
			case TestResult.Ignore:
				color = ConsoleColor.White;
				state = "I";
				break;
			case TestResult.Fail:
				color = ConsoleColor.Red;
				state = "F";
				break;
			case TestResult.KnownFailure:
				color = ConsoleColor.Blue;
				state = "K";
				break;
			case TestResult.UnexpectedPass:
				color = ConsoleColor.Cyan;
				state = "U";
				break;
			default:
				color = ConsoleColor.Green;
				state = "?";
				break;
			}

			if (!run.Runner.Driver.UseGdb)
				Console.ForegroundColor = color;
			
			if (run.VerboseLevel != VerboseLevel.None)
				Console.Write ("{0} ({1}):  ", test.InputFile, test.Id);
			Console.Write (state);
			if (run.VerboseLevel != VerboseLevel.None)
				Console.WriteLine ();
			
			if (!run.Runner.Driver.UseGdb)
				Console.ResetColor ();
			
		}

		private void ReportAllTests ()
		{
			Console.WriteLine ("========================= Test Summary =========================");
			foreach (Test t in run.Tests)
				AddResult (t, t.Result);
			Console.WriteLine ("Tests run:  {0}  {1} Pass, {2} Ignored,  {3} Failures,  {4} Known Failures",
					run.ExecutedTests.Count, run.PassedTests.Count, run.IgnoredTests.Count, run.FailedTests.Count, run.KnownFailures.Count);
			Console.WriteLine ("");
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
				Console.WriteLine ("Failed:  {0} ({2}) -- {1}", t.InputFile, t.FailedReason, t.Id);
				if (run.VerboseLevel >= VerboseLevel.ShowShockerLines) {
					Console.WriteLine ("  Log Lines:");
					foreach (string str in run.LoggingServer.GetTestLogLines (t.InputFileName)) {
						Console.WriteLine ("    {0}", str);
					}
				}
				/*
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
				*/
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

