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
		private int pass_pt;
		private int fail_pt;
		private int knfl_pt;
		private int ignore_pt;

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
				break;
			case TestResult.KnownFailure:
				len = GenerateTableRow (knfl_pt, "orange", " - Known Failure", test);
				break;
			case TestResult.Pass:
				len = GenerateTableRow (pass_pt, "green", " - PASS", test);
				break;
			case TestResult.Ignore:
				len = GenerateIgnoreTableRow (ignore_pt, test);
				break;
			}

			switch (result) {
			case TestResult.Fail:
				fail_pt += len;
				goto case TestResult.KnownFailure;
			case TestResult.KnownFailure:
				knfl_pt += len;
				goto case TestResult.Pass;
			case TestResult.Pass:
				pass_pt += len;
				goto case TestResult.Ignore;
			case TestResult.Ignore:
				ignore_pt += len;
				break;
			}

			WriteDocument ();
		}

		private void WriteDocument ()
		{
			using (StreamWriter writer = new StreamWriter (Path.Combine (ReportDirectory, "index.htm"), false)) {
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

		private void GenerateTableHeader (string bgcolor, string title)
		{
			doc.AppendLine ("<p><p>");
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

			row.AppendLine (String.Format ("<tr><td bgcolor=\"{0}\" colspan=\"2\">{1}{2}{3}{4} difference:  {5}</td></tr>",
					bgcolor, Path.GetFileName (test.InputFile), remark,
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
				res = CreateMosaicFromTiff (path);

			return res;
		}

		private void CopyFileToReportDirectory (string path)
		{
			if (!File.Exists (path))
				return;

			File.Copy (path, Path.Combine (ReportDirectory, Path.GetFileName (path)), true);
		}

		private string CreateMosaicFromTiff (string tiff_path)
		{
			Graphics g;
			Bitmap result;
			int num_images = 0;
			string result_path = String.Concat (tiff_path, ".png");
			Bitmap tiff = (Bitmap) Image.FromFile (tiff_path);

			Guid [] tiff_frames = tiff.FrameDimensionsList;
			for (int i = 0; i < tiff_frames.Length; i++) {
				FrameDimension tiff_dimension = new FrameDimension (tiff_frames [0]);
				int frames_count = tiff.GetFrameCount (tiff_dimension);

				for (int f = 0; f < frames_count; f++)
					num_images++;
			}

			if (num_images == 1) {
				result = new Bitmap (tiff.Width, tiff.Height);
				g = Graphics.FromImage (result);

				g.DrawImage (tiff, 0, 0);
				result.Save (result_path, ImageFormat.Png);
				return result_path;
			}

			
			int border_width = 2;
			int x = border_width;
			int y = border_width;
			int images_per_row = Math.Max (num_images / 4, 1);			

			result = new Bitmap (tiff.Width + border_width * 5, (images_per_row * (tiff.Height / 4)) + (border_width * (images_per_row + 1)));
			g = Graphics.FromImage (result);

			g.Clear (Color.Black);

			for (int i = 0; i < tiff_frames.Length; i++) {
				FrameDimension tiff_dimension = new FrameDimension (tiff_frames [0]);
				int frames_count = tiff.GetFrameCount (tiff_dimension);

				for (int f = 0; f < frames_count; f++) {
					tiff.SelectActiveFrame (tiff_dimension, f);

					g.DrawImage (tiff, x, y, tiff.Width / 4, tiff.Height / 4);

					x += tiff.Width / 4 + border_width;
					if (x >= tiff.Width) {
						x = border_width;
						y += tiff.Height / 4 + border_width;
					}
						
				}
			}

			result.Save (result_path, ImageFormat.Png);
			return result_path;
		}
	}
}

