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
using System.Threading;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Xml;

namespace MoonlightTests {

	public class Test {

		private static int AvailableLocationPort = 8000;
		
		private string id;
		private string input_file;
		private string master_file;
		private string base_directory;
		private double? image_compare_tolerance;
		private double image_diff;

		private bool size_loaded = false;
		private int result_width = 500;
		private int result_height = 500;

		private bool ignore;
		private bool remote;
		private bool unittest;
		
		private string stdout;
		private string stderr;
		private string ignore_reason;
		private string failed_reason;

		private TestCompleteReason complete_reason;

		private bool known_failure;
		private string known_failure_reason;

		private ArrayList categories;

		private int timeout = 120000;

		private ExternalProcess xsp;
		private string xsp_exec_dir;

		private ExternalProcess location_xsp;
		private string location;
		private Uri location_uri;
		private UriBuilder location_builder;
		private int location_port;

		private string codebehind;
		private TestResult test_result;

		public static Test Create (string base_directory, XmlNode node)
		{
			Test test;
			string id = null;
			string input_file = null;
			string master_file = null;
			string extension = null;
			bool remote = false;
			bool unittest = false;
			
			if (node.Attributes ["id"] != null)
				id = node.Attributes ["id"].Value;
			
			if (node.Attributes ["remote"] != null && bool.Parse (node.Attributes ["remote"].Value))
				remote = true;

			if (node.Attributes ["unittest"] != null)
				bool.TryParse (node.Attributes ["unittest"].Value, out unittest);
			
			if (node.Attributes ["inputFile"] != null) {
				input_file = node.Attributes ["inputFile"].Value;
				if (!remote)
					input_file = Path.Combine (base_directory, input_file);
			}

			if (node.Attributes ["masterFile"] != null)
				master_file = Path.Combine (base_directory, node.Attributes ["masterFile"].Value);
			else if (node.Attributes ["masterFile10"] != null)
				master_file = Path.Combine (base_directory, node.Attributes ["masterFile10"].Value);

			if (id == null || input_file == null || master_file == null)
				return null;

			if (remote) // Treat all remote files as html files
				extension = ".html";
			else
				extension = Path.GetExtension (input_file);
			
			switch (extension) {
			case ".xaml":
				test = XamlTest.Create (id, input_file, master_file, node);
				break;
			case ".htm":
			case ".html":
				test = new HtmlTest (id, input_file, master_file, node);
				break;
			default:
				Console.Error.WriteLine ("The test {0} is invalid, the test has to have a htm(l) or xaml extension.", input_file);
				return null;
			}
			
			test.base_directory = base_directory;
			test.remote = remote;
			test.unittest = unittest;
			
			if (node.Attributes ["knownFailure"] != null && Boolean.Parse (node.Attributes ["knownFailure"].Value)) {
				test.known_failure = true;
				if (node.Attributes ["failureReason"] != null)
					test.known_failure_reason = node.Attributes ["failureReason"].Value;
			} else if (node.Attributes ["ignore"] != null && Boolean.Parse (node.Attributes ["ignore"].Value)) {
				test.SetToIgnore (String.Format ("Ignore specified in drtlist{0}",
						node.Attributes ["ignoreReason"] != null ? " -- " + node.Attributes ["ignoreReason"].Value + "." : "."));
			}

			if (node.Attributes ["timeout"] != null)
				test.timeout = Int32.Parse (node.Attributes ["timeout"].Value);

			if (node.Attributes ["imageCompareTolerance"] != null)
				test.image_compare_tolerance = Double.Parse (node.Attributes ["imageCompareTolerance"].Value);

			if (node.Attributes ["categories"] != null)
				test.SetCategories (node.Attributes ["categories"].Value);
			
			if (node.Attributes ["xspExecDir"] != null)
				test.xsp_exec_dir = node.Attributes ["xspExecDir"].Value;

			if (node.Attributes ["codebehind"] != null)
				test.codebehind = node.Attributes ["codebehind"].Value;

			
			if (node.Attributes ["location"] != null) {
				test.location = node.Attributes ["location"].Value;
				test.LocationPort = AvailableLocationPort++;
				test.remote = true;
			}
			
			return test;
		}

		protected Test (string id, string input_file, string master_file)
		{
			this.id = id;
			this.input_file = input_file;
			this.master_file = master_file;
		}

		public TestResult Result {
			get { return test_result; }
			set { test_result = value; }
		}

		public bool UnitTest {
			get { return unittest; }
		}
		
		public int LocationPort {
			get {
				return location_port;
			}
			set {
				UriBuilder u = new UriBuilder (location);
				location_port = value;
				u.Port = location_port;
				location_uri = u.Uri;
				u.Path = Path.Combine (u.Path, Path.GetFileName (input_file));
				input_file = u.ToString ();
			}
		}
		
		public string Id {
			get { return id; }
		}

		public string InputFile {
			get { return input_file; }
		}

		public string InputFileName {
			get { return Path.GetFileName (input_file); }
		}

		public string LocalFilePath {
			get { return Path.Combine (base_directory, InputFileName); }
		}

		public int Timeout {
			get { return timeout; }
		}

		public string Location {
			get { return location; }
		}

		public bool Ignore {
			get { return ignore; }
		}

		public bool Remote {
			get { return remote;  }
		}
		
		public TestCompleteReason CompleteReason {
			get { return complete_reason; }
			set { complete_reason = value; }
		}

		public bool IsKnownFailure {
			get { return known_failure; }
		}

		public string KnownFailureReason {
			get {
				if (known_failure_reason == null)
					return String.Empty;
				return known_failure_reason;
			}
		}

		public double? ImageCompareTolerance {
			get { return image_compare_tolerance; }
		}

		public double ImageDifference {
			get { return image_diff; }
			set { image_diff = value; }
		}

		public string IgnoreReason {
			get { return ignore_reason; }
		}

		public string FailedReason {
			get { return failed_reason; }
		}
		
		public string MasterFile {
			get { return master_file; }
		}

		public string MoonMasterFile {
			get { return FindMoonMaster (); }
		}

		public string ResultFile {
			get { return FindTestResult ();	}
		}

		public int ResultWidth {
			get { 
				GetDimensionsFromMaster (); 
				return result_width;
			}
		}

		public int ResultHeight {
			get {
				GetDimensionsFromMaster (); 
				return result_height;
			}
		}

		public string Stderr {
			get {
				if (stderr == null)
					return String.Empty;
				return stderr;
			}
		}

		public string Stdout {
			get {
				if (stdout == null)
					return String.Empty;
				return stdout;
			}
		}

		public string [] Categories {
			get {
				if (categories == null || categories.Count == 0)
					return new string [0];
				return (string []) categories.ToArray (typeof (string));
			}
		}

		public bool IsInCategoryList (List<string> run_list)
		{
			if (categories == null)
				return false;

			foreach (string cat in run_list) {
				if (categories.Contains (cat))
					return true;
			}

			return false;
		}
		
		public bool IsInCategoryList (ArrayList run_list)
		{
			if (categories == null)
				return false;

			foreach (string cat in run_list) {
				if (categories.Contains (cat))
					return true;
			}

			return false;
		}

		public TestResult ComputeImageCompareResult ()
		{
			if (Path.GetFileName (master_file) != "None")
				return CompareResults ();
			return TestResult.Pass;
		}

		public void SetToIgnore (string reason)
		{
			ignore = true;
			ignore_reason = reason;
		}

		public void SetFailedReason (string reason)
		{
			failed_reason = reason;
		}

		public TestResult ProcessUnitTestResult ()
		{
			if (!unittest)
				return test_result;

			try {
				string filename = null;
				foreach (string line in location_xsp.Stdout.Split ('\n')) {
					if (line.StartsWith ("MoonLogProvider: Saving file to: ")) {
						filename = line.Substring ("MoonLogProvider: Saving file to: ".Length);
						break;
					}
				}
				
				if (filename == null)
					return test_result;

				if (!System.IO.File.Exists (filename))
					return test_result;

				int failed = 0;
				int knownissue = 0;
				int notexecuted = 0;
				int counter = 0;
				int passed = 0;
				string result = null;
				using (FileStream fs = new FileStream (filename, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)) {
					using (StreamReader stream = new StreamReader (fs, System.Text.Encoding.UTF8)) {
						XmlReaderSettings rs = new XmlReaderSettings ();
						rs.ConformanceLevel = ConformanceLevel.Document;
						rs.IgnoreComments = true;
						rs.IgnoreProcessingInstructions = true;
						rs.IgnoreWhitespace = true;
						XmlReader reader = XmlReader.Create (stream, rs);
						reader.Read ();
						reader.ReadToFollowing ("MoonLog");
						if (reader.ReadToDescendant ("Test")) {
							do {
								result = reader.GetAttribute ("Result");
								
								counter++;
								if (result == "Failed")
									failed++;
								else if (result == "KnownIssue")
									knownissue++;
								else if (result == "NotExecuted")
									notexecuted++;
								else if (result == "Passed")
									passed++;
								
								if (result != "Passed") {
									if (string.IsNullOrEmpty (result))
										result = "<unknown>";
									
									Console.WriteLine ("{0,-12} {1}: {2}", result + ":", reader.GetAttribute ("FullName"), reader.GetAttribute ("Message"));
								}
							} while (reader.ReadToFollowing ("Test"));
						}
					}
				}
				Console.WriteLine ("Unit tests executed: {0} {1} Pass, {2} Not executed, {3} Failures, {4} Known issues.", counter, passed, notexecuted, failed, knownissue);
				return failed == 0 ? TestResult.Pass : TestResult.Fail;
			} catch (Exception ex) {
				Console.WriteLine ("Exception while trying to parse unit test result:");
				Console.WriteLine (ex.ToString ());
			}
			
			return test_result;
		}
		
		public virtual void Setup ()
		{
			if (!Remote && !File.Exists (InputFile)) {
				SetToIgnore (String.Format ("Unable to find input file: {0}", InputFile));
				return;
			}

			// Delete the old results file, so if the test crashes we don't compare to the old results accidently
			string test_result_file = FindTestResult ();
			if (test_result_file != null)
				File.Delete (test_result_file);

			CodeBehindCompileIfNeeded ();
			RunLocationServerIfNeeded ();
			RunXspIfNeeded ();
		}

		public virtual void Teardown ()
		{			
			StopXspIfNeeded ();
			StopLocationServerIfNeeded ();
		}

		protected virtual string GetTestPath ()
		{
			return input_file;
		}

		private TestResult RunTest ()
		{
			return Agviewer.RunTest (this, timeout, out stdout, out stderr);
		}

		private TestResult CompareResults ()
		{
			string result_file = FindTestResult ();

			return ImageCompare.Compare (this, result_file, MasterFile);
		}

		private string FindTestResult ()
		{
			string ext;
			
			ext = Path.GetExtension (MasterFile);
			
			if (ext == String.Empty)
				ext = ".png";

			if (Remote)
				return Path.Combine (base_directory, String.Concat (InputFileName, ext));

			return String.Concat (InputFile, ext);
		}

		private string FindMoonMaster ()
		{
			return String.Concat (InputFile, "-moon-master", Path.GetExtension (MasterFile));
		}

		private void GetDimensionsFromMaster ()
		{
			if (!File.Exists (master_file))
				return;

			if (size_loaded)
				return;
			
			using (Image master = Image.FromFile (master_file)) {
				result_width = master.Width;
				result_height = master.Height;
			}
			
			size_loaded = true;
		}

		private void CodeBehindCompileIfNeeded ()
		{
			if (codebehind == null)
				return;

			string [] pieces = codebehind.Split (',');
			string cs_file = Path.Combine (base_directory, pieces [0].Trim ());
			string lib_name = null;

			if (pieces.Length > 1)
				lib_name = pieces [1].Trim ();			

			string args = String.Format ("-pkg:silver /target:library {0} {1}", lib_name != null ? String.Concat ("/out:", lib_name) : String.Empty, cs_file);
			using (ExternalProcess gmcs = new ExternalProcess ("gmcs", args, -1)) {

				gmcs.Run (true);
				if (gmcs.ExitCode < 0) {
					SetFailedReason (String.Format ("Unable to compile codebehind file: {0}.", gmcs.Stderr));
					return;
				}
			}
		}

		private void RunXspIfNeeded ()
		{
			if (xsp_exec_dir == null)
				return;

			string args = String.Format ("--root {0} --nonstop", Path.Combine (Path.GetDirectoryName (LocalFilePath), xsp_exec_dir));

			xsp = new ExternalProcess ("xsp", args, -1);
			xsp.RedirectStdOut = true;
			xsp.Run (false);

			if (!WaitForXsp (xsp)) {
				// We can't try another port, given that tests assume xsp is running on 8080.
				Console.WriteLine ("Harness: xsp didn't startup correctly, assuming there already is an xsp process running.");
			}
		}

		private static bool WaitForXsp (ExternalProcess process)
		{
			DateTime start = DateTime.Now;
			string stdout;
			while (true) {
				Thread.Sleep (100);
				if (start.AddSeconds (5) < DateTime.Now) {
					Console.WriteLine ("Harness: timeout while waiting for xsp to start. Assuming xsp started successfully.");
					return true;
				}
				
				stdout = process.Stdout;
				if (string.IsNullOrEmpty (stdout))
					continue;

				if (stdout.Contains ("Listening on port:"))
					return true;

				if (stdout.Contains ("Address already in use"))
					return false;

				if (!process.IsRunning) {
					Console.WriteLine ("Harness: xsp quitted unexpectedly.");
					return true;
				}
			}
		}
		
		private void RunLocationServerIfNeeded ()
		{
			if (location == null)
				return;

			Console.WriteLine ("Harness: Trying to start xsp on port {0}", LocationPort);
			do {
				string args = String.Format ("--applications '{0}:{1},/:.' --nonstop --port {2}", location_uri.AbsolutePath, base_directory, LocationPort);
				location_xsp = new ExternalProcess ("xsp2", args, -1);
				location_xsp.RedirectStdOut = true;
				location_xsp.Run (false);
				
				if (!WaitForXsp (location_xsp)) {
					Console.WriteLine ("Harness: Could not start xsp at port {0}, trying port {1}", LocationPort, AvailableLocationPort);
					LocationPort = AvailableLocationPort++;
					continue;
				} else {
					Console.WriteLine ("Harness. Started xsp on port {0} (args: {1})", LocationPort, args);
					break;
				}
			} while (true);
		}
		
		private void StopXspIfNeeded ()
		{
			if (xsp == null)
				return;
			StopXsp (xsp);
		}

		private void StopLocationServerIfNeeded ()
		{
			if (location_xsp == null)
				return;
			StopXsp (location_xsp);
		}

		private void StopXsp (ExternalProcess xsp)
		{
			try {
				if (xsp != null) {
					xsp.Kill ();
					xsp.Dispose ();
				}
			} catch (Exception ex) {
				Console.WriteLine ("Exception while trying to stop XSP: " + ex.Message + " (" + ex.GetType ().FullName + ")");
			}
		}

		private void SetCategories (string cat_str)
		{
			string [] cats = cat_str.Split (new char [] { ',' });

			categories = new ArrayList ();

			foreach (string cat in cats) {
				categories.Add (cat.Trim ());
			}
		}
		
		public override string ToString()
		{
			string str = string.Empty;
			str += "--- Test ---";
			str += "\nID: " + this.Id;
			str += "\n InputFile: " + this.InputFile;
			str += "\nMasterFile: " + this.MasterFile;
			str += "\nResultFile: " + this.ResultFile;
			str += "\n--- ---";
			return str;
		}
	}
}

