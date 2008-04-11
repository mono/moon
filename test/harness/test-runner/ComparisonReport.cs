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
using System.Text;
using System.Xml.XPath;
using System.Resources;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;


namespace MoonlightTests {

	public class ComparisonReport : IReport {

		private static readonly double GraphHeight = 480.0;
		private static readonly double GraphWidth = 640.0;

		private TestRun this_run;
		private string [] run_directories;
		private List<TestCompareData> compare_data = new List<TestCompareData> ();
		private List<TestRunData> test_runs;
		private List<TestRunData> complete_test_runs;

		private class TestRunData {
			public DateTime StartTime;
			public int NumTestsExecuted;
			public int NumTestsPassed;
			public int NumTestsFailed;
			public int NumTestsIgnored;
			public int NumTestsKnownFailure;
		}

		private class TestCompareData {
			public string InputFile;
			public DateTime CurrentRunTime;
			public TestResult CurrentResult;
			public string CurrentResultFile;

			public TestResult PreviousResult;
			public string PreviousResultFile;
			public string PreviousResultRun;
		}

		public void BeginRun (TestRun run)
		{
			this_run = run;

			run_directories = Directory.GetDirectories (XmlReport.TestRunDirectoryName);
			Array.Sort (run_directories, new DirectoryNamesByDateComparer ());

			test_runs = new List<TestRunData> ();
			foreach (string dir in run_directories)
				LoadRunData (dir);
		}

		public void EndRun ()
		{
			BuildCompleteTestRunsList ();
			WriteHtml ();
		}

		public void Executing (Test test)
		{
		}

		public void AddResult (Test test, TestResult result)
		{
			TestCompareData tcd = new TestCompareData ();
			tcd.InputFile = test.InputFile;
			tcd.CurrentRunTime = this_run.StartTime;
			tcd.CurrentResult = result;
			tcd.CurrentResultFile = test.ResultFile;

			SetPreviousResultData (test, tcd);
			compare_data.Add (tcd);
		}

		private void LoadRunData (string dir)
		{
			string run_file = Path.Combine (dir, XmlReport.TestRunFileName);

			if (!File.Exists (run_file))
				return;

			XPathDocument doc = new XPathDocument (run_file);
			XPathNavigator nav = doc.CreateNavigator ();
			XPathExpression expr = nav.Compile ("/TestRun");
			XPathNodeIterator iterator = nav.Select (expr);

			if (!iterator.MoveNext ())
				return;

			TestRunData test_run = new TestRunData ();
			test_run.StartTime = DateTime.ParseExact (GetBaseDirectory (dir), "yyyy-MM-dd-hh-mm", System.Globalization.CultureInfo.InvariantCulture);
			test_run.NumTestsExecuted = Int32.Parse (iterator.Current.GetAttribute ("ExecutedTests", String.Empty));
			test_run.NumTestsPassed = Int32.Parse (iterator.Current.GetAttribute ("PassedTests", String.Empty));
			test_run.NumTestsFailed = Int32.Parse (iterator.Current.GetAttribute ("FailedTests", String.Empty));
			test_run.NumTestsIgnored = Int32.Parse (iterator.Current.GetAttribute ("IgnoredTests", String.Empty));
			test_run.NumTestsKnownFailure = Int32.Parse (iterator.Current.GetAttribute ("KnownFailures", String.Empty));

			test_runs.Add (test_run);
		}

		private void SetPreviousResultData (Test test, TestCompareData data)
		{
			foreach (string run_dir in run_directories) {
				string run_file = Path.Combine (run_dir, XmlReport.TestRunFileName);

				if (!File.Exists (run_file))
					continue;

				XPathDocument doc = new XPathDocument (run_file);
				XPathNavigator nav = doc.CreateNavigator ();
				XPathExpression expr = nav.Compile (String.Format ("/TestRun/Test[@InputFile='{0}']", test.InputFile));
				XPathNodeIterator iterator = nav.Select (expr);

				if (!iterator.MoveNext ())
					continue;

				data.PreviousResult = (TestResult) Enum.Parse (typeof (TestResult), iterator.Current.GetAttribute ("TestResult", String.Empty));
				data.PreviousResultFile = iterator.Current.GetAttribute ("ResultFile", String.Empty);
				data.PreviousResultRun = run_dir;
				break;
			}
		}

		private class DirectoryNamesByDateComparer : IComparer {

			public int Compare (object a, object b)
			{
				string left = a as string;
				string right = b as string;

				if (a == null || b == null || !Directory.Exists (left) || !Directory.Exists (right))
					throw new Exception (String.Format ("Invalid directory supplied  {0} {1}", left, right));

				return DateTime.Compare (Directory.GetCreationTime (left), Directory.GetCreationTime (right));
			}
		}

		//
		// Build a list of test runs that were not partial runs
		//
		private void BuildCompleteTestRunsList ()
		{
			int min = (int) (this_run.ExecutedTests.Count * 0.85);
			complete_test_runs = new List<TestRunData> ();

			foreach (TestRunData run in test_runs) {
				if (run.NumTestsExecuted < min)
					continue;
				complete_test_runs.Add (run);
			}

			TestRunData t = new TestRunData ();
			t.StartTime = this_run.StartTime;
			t.NumTestsExecuted = this_run.ExecutedTests.Count;
			t.NumTestsPassed = this_run.PassedTests.Count;
			t.NumTestsFailed = this_run.FailedTests.Count;
			t.NumTestsIgnored = this_run.IgnoredTests.Count;
			t.NumTestsKnownFailure = this_run.KnownFailures.Count;

			complete_test_runs.Add (t);
			
		}

		private void WriteHtml ()
		{
			StreamReader reader = new StreamReader (Assembly.GetExecutingAssembly ().GetManifestResourceStream ("comparison_report.html"));
			string html = reader.ReadToEnd ();

			html = html.Replace ("$GRAPH_XAML", BuildCompareGraph ());
			html = html.Replace ("$COMPARE_HTML", BuildCompareHtml ());

			using (StreamWriter writer = new StreamWriter ("comparison_report.html", false)) {
				writer.Write (html);
			}
		}

		private string BuildCompareHtml ()
		{
			StringBuilder html = new StringBuilder ();

			TestResult prev;
			string prev_dir;
			bool regressions_header_added = false;
			foreach (TestCompareData tcd in compare_data) {
				if (tcd.CurrentResult != TestResult.Fail || tcd.PreviousResult == TestResult.Fail)
					continue;
				if (!regressions_header_added) {
					html.AppendLine ("<table border=\"1\">");
					html.AppendLine ("<tr><td colspan=\"2\"><h1>Regressions</h1></td></tr>");
					html.AppendLine ("<tr><td>Current Result</td><td>Previous Result</td></tr>");
					regressions_header_added = true;
				}
				html.AppendFormat ("<tr><td colspan=\"2\">{0}</td></tr>", Path.GetFileName (tcd.InputFile));
				html.AppendFormat ("<tr><td><img src=\"{0}\"></td><td><img src=\"{1}\"></td></tr>", GetCurrentResultFile (tcd), tcd.PreviousResultFile);
			}

			if (regressions_header_added)
				html.Append ("</table>");

			return html.ToString ();
		}

		private string GetCurrentResultFile (TestCompareData tcd)
		{
			string dir = Path.Combine (XmlReport.TestRunDirectoryName, tcd.CurrentRunTime.ToString ("yyyy-MM-dd-hh-mm"));

			return Path.Combine (dir, Path.GetFileName (tcd.CurrentResultFile));
		}
		
		/*
		private bool FindPrevResult (Test test, out TestResult result, out string dir)
		{
			foreach (string d in run_directories) {
				string run_file = Path.Combine (d, XmlReport.TestRunFileName);

				if (!File.Exists (run_file))
					return;

				XPathDocument doc = new XPathDocument (run_file);
				XPathNavigator nav = doc.CreateNavigator ();
				XPathExpression expr = nav.Compile ("/TestRun");
				XPathNodeIterator iterator = nav.Select (expr);

				if (!iterator.MoveNext ())
					continue;

				
			}

			return false;
		}
		*/

		private string BuildCompareGraph ()
		{
			int x_max = GetXMax ();
			int y_max = GetYMax ();
			double x_scale = GetXScale (x_max);
			double y_scale = GetYScale (y_max);
			int x_axis_width = 25;
			int title_height = 25;

			StringBuilder graph = new StringBuilder ();
			StringBuilder title = CreateTitleCanvas (x_axis_width, 0);
			StringBuilder x_axis = CreateXAxisCanvas (0, title_height, y_max, y_scale);

			StringBuilder chart = CreateChartCanvas (x_axis_width, title_height, x_max, y_max, x_scale, y_scale);

			graph.AppendLine ("<Canvas>");
			graph.AppendLine (title.ToString ());
			graph.AppendLine (x_axis.ToString ());
//			graph.AppendLine (y_axis.ToString ());
			graph.AppendLine (chart.ToString ());
			graph.AppendLine ("</Canvas>");


			return graph.ToString ();
		}

		private StringBuilder CreateTitleCanvas (int pos_x, int pos_y)
		{
			StringBuilder res = new StringBuilder ();

			res.AppendFormat ("<Canvas  Canvas.Left=\"{0}\" Canvas.Top=\"{1}\" Width=\"{0}\">", pos_x, pos_y, GraphWidth);
			res.AppendLine ("<TextBlock Text=\"Test Run Results\" />");
			res.AppendLine ("</Canvas>");

			return res;
		}

		private StringBuilder CreateXAxisCanvas (int pos_x, int pos_y, int y_max, double y_scale)
		{
			StringBuilder res = new StringBuilder ();

			res.AppendFormat ("<Canvas  Canvas.Left=\"{0}\" Canvas.Top=\"{1}\">", pos_x, pos_y, GraphWidth);

			int step_height = FindAppropriateStepHeight (y_scale);
			int step = step_height;

			while (step < y_max) {
				res.AppendFormat ("<TextBlock Text=\"{0}\" Canvas.Top=\"{1}\" />\n", step, GraphHeight - step * y_scale - 10);
				step += step_height;
			}

			res.AppendLine ("</Canvas>");
			return res;
		}

		private StringBuilder CreateChartCanvas (int pos_x, int pos_y, int x_max, int y_max, double x_scale, double y_scale)
		{
			StringBuilder pass_line = new StringBuilder ("M ");
			StringBuilder fail_line = new StringBuilder ("M ");
			StringBuilder known_line = new StringBuilder ("M ");
			StringBuilder ignore_line = new StringBuilder ("M ");
			StringBuilder points = new StringBuilder ();

			int r = 0;
			foreach (TestRunData run in complete_test_runs) {
				double x = r * x_scale;

				double y = GraphHeight - (run.NumTestsPassed * y_scale);
				pass_line.Append (PointToStr (x, y));
				points.AppendLine (PointToEllipseStr (x, y, "Green", run, "Passed", run.NumTestsPassed));
				
				y = GraphHeight - (run.NumTestsFailed * y_scale);
				fail_line.Append (PointToStr (x, y));
				points.AppendLine (PointToEllipseStr (x, y, "Red", run, "Failed", run.NumTestsFailed));

				y = GraphHeight - (run.NumTestsKnownFailure * y_scale);
				known_line.Append (PointToStr (x, y));
				points.AppendLine (PointToEllipseStr (x, y, "Orange", run, "Known Failures", run.NumTestsKnownFailure));

				y = GraphHeight - (run.NumTestsIgnored * y_scale);
				ignore_line.Append (PointToStr (x, y));
				points.AppendLine (PointToEllipseStr (x, y, "Black", run, "Ignored", run.NumTestsIgnored));

				r++;
			}

			StringBuilder bg_lines = new StringBuilder ();
			int step_height = FindAppropriateStepHeight (y_scale);
			int step = step_height;

			while (step < y_max) {
				bg_lines.AppendFormat ("M {0} H {1}", PointToStr (0, GraphHeight - step * y_scale), GraphWidth);
				step += step_height;
			}

			StringBuilder xaml = new StringBuilder ();
			xaml.AppendFormat ("<Canvas  Canvas.Left=\"{0}\" Canvas.Top=\"{1}\">", pos_x, pos_y);
			xaml.AppendFormat ("<Rectangle Stroke=\"Black\" StrokeThickness=\"1\" Canvas.Left=\"{0}\" Canvas.Top=\"{1}\" Width=\"{2}\" Height=\"{3}\" />",
					0, 0, GraphWidth, GraphHeight);
			xaml.AppendFormat ("<Path Stroke=\"Gray\" StrokeThickness=\"1\" Data=\"{0}\" />\n", bg_lines);
			xaml.AppendFormat ("<Path Stroke=\"Green\" StrokeThickness=\"1\" Data=\"{0}\" />\n", pass_line);
			xaml.AppendFormat ("<Path Stroke=\"Red\" StrokeThickness=\"1\" Data=\"{0}\" />\n", fail_line);
			xaml.AppendFormat ("<Path Stroke=\"Orange\" StrokeThickness=\"1\" Data=\"{0}\" />\n", known_line);
			xaml.AppendFormat ("<Path Stroke=\"Black\" StrokeThickness=\"1\" Data=\"{0}\" />\n", ignore_line);
			xaml.AppendLine (points.ToString ());
			xaml.AppendLine ("</Canvas>");
			
			return xaml;
		}

		private int FindAppropriateStepHeight (double y_scale)
		{
			int res = 1;
			int ideal_step_height = (int) GraphHeight / 10;
			double value_of_step_height = ideal_step_height / y_scale;

			res = (int) (Math.Max (1, (int) value_of_step_height / 10) * 10);
			return res;
		}

		private int GetYMax ()
		{
			int max = this_run.ExecutedTests.Count;

			foreach (TestRunData run in test_runs)
				max = Math.Max (max, run.NumTestsExecuted);

			return max;
		}

		private int GetXMax ()
		{
			return complete_test_runs.Count;
		}

		private double GetXScale (int x_max)
		{
			return GraphWidth / x_max;
		}

		private double GetYScale (int y_max)
		{
			return GraphHeight / y_max;
		}

		private string PointToStr (double x, double y)
		{
			return String.Format ("{0},{1} ", x, y);
		}

		private string PointToEllipseStr (double x, double y, string colour, TestRunData run_data, string line, int count)
		{
			return String.Format ("<Ellipse Fill=\"{0}\" Width=\"5\" Height=\"5\" Canvas.Left=\"{1}\" Canvas.Top=\"{2}\" " +
					"MouseEnter=\"EllipseMouseEnter\" MouseLeave=\"EllipseMouseLeave\" Tag=\"{3}\" />", colour, x - 2.5, y - 2.5,
					TagForEllipse (run_data, line, count));
		}

		private string TagForEllipse (TestRunData run_data, string line, int count)
		{
			return String.Format ("{0},{1},{2}", run_data.StartTime, line, count);
		}

		private string GetBaseDirectory (string str)
		{
			int ds = str.LastIndexOf (Path.DirectorySeparatorChar);

			if (ds <= 0)
				return str;

			return str.Substring (ds + 1, str.Length - ds - 1);
		}
	}
}

