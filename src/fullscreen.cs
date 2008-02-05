using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;

class convert {
	static int Main (string [] args)
	{
		try {
			string sourcefile;
			if (args.Length == 0)
				sourcefile = "fullscreen.xaml";
			else
				sourcefile = args [0];
			string destfile = Path.ChangeExtension (sourcefile, ".h");
			string [] source = File.ReadAllLines (sourcefile);
			List<string> dest = new List <string> ();

			dest.Add (@"/*
 * fullscreen.h: the xaml for the fullscreen message.
 *
 * Author:
 *  Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */");

			dest.Add ("#define FULLSCREEN_MESSAGE \\");

			for (int i = 0; i < source.Length; i++) {
				dest.Add ("\"" + source [i].Replace ("\"", "\\\"") + "\\n\" \\");
			}
			dest.Add ("\"\"");
			dest.Add ("");
			File.WriteAllLines (destfile, dest.ToArray ());

			return 0;
		} catch (Exception ex) {
			Console.WriteLine (ex.Message);
			Console.WriteLine (ex.StackTrace);
			return 1;
		}
	}
}
