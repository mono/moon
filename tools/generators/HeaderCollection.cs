/*
 * HeaderCollection.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

class HeaderCollection
{
	private Dictionary <int, List<string>> all = new Dictionary<int,List<string>> ();
	
	public void Add (string header, int version)
	{
		List<string> headers;
		
		header = Path.GetFileName (header);
		
		if (!all.TryGetValue (version, out headers)) {
			headers = new List<string> ();
			all.Add (version, headers);
		}
		
		if (!headers.Contains (header))
			headers.Add (header);
	}
	
	public void Write (StringBuilder text)
	{
		int version;
		foreach (KeyValuePair <int, List<string>> pair in all) {
			version = pair.Key;
			if (version > 1) {
				text.Append ("#if SL_");
				text.Append (version);
				text.AppendLine ("_0");
			}
			pair.Value.Sort ();
			foreach (string header in pair.Value) {
				text.Append ("#include \"");
				text.Append (header);
				text.AppendLine ("\"");
			}
			
			if (version > 1)
				text.AppendLine ("#endif");
		}
	}
}