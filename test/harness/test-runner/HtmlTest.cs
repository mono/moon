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
using System.Text;
using System.Text.RegularExpressions;


namespace MoonlightTests {

	public class HtmlTest : Test {
		private bool clear_cache;
		private int? capture_interval;
		private int? max_images_to_capture;
		private int initial_delay = 500;
		private int? capture_x;
		private int? capture_y;
		private int? capture_width;
		private int? capture_height;

		private string expected_error;

		public HtmlTest (string id, string input_file, string master_file, XmlNode node) : base (id, input_file, master_file)
		{
			if (node.Attributes ["captureSpecifier"] != null)
				SetCaptureSpecifier (node.Attributes ["captureSpecifier"].Value);

			if (node.Attributes ["clearCache"] != null)
				ClearCache = Boolean.Parse (node.Attributes ["clearCache"].Value);

			if (node.Attributes ["captureInterval"] != null)
				CaptureInterval = Int32.Parse (node.Attributes ["captureInterval"].Value);

			if (node.Attributes ["maxImagesToCapture"] != null)
				MaxImagesToCapture = Int32.Parse (node.Attributes ["maxImagesToCapture"].Value);

			if (node.Attributes ["initialDelay"] != null)
				InitialDelay = Int32.Parse (node.Attributes ["initialDelay"].Value);

			if (node.Attributes ["captureX"] != null)
				CaptureX = Int32.Parse (node.Attributes ["captureX"].Value);

			if (node.Attributes ["captureY"] != null)
				CaptureY = Int32.Parse (node.Attributes ["captureY"].Value);

			if (node.Attributes ["captureWidth"] != null)
				CaptureWidth = Int32.Parse (node.Attributes ["captureWidth"].Value);

			if (node.Attributes ["captureHeight"] != null)
				CaptureHeight = Int32.Parse (node.Attributes ["captureHeight"].Value);

			if (node.Attributes ["expectedError"] != null)
				ExpectedError = node.Attributes ["expectedError"].Value;

		}

		private bool ClearCache {
			get { return clear_cache; }
			set { clear_cache = value; }
		}

		private int? CaptureInterval {
			get { return capture_interval; }
			set {
				if (value < 0)
					throw new ArgumentException ("capture interval must be greater than or equal to zero.");
				capture_interval = value;
			}
		}

		private int? MaxImagesToCapture {
			get { return max_images_to_capture; }
			set {
				if (value <= 0)
					throw new ArgumentException ("max images to capture must be great than zero.");
				max_images_to_capture = value;
			}
		}

		private int InitialDelay {
			get { return initial_delay; }
			set {
				if (value < 0)
					throw new ArgumentException ("initial delay must be greater than or equal to zero.");
				initial_delay = value;
			}
		}

		private int? CaptureX {
			get { return capture_x; }
			set {
				if (value < 0)
					throw new ArgumentException ("Capture X coordindate must be zero or greater.");
				capture_y = value;
			}
		}

		private int? CaptureY {
			get { return capture_y; }
			set {
				if (value < 0)
					throw new ArgumentException ("Capture Y coordindate must be zero or greater.");
				capture_y = value;
			}
		}

		private int? CaptureWidth {
			get { return capture_width; }
			set {
				if (value <= 0)
					throw new ArgumentException ("Capture Width must be greater than zero.");
				capture_width = value;
			}
		}

		private int? CaptureHeight {
			get { return capture_height; }
			set {
				if (value <= 0)
					throw new ArgumentException ("Capture Height must be greater than zero.");
				capture_height = value;
			}
		}

		private string ExpectedError {
			get { return expected_error; }
			set {
				
				switch (value) {
					// Add sanity checking here
				}
				expected_error = value;
			}
		}

		private void SetCaptureSpecifier (string specifier)
		{
			string [] parts = specifier.Split (",".ToCharArray ());
			CaptureInterval = Int32.Parse (parts [0]);
			MaxImagesToCapture = Int32.Parse (parts [1]);
		}

		public override void Setup ()
		{
			base.Setup ();

			if (Remote && Location == null)
				return;

			if (!File.Exists (LocalFilePath)) {
				SetToIgnore (String.Format ("Unable to find input file: {0}", LocalFilePath));
				return;
			}
						
			StringBuilder built = new StringBuilder ();
			StreamReader reader = new StreamReader (File.OpenRead (LocalFilePath));

			string line;
			string xaml_source = null;
			string control_id = null;
			string control_width = "500";
			string control_height = "500";
			string background = "white";
			bool test_body_generated = false;
			bool shocker_embed_script_found = false;

			while ((line = reader.ReadLine ()) != null) {
				Match m = Regex.Match (line, "Source=\"(.*?)\"");
				if (m.Success)
					xaml_source = m.Groups [1].Value;

				m = Regex.Match (line, "id=\"(.*?)\"");
				if (m.Success)
					control_id = m.Groups [1].Value;

				m = Regex.Match (line, "width=\"(.*?)\"");
				if (m.Success)
					control_width = m.Groups [1].Value;

				m = Regex.Match (line, "height=\"(.*?)\"");
				if (m.Success)
					control_height = m.Groups [1].Value;

				m = Regex.Match (line, "Background=\"(.*?)\"");
				if (m.Success)
					background = m.Groups [1].Value;

				if (line.Contains ("document.write('Browser not supported');") || line.Contains ("$TEST_SILVERLIGHT_EMBED")) {
					built.AppendLine (GenerateSilverlightEmbedHtml (xaml_source, control_id, control_width, control_height, background));
					continue;
				}

				if (line.Contains ("$EXPECTED_ERROR_VAR")) {
					built.AppendLine (GenerateExpectedErrorVar ());
					continue;
				}

				if (line.Contains ("$RUN_TEST_BODY")) {
					built.AppendLine (line.Replace ("$RUN_TEST_BODY", GenerateRunTestBody ()));
					test_body_generated = true;
					continue;
				}

				if (line.Contains ("http://jscratch/test/jtr.js")) {
					built.AppendLine (line.Replace ("http://jscratch/test/jtr.js", "jtr.js"));
					continue;
				}

				if (line.Contains ("http://localhost/test/jtr.js")) {
					built.AppendLine (line.Replace ("http://localhost/test/jtr.js", "jtr.js"));
					continue;
				}

				if (line.Contains ("aghostDrt.js")) {
					built.AppendLine (line.Replace ("aghostDrt.js", "aghostDRT.js"));
					continue;
				}

				if (line.Contains ("$MOONLIGHT_CONTROL_WIDTH")) {
					built.AppendLine (line.Replace ("$MOONLIGHT_CONTROL_WIDTH", ResultWidth.ToString ()));
					continue;
				}

				if (line.Contains ("$MOONLIGHT_CONTROL_HEIGHT")) {
					built.AppendLine (line.Replace ("$MOONLIGHT_CONTROL_HEIGHT", ResultHeight.ToString ()));
					continue;
				}

				if (line.Contains ("<embed id=\"_TestPlugin\"")) {
					built.AppendLine (line);
					shocker_embed_script_found = true;
					continue;
				}

				if (line.Contains ("</body>") || line.Contains ("</BODY>")) {
					if (!shocker_embed_script_found)
						built.AppendLine (GenerateShockerEmbedScript (test_body_generated));
					built.AppendLine (line);
					continue;
				}

				if (line.Contains ("window.moveTo") || line.Contains ("window.resizeTo") || line.Contains ("window.resizeBy"))
					continue;

				built.AppendLine (line);
			}

			reader.Close ();
			
			string built_str = built.ToString ();
			if (built_str != File.ReadAllText (LocalFilePath))
				File.WriteAllText (LocalFilePath, built_str);
		}

		private string GenerateSilverlightEmbedHtml (string xaml_source, string control_id, string control_width, string control_height, string background)
		{
			StringBuilder res = new StringBuilder ();

			if (xaml_source == null)
				throw new Exception (String.Format ("Unable to find xaml source in html file:  {0}", InputFile));

			if (control_id == null)
				throw new Exception (String.Format ("Unable to find control id in html file:  {0}", InputFile));

			res.AppendLine ("\t\tdocument.write('<div>');");
			res.AppendLine ("\t\tdocument.write(' <embed type=\"application/x-silverlight\"');");
			res.AppendFormat ("\t\tdocument.write(' width=\"{0}\" height=\"{1}\" id=\"{2}\" Source=\"{3}\" ",
					control_width, control_height, control_id, xaml_source);
			res.AppendFormat ("Background=\"{0}\" OnError=\"ErrorHandler\" style=\"position:absolute; left:0px; top:0px\" > ');\n", background);
			res.AppendLine ("\t\tdocument.write(' </embed></div>');");

			return res.ToString ();
		}

		private string GenerateExpectedErrorVar ()
		{
			return String.Format ("\t\tvar expected_error = {0};\n", expected_error != null ? String.Concat ("\"", expected_error, "\"") : "null");
		}

		private string GenerateRunTestBody ()
		{
			StringBuilder res = new StringBuilder ();

			res.AppendLine ("\t\t\tvar moonlight_control = document.getElementById (\"MoonlightControl\");");

			if (capture_interval != null || max_images_to_capture != null) {
				res.AppendFormat ("\t\t\tTakeMultipleSnapshotsAndShutdown (moonlight_control, {0}, {1}, {2}, {3}, {4});",
						  max_images_to_capture, capture_interval, InitialDelay,
						  (capture_width != null ? capture_width : ResultWidth),
						  (capture_height != null ? capture_height : ResultHeight));
			} else {
				// what happens if initial delay is specified but not width/height ??? 	
				res.AppendFormat ("\t\t\tTakeSingleSnapshotAndShutdown (moonlight_control, \"{0}\", {1}, {2}, {3});",
						  String.Concat (Path.GetFileName (LocalFilePath), ".png"),
						  (capture_width != null ? capture_width : ResultWidth),
						  (capture_height != null ? capture_height : ResultHeight),
						  InitialDelay);
			}

			return res.ToString ();
		}

		private string GenerateShockerEmbedScript (bool test_body_generated)
		{
			StringBuilder res = new StringBuilder ();

			// '<embed id="_TestPlugin" ' +
			// '<\/embed>';

			res.AppendLine ("<embed id=\"_TestPlugin\" width=\"0\" height=\"0\" type=\"application/x-jolttest\" ");
			if (max_images_to_capture > 0 && !test_body_generated) {
				res.AppendFormat ("captureinterval=\"{0}\"\n", capture_interval != null ? capture_interval : 300);
				res.AppendFormat ("maximagestocapture=\"{0}\"\n", max_images_to_capture != null ? max_images_to_capture : 15);
				res.AppendFormat ("initialdelay=\"{0}\"\n", InitialDelay);
				if (capture_x != null)
					res.AppendFormat ("capturex=\"{0}\"\n", capture_x);
				if (capture_y != null)
					res.AppendFormat ("capturey=\"{0}\"\n", capture_y);
				res.AppendFormat ("capturewidth=\"{0}\"\n", capture_width != null ? capture_width : ResultWidth);
				res.AppendFormat ("captureheight=\"{0}\"", capture_height != null ? capture_height : ResultHeight);
			}
			res.AppendLine (">\n</embed>");

			return res.ToString ();
		}
	}
}

