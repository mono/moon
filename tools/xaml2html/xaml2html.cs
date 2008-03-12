//
// xaml2html.cs: Tool to embed xaml in html
//
// Takes a given xaml file and embeds it in a simple stand-alone html page.
//
// Author:
//   Michael Dominic K. (mdk@mdk.am)
//
//
// See LICENSE file in the Moonlight distribution for licensing details

using System;
using System.IO;

class XamlToHtml {

	static readonly string html_template =  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" + 
						"<head>\n" +
						"<title>@TITLE@</title>\n" +
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

	static void Help ()
	{
		Console.WriteLine ("Usage is: xaml2html [file.xaml]\n");
	}

	static int Main (string [] args)
	{
		if (args.Length < 1){
			Help ();
			return 1;
		}

		try {
			string xaml_basename = Path.GetFileNameWithoutExtension (args [0]);
			string xaml_content = File.ReadAllText (args [0]);
			string html_content = html_template;
		
			// Substitute
			html_content = html_content.Replace ("@XAML@", xaml_content);
			html_content = html_content.Replace ("@TITLE@", xaml_basename);

			File.WriteAllText (xaml_basename + ".html", html_content);

			Console.WriteLine ("Written {0}", xaml_basename + ".html");

		} catch {
			Console.WriteLine ("Failed to embed {0}, file missing?", args [0]);
			return 127;
		}

		return 0;	
	}

}


