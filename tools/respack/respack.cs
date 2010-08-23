//
// Packs a bunch of files as a resource
//
// Used to bundle all the xaml files into the APP.g.resources file
//
// Copyrigh 2008 Novell, Inc.
//
using System;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Resources;
using System.IO;
using System.Linq;
using NDesk.Options;

class ResourcePacker {

	static void ShowHelp (OptionSet os)
	{
		Console.WriteLine ("Usage: respack outputfile [file1[,name] .. [fileN]]");
		Console.WriteLine ();
		os.WriteOptionDescriptions (Console.Out);
		Environment.Exit (0);
	}
	
	static bool verbose = false;

	public static int Main (string [] args)
	{
		bool help = false;
		bool unpack = false;
		string pattern = @"^.+\.xaml?";

		var p = new OptionSet () {
			{ "h|?|help", v => help = v != null },
			{ "u|unpack", "Extract resources from the supplied assembly.", v => unpack = v != null  },
			{ "p|pattern=", "Only extract the resources that match supplied pattern. By default only .xaml files will be extracted.", v => pattern = v  },
			{ "v|verbose", v=> verbose = v != null }
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

		if (unpack)
			return Unpack (files [0], pattern);

		if (files == null || files.Count == 0)
			ShowHelp (p);

		return Pack (files);
	}
	
	public static int Pack (List<string> files)
	{
		IResourceWriter output = null;
		try {
			if (files [0].EndsWith (".resx")) {
				output = new ResXResourceWriter (files [0]);
			} else {
				output = new ResourceWriter (files [0]);
			}
		} catch {
			Console.WriteLine ("Error creating {0} file", files [0]);
		}
		

		int i = 1;
		while (i < files.Count) {
			string f = files [i++];
			string key;
			string file = f;
			
			if (f.StartsWith ("@")) {
				files.AddRange (System.IO.File.ReadAllLines (f.Substring (1)));
				continue;
			}
			
			int comma = file.IndexOf (',');
			if (comma != -1) {
				key = file.Substring (comma + 1);
				file = file.Substring (0, comma);
			} else {
				key = Path.GetFileName (file);
			}

			
			using (FileStream source = File.OpenRead (file)){
				byte [] buffer = new byte [source.Length];
				source.Read (buffer, 0, (int) source.Length);

				// Sadly, we cant just pass byte arrays to resourcewriter, we need
				// to wrap this on a MemoryStream

				MemoryStream value = new MemoryStream (buffer);

				output.AddResource (Uri.EscapeUriString (key).ToLowerInvariant (), value);
			}
		}

		output.Generate ();
		return 0;
	}

	public static int Unpack (string assembly, string pattern)
	{
		Assembly asm = null;
		try {
			asm = Assembly.LoadFile (assembly);
		} catch (Exception e) {
			Console.Error.WriteLine ("Unable to load assembly: {0}.", assembly);
			Console.Error.WriteLine (e);
			return 1;
		}

		string [] resources = asm.GetManifestResourceNames ();
	
		foreach (string resource in resources) {
			if (verbose)
				Console.WriteLine ("decompressing '{0}'", resource);

			ResourceReader reader = null;

			try {
				using (reader = new ResourceReader (asm.GetManifestResourceStream (resource))) {
				
					IDictionaryEnumerator id = reader.GetEnumerator (); 

					while (id.MoveNext ()) {
						string key = (string) id.Key;
						if (!Regex.IsMatch (key, pattern))
							continue;

						if (verbose)
							Console.WriteLine ("  stream: {0}", key);

						MemoryStream stream = id.Value as MemoryStream;
						if (stream == null) {
							Console.Error.WriteLine ("Item not stored as a MemoryStream. {0}", key);
							continue;
						}

						byte [] data = new byte [stream.Length];
						stream.Read (data, 0, data.Length);

						string dir = Path.GetDirectoryName (key);
						if (!String.IsNullOrEmpty (dir))
							Directory.CreateDirectory (dir);

						using (FileStream fs = File.OpenWrite (key)) {
							fs.Write (data, 0, data.Length);
						}
									      
					}
				}
			} catch (Exception e) {
				Console.WriteLine ("failed to decompress {0}, exception '{1}'.. skipping", resource, e.Message);
			}
		}

		return 0;
	}
}
