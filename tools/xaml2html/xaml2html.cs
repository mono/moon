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
using System.Xml;
using System.Reflection;

using NDesk.Options;

class XamlToHtml {

	static readonly string html_template =  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" + 
						"<head>\n" +
						"<title>@TITLE@</title>\n" +
						"<meta>@META@</meta>\n" +
						"</head>\n" +
						"<body bgcolor=\"#eeeeee\">\n" +
						"<object type=\"application/x-silverlight\" data=\"data:,\" id=\"slControl\" width=\"@WIDTH@\" height=\"@HEIGHT@\">\n" +
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
		Console.WriteLine ("Usage is: xaml2html [--v] [--chain] [file.[xaml|.xap] ...]|\n");
	}

	static string FindMasterCanvasAttribute (string xml, string attribute, string def)
	{
		try {
			XmlDocument document = new XmlDocument ();
			document.LoadXml (xml);

			XmlNode node = document.GetElementsByTagName ("Canvas") [0];
			return node.Attributes [attribute].InnerText;
                } catch {
		}

		// Failed, return default
		return def;
	}

	static bool ProcessFile (string file, string next)
	{
		try {
			bool is_xap = file.EndsWith (".xap");
			string xaml_basename = Path.GetFileNameWithoutExtension (file);
			string xaml_content = File.ReadAllText (file);
			string html_content = is_xap ? (new StreamReader (Assembly.GetExecutingAssembly ().GetManifestResourceStream ("sl2template.html"))).ReadToEnd () : html_template;

			string canvas_width = FindMasterCanvasAttribute (xaml_content, "Width", is_xap ? "1600" : "640");
			string canvas_height = FindMasterCanvasAttribute (xaml_content, "Height", is_xap ? "1200" : "480");

			// Substitute
			html_content = html_content.Replace ("@XAML@", xaml_content);
			html_content = html_content.Replace ("@XAP_FILE@", Path.GetFileName (file));
			html_content = html_content.Replace ("@TITLE@", xaml_basename);
			html_content = html_content.Replace ("@WIDTH@", canvas_width);
			html_content = html_content.Replace ("@HEIGHT@", canvas_height);

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

		var p = new OptionSet () {
			{ "-v", v => verbose++ },
			{ "-chain", v => chain = true }
		};

		List<string> files = null;
		try {
			files = p.Parse (args);
		} catch (OptionException) {
			Console.WriteLine ("Try `xaml2html --help' for more information.");
			return 1;
		}

		for (int i=0; i < files.Count; i++) {
			string next = (chain && (i < files.Count - 1)) ? files [i + 1] : null;
			if (!ProcessFile (files [i], next))
				return 127;
		}

		return 0;	
	}
}
