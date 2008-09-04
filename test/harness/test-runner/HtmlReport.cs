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
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;


namespace MoonlightTests {

	public class HtmlReport : IReport {

		private static string ReportDirectory = "html_report";

		private TestRun run;

		private StringBuilder doc;
		private int summary_pt;
		private int pass_pt;
		private int fail_pt;
		private int knfl_pt;
		private int ignore_pt;
		
		private int fail_count = 0;
		private int pass_count = 0;
		private int ignore_count = 0;
		private int known_fail_count = 0;
		

		public void BeginRun (TestRun run)
		{
			this.run = run;

			doc = new StringBuilder ();

			// So people don't get confused with the old reports
			if (File.Exists ("html_report.html"))
				File.Delete ("html_report.html");

			Directory.CreateDirectory ("html_report");

			foreach (string file in Directory.GetFiles (ReportDirectory)) {
				File.Delete (file);
			}

			GeneratePageHeader ();
			summary_pt = doc.Length;
			UpdateSummaryTable();

			GenerateTableHeader ("red", "Failing Tests");
			fail_pt = doc.Length;
			GenerateTableFooter ();

			GenerateTableHeader ("orange", "Known Failures");
			knfl_pt = doc.Length;
			GenerateTableFooter ();

			GenerateTableHeader ("green", "Passing Tests");
			pass_pt = doc.Length;
			GenerateTableFooter ();

			GenerateTableHeader ("white", "Ignored");
			ignore_pt = doc.Length;
			GenerateTableFooter ();

			GeneratePageFooter ();
		}

		public void EndRun ()
		{
		}

		public void Executing (Test test)
		{
		}

		public void AddResult (Test test, TestResult result)
		{
			int len = 0;

			switch (result) {
			case TestResult.Fail:
				len = GenerateTableRow (fail_pt, "red", " - FAIL", test);
				++fail_count;
				break;
			case TestResult.KnownFailure:
				len = GenerateTableRow (knfl_pt, "orange", " - Known Failure", test);
				++known_fail_count;
				break;
			case TestResult.Pass:
			case TestResult.UnexpectedPass:
				len = GenerateTableRow (pass_pt, "green", " - PASS", test);
				++pass_count;
				break;
			case TestResult.Ignore:
				len = GenerateIgnoreTableRow (ignore_pt, test);
				++ignore_count;
				break;
			}

			switch (result) {
			case TestResult.Fail:
				fail_pt += len;
				goto case TestResult.KnownFailure;
			case TestResult.KnownFailure:
				knfl_pt += len;
				goto case TestResult.Pass;
			case TestResult.UnexpectedPass:
			case TestResult.Pass:
				pass_pt += len;
				goto case TestResult.Ignore;
			case TestResult.Ignore:
				ignore_pt += len;
				break;
			}
			UpdateSummaryTable();

			WriteDocument ();
		}

		private void WriteDocument ()
		{
			using (StreamWriter writer = new StreamWriter (Path.Combine (ReportDirectory, "index.html"), false)) {
				writer.Write (doc.ToString ());
			}
		}

		private void GeneratePageHeader ()
		{
			doc.AppendLine ("<html>");
			doc.AppendLine ("<head><title>Moonlight Tests Report</title></head>");
			doc.AppendLine ("<body>");
			doc.AppendFormat ("<h1>Test run from: {0}</h1>", run.StartTime);
			doc.AppendLine ("\n\n");
		}

		private void GeneratePageFooter ()
		{
			doc.AppendLine ("</table>");
			doc.AppendLine ("</body>");
			doc.AppendLine ("</html>");
		}

		private StringBuilder GenerateSummaryTable ()
		{
			StringBuilder summary = new StringBuilder();
			summary.AppendLine ("<p><p><b>");
			summary.AppendLine ("<table border=\"1\" width=\"30%\">");

			float pct = (pass_count * 100.0f) / (pass_count + fail_count + known_fail_count);

			summary.AppendLine (String.Format ("<th colspan=\"2\" bgcolor=\"{0}\"><b>{1} {2,5}%</b></th>", "blue","Test Summary",pct.ToString("##.#")));
			summary.AppendLine (String.Format("<tr><td width=\"50%\"><a href=\"#Failing Tests\">Failures</a></td><td style=\"color:{0}\">{1,5}</td></tr>","red",fail_count));
			summary.AppendLine (String.Format("<tr><td><a href=\"#Known Failures\">Known Failures</a></td><td style=\"color:{0}\">{1,5}</td></tr>","orange",known_fail_count));
			summary.AppendLine (String.Format("<tr><td><a href=\"#Passing Tests\">Passes</a></td><td style=\"color:{0}\">{1,5}</td></tr>","green",pass_count));
			summary.AppendLine (String.Format("<tr><td><a href=\"#Ignored\">Ignored</a></td><td style=\"color:{0}\">{1,5}</td></tr>","gray",ignore_count));
			summary.AppendLine("</table></b>");
			
			return summary;
		}
		private void UpdateSummaryTable()
		{
			StringBuilder str = GenerateSummaryTable(); //Get summary table
			
			if (summary_pt + str.Length < doc.Length)
			{
				doc.Remove(summary_pt, str.Length);			// Delete old summary table
			}
			doc.Insert(summary_pt, str);                // Insert new table
		}
		
		private void GenerateTableHeader (string bgcolor, string title)
		{
			doc.AppendLine (String.Format("<p><p><a name=\"{0}\"></a>",title));
			doc.AppendLine ("<table border=\"1\" width=\"100%\">");

			doc.AppendLine (String.Format ("<th colspan=\"2\" bgcolor=\"{0}\"><b>{1}</b></th>", bgcolor, title));
			doc.AppendLine ("<tr><td><b>Results File</b></td><td><b>Master file</b></td></tr>");
		}

		private void GenerateTableFooter ()
		{
			doc.AppendLine ("</table>");
		}

		private int GenerateTableRow (int insert_pt, string bgcolor, string remark, Test test)
		{
			StringBuilder row = new StringBuilder ();

			row.AppendLine (String.Format ("<tr><td bgcolor=\"{0}\" colspan=\"2\">{1} ({2}){3}{4}{5} difference:  {6}</td></tr>",
					bgcolor, Path.GetFileName (test.InputFile), test.Id, remark,
					(test.IsKnownFailure ? String.Concat (" - ", test.KnownFailureReason) : String.Empty),
					(test.FailedReason != null ? String.Concat (" - ", test.FailedReason) : String.Empty),
					test.ImageDifference));

			string result_file = GetFilePath (test.ResultFile);
			string master_file = GetFilePath (test.MasterFile);

			CopyFileToReportDirectory (result_file);
			CopyFileToReportDirectory (master_file);

			row.AppendLine ("<tr>");
			row.AppendLine (String.Format ("<td><img src=\"{0}\" /></td>", Path.GetFileName (result_file)));
			row.AppendLine (String.Format ("<td><img src=\"{0}\" /></td>", Path.GetFileName (master_file)));
			row.AppendLine ("</tr>");

			string edge_diff_file = GetFilePath (String.Concat (test.InputFile, "-edge-diff.png"));

			CopyFileToReportDirectory (edge_diff_file);
			
			row.AppendLine ("<tr>");
			row.AppendLine (String.Format ("<td><img src=\"{0}\" /></td>", Path.GetFileName (edge_diff_file)));
			row.AppendLine (String.Format ("<td>&nbsp;</td>"));
			row.AppendLine ("</tr>");

			doc.Insert (insert_pt, row.ToString ());
			return row.Length;
		}

		private int GenerateIgnoreTableRow (int insert_pt, Test test)
		{
			StringBuilder row = new StringBuilder ();

			row.AppendLine (String.Format ("<tr><td>{0} - IGNORED </td>", Path.GetFileName (test.InputFile)));
			row.AppendLine (String.Format ("<td>Ignore Reason: {0}</td></tr>", test.IgnoreReason == String.Empty ? "Unknown" : test.IgnoreReason));

			doc.Insert (insert_pt, row.ToString ());
			return row.Length;
		}

		private string GetFilePath (string path)
		{
			string res = path;

			if (!File.Exists (res))
				return "image-not-found.png";

			if (path.EndsWith (".tif") || path.EndsWith (".tiff"))
				res = ImageCompare.CreateMosaicFromTiff(path);

			return res;
		}

		private void CopyFileToReportDirectory (string path)
		{
			if (!File.Exists (path))
				return;

			File.Copy (path, Path.Combine (ReportDirectory, Path.GetFileName (path)), true);
		}

	}
}

