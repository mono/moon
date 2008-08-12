/*
 * Property.cs.
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
using System.Text;

class Annotation {
	public string Name;
	public string Value;
	public Annotation (string Name, string Value)
	{
		this.Name = Name;
		this.Value = Value;
	}
	public Annotation (string Name) : this (Name, null)
	{
	}
}

class Annotations : Dictionary <string, Annotation> {
	public void Add (string args)
	{
		//
		// The format is like: /* @... */
		// Where ... is:
		//  key1[=value1],key2=[value2]
		// 
		// Special characters are: '=', ',' and '\' (only)
		// '\' is used to escape whenever necessary.
		// 
		// Trailing spaces before the ending */ are removed.
		// Quotes are removed from the value if present
		// Spaces between properties are *not* removed.
		// Which means that if you need a trailing space in a value, do something like:
		// /* @Property1=Value1 ,Dummy */
		// 
		StringBuilder key = new StringBuilder ();
		StringBuilder value = new StringBuilder ();
		bool found_key = false;
		bool quoted = false;
		
		// The string we're passed here does not have the /* and */
		
		for (int i = 0; i < args.Length; i++) {
			char input = args [i];
			bool escaped = false;
			if (input == '\\' && i + 1 < args.Length) {
				input = args [i + 1];
				i++;
				escaped = true;
			}
			
			if (!escaped && input == '=') {
				if (key.Length == 0 || found_key)
					throw new Exception (string.Format ("Invalid format for metadata at position {1} '{2}': '{0}' (key.Length: {3})", args, i, input, key.Length));
				
				found_key = true;
				quoted = false;
				if (i + 1 < args.Length && args [i + 1] == '"') {
					quoted = true;
				 	i++;
				}
			} else if (!escaped && (input == ',' || (quoted && input == '"' && value.Length > 0))) {
				if (key.Length == 0)
					throw new Exception (string.Format ("Invalid format for metadata at position {1} '{2}': '{0}' (found_key: {3})", args, i, input, found_key));
				
				Add (new Annotation (key.ToString (), value.Length > 0 ? value.ToString () : null));
				
				if (quoted)
					i++;
				
				if (i + 1 < args.Length && args [i] != ',')
					throw new Exception (string.Format ("Invalid format for metadata at position {1} '{2}': '{0}', expected ','", args, i, input));
				
				found_key = false;
				quoted = false;
				key.Length = 0;
				value.Length = 0;
			} else {
				if (found_key)
					value.Append (input);
				else
					key.Append (input);
			}
		}
		
		if (key.Length != 0)
			Add (new Annotation (key.ToString (), value.Length > 0 ? value.ToString () : null));
		
	}
	
	public void Add (Annotation p)
	{
		//Console.WriteLine ("Added metadata: '{0}' = '{1}'", p.Name, p.Value == null ? "null" : p.Value);
		base.Add (p.Name, p);
	}
	
	public string GetValue (string name)
	{
		Annotation property;
		if (!TryGetValue (name, out property))
			return null;
		if (property != null)
			return property.Value;
		return null;
	}
	
	public void Dump ()
	{
		foreach (KeyValuePair <string, Annotation> p in this) {
			if (p.Value == null)
				Console.WriteLine ("/* @{0}*/", p.Key);
			else
				Console.WriteLine ("/* @{0}={1}*/", p.Key, p.Value.Value);
		}
	}
}
