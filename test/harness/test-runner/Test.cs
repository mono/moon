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
using System.Diagnostics;

namespace MoonlightTests {

	public class Test {

		private string id;
		private string input_file;
		private string master_file;
		private string base_directory;
		private double? image_compare_tolerance;
		private double image_diff;

		private int result_width = 500;
		private int result_height = 500;

		private bool ignore;

		private string stdout;
		private string stderr;
		private string ignore_reason;
		private string failed_reason;

		private bool known_failure;
		private string known_failure_reason;

		private ArrayList categories;

		private int timeout = 120000;

		private ExternalProcess xsp;
		private string xsp_exec_dir;

		private string codebehind;

		public static Test Create (string base_directory, XmlNode node)
		{
			Test test;
			string id = null;
			string input_file = null;
			string master_file = null;

			if (node.Attributes ["id"] != null)
				id = node.Attributes ["id"].Value;
			if (node.Attributes ["inputFile"] != null)
				input_file = Path.Combine (base_directory, node.Attributes ["inputFile"].Value);

			if (node.Attributes ["masterFile"] != null)
				master_file = Path.Combine (base_directory, node.Attributes ["masterFile"].Value);
			else if (node.Attributes ["masterFile10"] != null)
				master_file = Path.Combine (base_directory, node.Attributes ["masterFile10"].Value);

			if (id == null || input_file == null || master_file == null)
				return null;

			switch (Path.GetExtension (input_file)) {
			case ".xaml":
				test = XamlTest.Create (id, input_file, master_file, node);
				break;
			case ".htm":
			case ".html":
				test = new HtmlTest (id, input_file, master_file, node);
				break;
			default:
				return null;
			}
			
			test.base_directory = base_directory;

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

			return test;
		}

		protected Test (string id, string input_file, string master_file)
		{
			this.id = id;
			this.input_file = input_file;
			this.master_file = master_file;

			GetDimensionsFromMaster ();
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
			get { return result_width; }
		}

		public int ResultHeight {
			get { return result_height; }
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

		public TestResult Execute (bool compare_to_moon)
		{
			TestResult result;

			if (ignore) {
				// Create an error report?
				return TestResult.Ignore;
			}

			string test_result_file = FindTestResult ();
			if (test_result_file != null)
				File.Delete (test_result_file);

			Setup ();

			if (ignore)
				return TestResult.Ignore;

			result = RunTest ();
			if (result != TestResult.Pass) {
				Teardown ();
				return result;
			}

			if (Path.GetFileName (master_file) != "None")
				result = CompareResults (compare_to_moon);

			Teardown ();

			return result;
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

		protected virtual void Setup ()
		{
			if (!File.Exists (InputFile)) {
				SetToIgnore (String.Format ("Unable to find input file: {0}", InputFile));
				return;
			}

			CodeBehindCompileIfNeeded ();
			RunXspIfNeeded ();
		}

		protected virtual void Teardown ()
		{			
			StopXspIfNeeded ();
		}

		protected virtual string GetTestPath ()
		{
			return input_file;
		}

		private TestResult RunTest ()
		{
			return Agviewer.RunTest (this, timeout, out stdout, out stderr);
		}

		private TestResult CompareResults (bool compare_to_moon)
		{
			string result_file = FindTestResult ();

			return ImageCompare.Compare (this, result_file, compare_to_moon ? FindMoonMaster () : MasterFile);
		}

		private string FindTestResult ()
		{
			string ext = Path.GetExtension (MasterFile);
			if (ext == String.Empty)
				ext = ".png";

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

			Image master = Image.FromFile (master_file);
			result_width = master.Width;
			result_height = master.Height;
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
			ExternalProcess gmcs = new ExternalProcess ("gmcs", args, -1);

			gmcs.Run (true);
			if (gmcs.ExitCode < 0) {
				SetFailedReason (String.Format ("Unable to compile codebehind file: {0}.", gmcs.Stderr));
				return;
			}
		}

		private void RunXspIfNeeded ()
		{
			if (xsp_exec_dir == null)
				return;

			string args = String.Format ("--root {0}", Path.Combine (Path.GetDirectoryName (input_file), xsp_exec_dir));

			xsp = new ExternalProcess ("xsp", args, -1);
			xsp.Run (false);
		}

		private void StopXspIfNeeded ()
		{
			if (xsp != null)
				xsp.Kill ();
		}

		private void SetCategories (string cat_str)
		{
			string [] cats = cat_str.Split (new char [] { ',' });

			categories = new ArrayList ();

			foreach (string cat in cats) {
				categories.Add (cat.Trim ());
			}
		}
	}
}

