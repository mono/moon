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
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.Globalization;

namespace MoonlightTests {

	public class XmlReport : IReport {

		private static readonly int MaxImageWidth = 400;
		private static readonly string TestRunDirectoryName = "test-run-data";
		private static readonly string TestRunFileName = "run.xml";

		private TestRun run;
		private XmlDocument document;
		private XmlElement runs_node;
		private string test_run_dir;
		private string test_run_path;

		public void BeginRun (TestRun run)
		{
			this.run = run;

			document = new XmlDocument ();
			runs_node = document.CreateElement ("TestRun", null);
			document.AppendChild (runs_node);

			SetupRunsNode ();
			CreateTestRunPathNames (run);
		}

		public void EndRun ()
		{
			runs_node.SetAttribute ("EndTime", DateTime.Now.ToString (CultureInfo.InvariantCulture));
		}

		public void Executing (Test test)
		{
		}

		public void AddResult (Test test, TestResult result)
		{
			XmlElement el = document.CreateElement ("Test");

			string result_file = GetFilePath (test.ResultFile);
			string master_file = GetFilePath (test.MasterFile);
			string edge_diff_file = GetFilePath (String.Concat (test.InputFile, "-edge-diff.png"));

			CopyImageToRunDirectory (result_file);
			CopyImageToRunDirectory (master_file);
			CopyImageToRunDirectory (edge_diff_file);

			el.SetAttribute ("Id", test.Id);
			el.SetAttribute ("InputFile", test.InputFile);
			el.SetAttribute ("ResultFile", MakeRelativePath (result_file));
			el.SetAttribute ("MasterFile", MakeRelativePath (master_file));
			el.SetAttribute ("EdgeDiffFile", MakeRelativePath (edge_diff_file));
			el.SetAttribute ("TestResult", result.ToString ());

			foreach (string category in test.Categories) {
				XmlElement cat = document.CreateElement ("Category");
				cat.SetAttribute ("Name", category);

				el.AppendChild (cat);
			}

			runs_node.AppendChild (el);

			UpdateRunsNode ();
			SaveDocument ();
		}

		private void SetupRunsNode ()
		{
			runs_node.SetAttribute ("BaseDirectory", run.BaseDirectory);
			runs_node.SetAttribute ("StartTime", run.StartTime.ToString (CultureInfo.InvariantCulture));
		}

		private void UpdateRunsNode ()
		{
			runs_node.SetAttribute ("ExecutedTests", run.ExecutedTests.Count.ToString (CultureInfo.InvariantCulture));
			runs_node.SetAttribute ("PassedTests",  run.PassedTests.Count.ToString (CultureInfo.InvariantCulture));
			runs_node.SetAttribute ("IgnoredTests", run.IgnoredTests.Count.ToString (CultureInfo.InvariantCulture));
			runs_node.SetAttribute ("FailedTests", run.FailedTests.Count.ToString (CultureInfo.InvariantCulture));
			runs_node.SetAttribute ("KnownFailures", run.KnownFailures.Count.ToString (CultureInfo.InvariantCulture));
		}
			
		private void SaveDocument ()
		{
			document.Save (test_run_path);
		}

		private void CreateTestRunPathNames (TestRun run)
		{
			test_run_dir = Path.Combine (TestRunDirectoryName, run.StartTime.ToString ("yyyy-MM-dd-hh-mm"));
			test_run_path = Path.Combine (test_run_dir, TestRunFileName);

			if (!Directory.Exists (test_run_dir))
				Directory.CreateDirectory (test_run_dir);
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

		private void CopyImageToRunDirectory (string path)
		{
			if (!File.Exists (path))
				return;

			string newpath = Path.Combine (test_run_dir, Path.GetFileName (path));
			File.Copy (path, Path.Combine (test_run_dir, Path.GetFileName (path)), true);

			
			Bitmap image = (Bitmap) Image.FromFile (newpath);
			if (image.Width > MaxImageWidth)
				ResizeImage (image, newpath);
		}

		private void ResizeImage (Bitmap image, string path)
		{
			float ratio = (float) image.Height / image.Width;
			Bitmap ni = new Bitmap (image, MaxImageWidth, (int) (MaxImageWidth * ratio));
			ni.Save (path);
		}

		private string MakeRelativePath (string path)
		{
			return Path.Combine (test_run_dir, Path.GetFileName (path));
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


