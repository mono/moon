//
// xaml2html.cs: Tool to embed xaml in html
//
// Takes a given xaml file and embeds it in a simple stand-alone html page.
//
// Authors:
//   Michael Dominic K. (mdk@mdk.am)
//   Sebastien Pouliot  <sebastien@ximian.com>
//
//
// See LICENSE file in the Moonlight distribution for licensing details

using System;
using System.Collections.Generic;
using System.IO;

class XamlToHtml {

	static readonly string html_template =  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" + 
						"<head>\n" +
						"<title>@TITLE@</title>\n" +
						"<meta>@META@</meta>\n" +
						"</head>\n" +
						"<body>\n" +
						"<object type=\"application/x-silverlight\" data=\"data:,\" id=\"slControl\" width=\"640\" height=\"480\">\n" +
						"<param name=\"background\" value=\"#FFFFFF\"/>\n" + 
						"<param name=\"source\" value=\"#xamlContent\"/>\n" +
						"</object>\n" + 
						"<script type=\"text/xaml\" id=\"xamlContent\">\n" +
						"<?xml version=\"1.0\"?>\n" + 
						"@XAML@" + 
						"</script>\n" + 
						"</body>\n" + 
						"</html>\n";

	static int verbose = 0;

	static void Help ()
	{
		Console.WriteLine ("Usage is: xaml2html [--v] [--chain] [file.xaml ...]\n");
	}

	static bool ProcessFile (string file, string next)
	{
		try {
			string xaml_basename = Path.GetFileNameWithoutExtension (file);
			string xaml_content = File.ReadAllText (file);
			string html_content = html_template;

			// Substitute
			html_content = html_content.Replace ("@XAML@", xaml_content);
			html_content = html_content.Replace ("@TITLE@", xaml_basename);

			if (next != null) {
				next = Path.GetFileNameWithoutExtension (next) + ".html";
				string refresh = String.Format ("<meta http-equiv=\"refresh\" content=\"5;URL={0}\">", next);
				html_content = html_content.Replace ("@META@", refresh);
			} else {
				html_content = html_content.Replace ("@META@", String.Empty);
			}
		
			File.WriteAllText (xaml_basename + ".html", html_content);

			if (verbose > 0)
				Console.WriteLine ("Written {0}", xaml_basename + ".html");

			return true;
		}
		catch (Exception e) {
			if (verbose > 0)
				Console.WriteLine ("Failed to embed {0}, file missing?", file);
			if (verbose > 1)
				Console.WriteLine ("Exception: {0}", e);
			return false;
		}
	}

	static int Main (string [] args)
	{
		bool chain = false;

		if (args.Length < 1){
			Help ();
			return 1;
		}

		List<string> files = new List<string> ();
		foreach (string arg in args) {
			switch (arg) {
			case "--v":
				verbose++;
				break;
			case "--chain":
				chain = true;
				break;
			default:
				files.Add (arg);
				break;
			}
		}

		for (int i=0; i < files.Count; i++) {
			string next = (chain && (i < files.Count - 1)) ? files [i + 1] : null;
			if (!ProcessFile (files [i], next))
				return 127;
		}

		return 0;	
	}
}
