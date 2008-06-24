using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text;

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
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */");

			dest.Add ("#define FULLSCREEN_MESSAGE \\");

			for (int i = 0; i < source.Length; i++) {
				int j;
				StringBuilder sb = new StringBuilder ();
				/* put the leading whitespace in the file */
				for (j = 0; j < source[i].Length; j ++) {
					if (!Char.IsWhiteSpace (source[i], j))
						break;
				}
				if (j > 0) sb.Append (source[i].Substring (0, j));
				sb.Append ("\"" + source [i].Substring (j).Replace ("\"", "\\\"") + (source[i][source[i].Length-1] == '>' ? "" : " ") + "\" \\");
				dest.Add (sb.ToString());
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
