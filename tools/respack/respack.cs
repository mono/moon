//
// Packs a bunch of files as a resource
//
// Used to bundle all the xaml files into the APP.g.resources file
//
// Copyrigh 2008 Novell, Inc.
//
using System;
using System.Collections.Generic;
using System.Resources;
using System.IO;
using System.Linq;
using NDesk.Options;

class ResourcePacker {

	static void ShowHelp (OptionSet os)
	{
		Console.WriteLine ("Usage: respack outputfile [file1 .. [fileN]]");
		Console.WriteLine ();
		os.WriteOptionDescriptions (Console.Out);
		Environment.Exit (0);
	}
	
	public static int Main (string [] args)
	{
		bool help = false;
		
		var p = new OptionSet () {
			{ "h|?|help", v => help = v != null },
		};

		List<string> files = null;
		try {
			files = p.Parse(args);
		} catch (OptionException e){
			Console.WriteLine ("Try `respack --help' for more information.");
			return 1;
		}

		if (help)
			ShowHelp (p);

		if (files == null || files.Count == 0)
			ShowHelp (p);

		ResourceWriter output = null;
		try {
			output = new ResourceWriter (files [0]);
		} catch {
			Console.WriteLine ("Error creating {0} file", files [0]);
		}
		

		foreach (string file in files.Skip(1)){
			string key = Path.GetFileName (file);
			
			using (FileStream source = File.OpenRead (file)){
				byte [] buffer = new byte [source.Length];
				source.Read (buffer, 0, (int) source.Length);

				// Sadly, we cant just pass byte arrays to resourcewriter, we need
				// to wrap this on a MemoryStream

				MemoryStream value = new MemoryStream (buffer);

				output.AddResource (key, value);
			}
		}

		output.Generate ();
		return 0;
	}
}