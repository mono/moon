using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text;

class convert {
	static int Main (string [] args)
	{
		Console.WriteLine ("hi!");

		try {
			string sourcefile = args [0];
			string destfile = args [1];
			string cpp_symbol = args [2];

			byte [] source = File.ReadAllBytes (sourcefile);
			List<string> dest = new List <string> ();

			dest.Add (string.Format (@"/*
 * {0} converted to a friendlier format for inclusion
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */", sourcefile));

			dest.Add (string.Format ("static guchar {0}[] = {{", cpp_symbol));

			StringBuilder sb = new StringBuilder ();
			for (int i = 0; i < source.Length; i++) {
				sb.AppendFormat ("0x{0:x},", source[i]);
				if (i > 0 && (i % 32 == 0)) {
					dest.Add (sb.ToString());
					sb = new StringBuilder ();
				}
			}

			dest.Add ("};");
			Console.WriteLine ("writing to {0}", destfile);
			File.WriteAllLines (destfile, dest.ToArray ());

			return 0;
		} catch (Exception ex) {
			Console.WriteLine (ex.Message);
			Console.WriteLine (ex.StackTrace);
			return 1;
		}
	}
}
