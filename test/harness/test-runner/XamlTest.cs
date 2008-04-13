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
using System.Resources;
using System.Reflection;


namespace MoonlightTests {

	public class XamlTest {

		public static Test Create (string id, string input_file, string master_file, XmlNode node)
		{
			string generated_html = String.Concat (input_file, ".g.htm");

			//
			// For now, let's regen every time, in the future we can enable this optimization
			//
			//if (File.Exists (generated_html)
			//
			
			GenerateHtmlFile (generated_html, input_file);
			Test test = new HtmlTest (id, generated_html, master_file, node);

			// Some tests dont actually have xaml files
			if (!File.Exists (input_file))
				test.SetToIgnore (String.Format ("Xaml file does not exist: {0}", input_file));

			return test;
		}

		private static void GenerateHtmlFile (string path, string input_file)
		{
			string html = GenerateHtmlFileContents (input_file);

			File.WriteAllText (path, html);
		}

		private static string GenerateHtmlFileContents (string input_file)
		{
			StringBuilder res = new StringBuilder ();
			StreamReader reader = new StreamReader (Assembly.GetExecutingAssembly ().GetManifestResourceStream ("html_template.html"));

			string line;
			while ((line = reader.ReadLine ()) != null) {
				if (line.Contains ("$SOURCE"))
					res.AppendLine (line.Replace ("$SOURCE", Path.GetFileName (input_file)));
				else
					res.AppendLine (line);
			}

			return res.ToString ();
		}
	}
}


