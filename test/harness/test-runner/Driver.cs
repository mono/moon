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
using System.Collections;
using System.Collections.Generic;

using NDesk.Options;

namespace MoonlightTests {

	public class Driver {

		private TestRun test_run;
		private ArrayList tests = new ArrayList ();
	
		private ArrayList categories;
		private ArrayList fixtures;

		private VerboseLevel verbose_level = VerboseLevel.None;

		private ArrayList reports;

		private bool compare_to_moon = false;
		private bool run_known_failures = false;

		public VerboseLevel VerboseLevel {
			get { return verbose_level; }
			set { verbose_level = value; }
		}

		public bool CompareToMoon {
			get { return compare_to_moon; }
			set { compare_to_moon = value; }
		}

		public bool RunKnownFailures {
			get { return run_known_failures; }
			set { run_known_failures = value; }
		}

		public void SetCategories (string cat_str)
		{
			string [] cats = cat_str.Split (new char [] { ',' });

			if (categories == null)
				categories = new ArrayList ();

			foreach (string cat in cats)
				categories.Add (cat.Trim ());
		}

		public void SetFixtures (string fix_str)
		{
			string [] fixs = fix_str.Split (new char [] { ',' });

			if (fixtures == null)
				fixtures = new ArrayList ();

			foreach (string fix in fixs)
				fixtures.Add (fix.Trim ());
		}

		public bool TestIsInFixtureList (Test test)
		{
			if (fixtures == null)
				return false;

			return fixtures.Contains (test.Id);
		}

		public void LoadTests (string drtlist)
		{
			XmlDocument doc = new XmlDocument ();
			doc.Load (drtlist);
			XmlNodeList tests_xml = doc.SelectNodes ("/DRTList/Test");

			string base_directory = Path.GetDirectoryName (drtlist);

			foreach (XmlNode test in tests_xml) {
				Test t = Test.Create (base_directory, test);

				if (t == null) {
					Console.Error.WriteLine ("Unable to load test:  {0}", test);
					continue;
				}

				tests.Add (t);
			}
		}

		public int Run (string drtlist)
		{
			int return_code = 0;

			LoggingServer logging_server = new LoggingServer ();
			logging_server.Start ();

			test_run = new TestRun (Path.GetDirectoryName (drtlist), verbose_level, logging_server);

			LoadTests (drtlist);
			Screensaver.Inhibit ();

			ReportsBeginRun (test_run);

			foreach (Test test in tests) {
				
				if (categories != null && !test.IsInCategoryList (categories))
					continue;

				if (fixtures != null && !TestIsInFixtureList (test))
					continue;

				if (test.IsKnownFailure && !run_known_failures) {
					ReportsExecuting (test);
					RecordResult (test, TestResult.KnownFailure, false);
					ReportsAddResult (test, TestResult.KnownFailure);
					continue;
				}

				ReportsExecuting (test);
				TestResult result = test.Execute (compare_to_moon);
				
				if (logging_server.IsTestResultSet (test.InputFileName)) {
					result = logging_server.GetTestResult (test.InputFileName);
					if (result == TestResult.Fail)
						test.SetFailedReason ("Test LogResult set to FAIL");
				}

				RecordResult (test, result, true);
				ReportsAddResult (test, test.IsKnownFailure ? TestResult.KnownFailure : result);

				if (logging_server.HasShutdownBeenRequested ())
					break;
			}

			logging_server.Stop ();

			ReportsEndRun ();
			return return_code;
		}

		public void AddReport (IReport report)
		{
			if (reports == null)
				reports = new ArrayList ();
			reports.Add (report);
		}

		private void ReportsBeginRun (TestRun run)
		{
			if (reports != null) {
				foreach (IReport report in reports)
					report.BeginRun (run);
			}
		}

		private void ReportsEndRun ()
		{
			if (reports != null) {
				foreach (IReport report in reports)
					report.EndRun ();
			}
		}

		private void ReportsExecuting (Test test)
		{
			if (reports != null) {
				foreach (IReport report in reports)
					report.Executing (test);
			}
		}

		private void ReportsAddResult (Test test, TestResult result)
		{
			if (reports != null) {
				foreach (IReport report in reports)
					report.AddResult (test, result);
			}
		}

		private void RecordResult (Test test, TestResult result, bool test_executed)
		{
			switch (result) {
			case TestResult.Pass:
				test_run.PassedTests.Add (test);
				break;
			case TestResult.Ignore:
				test_run.IgnoredTests.Add (test);
				break;
			case TestResult.Fail:
				test_run.FailedTests.Add (test);
				break;
			case TestResult.KnownFailure:
				test_run.KnownFailures.Add (test);
				break;
			}

			if (test_executed)
				test_run.ExecutedTests.Add (test);
		}

		private void RunImageSanityChecks (string drtlist)
		{
			XmlDocument doc = new XmlDocument ();
			doc.Load (drtlist);
			XmlNodeList tests_xml = doc.SelectNodes ("/DRTList/Test");

			string base_directory = Path.GetDirectoryName (drtlist);

			foreach (XmlNode test in tests_xml) {
				Test t = Test.Create (base_directory, test);

				if (t == null) {
					Console.Error.WriteLine ("Unable to load test:  {0}", test);
					continue;
				}

				TestResult res = ImageCompare.Compare (t, t.MasterFile, t.MasterFile);

				Console.WriteLine ("Sanity Checking: {0}\n -- Result:{1}", t.MasterFile, res);
			}
			
			Environment.Exit (0);
		}

		private void GenerateMasterFiles (string drtlist)
		{
			XmlDocument doc = new XmlDocument ();
			doc.Load (drtlist);
			XmlNodeList tests_xml = doc.SelectNodes ("/DRTList/Test");

			string base_directory = Path.GetDirectoryName (drtlist);

			foreach (XmlNode test in tests_xml) {
				Test t = Test.Create (base_directory, test);

				if (t == null) {
					Console.Error.WriteLine ("Unable to load test:  {0}", test);
					continue;
				}

				t.Execute (false);

				if (!File.Exists (t.ResultFile)) {
					Console.Error.WriteLine ("Unable to generate Master file for: {0}.", t.InputFile);
					continue;
				}

				File.Copy (t.ResultFile, t.MoonMasterFile, true);
			}

			Environment.Exit (0);
		}

		public void ListKnownFailures (string drtlist)
		{
			LoadTests (drtlist);

			Console.WriteLine ("========================= Known Failures =========================");

			int i = 1;
			foreach (Test test in tests) {
				if (!test.IsKnownFailure)
					continue;
				Console.WriteLine ("{0}:  {1} -- {2}", i++, test.InputFile, test.KnownFailureReason);
			}

			Environment.Exit (0);
		}

		public static int Main (string [] args)
		{
			string drtlist = "drtlist.xml";

			Driver d = new Driver ();
			bool add_console_report = true;

			CheckEnvVars (d);

			OptionSet p = new OptionSet ();
			p.Add ("v|verbose", "Increase the verbosity level (None, ShowShockerLines, ShowStderr, ShowStdout).",
					delegate (string v) { d.VerboseLevel++; });
			p.Add ("fixture=", "Run a single test fixture with the specified name (names are case sensitive).",
					delegate (string v) { d.SetFixtures (v); });
			p.Add ("fixtures=", "Run a set of fixtures (names are case sensitive and seperated by commas).",
					delegate (string v) { d.SetFixtures (v); });
			p.Add ("categories=", "Only run the tests in the specified categories (comma separated list).",
					delegate (string v) { d.SetCategories (v); });
			p.Add ("drtlist=", "Specify the drtlist to be used for the test run.",
					delegate (string v) { drtlist = v; });
			p.Add ("agviewer=", "Specify the path to the agviewer binary (if this is not specified a temp agviewer will be created).",
					delegate (string v) { Agviewer.SetProcessPath (v); });
			p.Add ("no-console-report", "Do not report any of the test progress or results on the console.",
					delegate (string v) { add_console_report = false; });
			p.Add ("compare-to-moon", "Compare to the moon master files (generate moon masters before using this option).",
					delegate (string v) { d.CompareToMoon = true; });
			p.Add ("html-report|generate-html-report", "Generate an html report.",
					delegate (string v) { d.AddReport (new HtmlReport ()); });
			p.Add ("run-known-failures", "Run the tests that are marked as known failure.",
					delegate (string v) { d.RunKnownFailures = true; });
			p.Add ("list-known-failures", "List all the tests marked as known failure.",
					delegate (string v) { d.ListKnownFailures (drtlist); });
			p.Add ("generate-master-files", "Generate master files",
					delegate (string v) { d.GenerateMasterFiles (drtlist); });
			p.Add ("run-image-sanity-checks", "Run internal sanity checks on the image compare function.",
					delegate (string v) { d.RunImageSanityChecks (drtlist); });

			p.Parse (args);

			if (add_console_report)
				d.AddReport (new ConsoleReport ());

			d.AddReport (new XmlReport ());

			return d.Run (drtlist);
		}

		private static void CheckEnvVars (Driver d)
		{
			string fixture = Environment.GetEnvironmentVariable ("MOON_DRT_FIXTURE");
			string fixtures = Environment.GetEnvironmentVariable ("MOON_DRT_FIXTURES");
			string categories = Environment.GetEnvironmentVariable ("MOON_DRT_CATEGORIES");

			if ((fixture != null && fixture != String.Empty) && (fixtures != null && fixtures != String.Empty))
				Console.Error.WriteLine ("Warning, both MOON_DRT_FIXTURE and MOON_DRT_FIXTURES are set, these will be combined.");

			if (fixture != null && fixture != String.Empty)
				d.SetFixtures (fixture);
			if (fixtures != null && fixtures != String.Empty)
				d.SetFixtures (fixtures);
			if (categories != null && categories != String.Empty)
				d.SetCategories (categories);
		}
	}
}
