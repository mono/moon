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

		public static readonly string TestRunDirectoryName = "test-run-data";
		public static readonly string TestRunFileName = "run.xml";

		private static readonly int MaxImageWidth = 400;

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
			runs_node.SetAttribute ("EndTime", DateTime.Now.ToString ("yyyy-MM-dd-HH-mm-ss"));
		}

		public void Executing (Test test)
		{
		}

		public void AddResult (Test test, TestResult result)
		{			
			XmlElement el = document.CreateElement ("Test");

			string result_file = GetFilePath (test.ResultFile);
			string master_file = GetFilePath (test.MasterFile);

			CopyImageToRunDirectory (test_run_dir,result_file);

			el.SetAttribute ("Id", test.Id);
			el.SetAttribute ("InputFile", test.InputFile);
			el.SetAttribute ("ResultFile", MakeRelativePath (result_file));
			el.SetAttribute ("MasterFile", Path.GetFullPath (master_file));
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
			test_run_dir = Path.Combine (TestRunDirectoryName, run.StartTime.ToString ("yyyy-MM-dd-HH-mm-ss"));
			test_run_path = Path.Combine (test_run_dir, TestRunFileName);

			if (!Directory.Exists (test_run_dir))
				Directory.CreateDirectory (test_run_dir);
		}

		public static string GetFilePath (string path)
		{
			string res = path;

			if (!File.Exists (res))
				return "image-not-found.png";

			if (path.EndsWith (".tif") || path.EndsWith (".tiff"))
				res = ImageCompare.CreateMosaicFromTiff(path);

			
			return res;
		}

		public static void CopyImageToRunDirectory (string test_run_dir, string path)
		{
			if (!File.Exists (path))
				return;

			string newpath = Path.Combine (test_run_dir, Path.GetFileName (path));
			File.Copy (path, Path.Combine (test_run_dir, Path.GetFileName (path)), true);

			
			using (Bitmap image = (Bitmap) Image.FromFile (newpath)) {
				if (image.Width > MaxImageWidth)
					ResizeImage (image, newpath);
			}
		}

		public static void ResizeImage (Bitmap image, string path)
		{
			float ratio = (float) image.Height / image.Width;
			using (Bitmap ni = new Bitmap (image, MaxImageWidth, (int) (MaxImageWidth * ratio))) 
				ni.Save (path);
		}

		private string MakeRelativePath (string path)
		{
			return Path.Combine (test_run_dir, Path.GetFileName (path));
		}
	}
}


