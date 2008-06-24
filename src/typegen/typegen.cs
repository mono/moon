/*
 * typegen.cs: Code generator for the type system
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Text;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;

class TypeInfo {
	public string Name; // The name as it appears in code (char*, Point[], etc)
	private string _KindName; // The name as it appears in the Kind enum (STRING, POINT_ARRAY, etc)
	public string Base; // The parent type
	public string Code; // The code in this class
	public string C_Constructor; // The C constructor
	public bool IsStruct; // class or struct
	public string Header; // The .h file where the class was defined
	public int Line;
	public string ContentProperty;
	public int SilverlightVersion;
	public List<string> Events = new List<string> ();
	public int TotalEventCount;
	public Types types;
	public bool Include; // Force inclusion of this type into the type system (for manual types, char, point[], etc)
	public bool IsValueType;
	public bool ImplementsGetObjectType;
	
	public bool IsNested {
		get {
			return Name.Contains ("::");
		}
	}
	
	public int GetEventId (string Event)
	{
		if (Events != null && Events.Contains (Event)) {
			return Events.IndexOf (Event) + GetBaseEventCount ();
		} else {
			return types [Base].GetEventId (Event);
		}
	}
	
	public string KindName {
		get {
			if (_KindName != null)
				return _KindName;
			if (Name == null || Name == string.Empty)
				return "INVALID";
			return Generator.getU (Name);
		} 
		set {
			_KindName = value;
		}
	}
			
	public int GetBaseEventCount ()
	{
		if (Base == null || Base == string.Empty || !types.ContainsKey (Base))
			return 0;
		else
			return types [Base].GetTotalEventCount ();
	}
	
	public int GetTotalEventCount ()
	{
		return Events.Count + GetBaseEventCount ();
	}
	
	public void Generate ()
	{
		// C constructor
		for (int i = 0; i < Name.Length; i++) {
			if (char.ToUpper (Name [i]) == Name [i] && C_Constructor != null)
				C_Constructor += "_" + char.ToLower (Name [i]);
			else
				C_Constructor += char.ToLower (Name [i]);
		}
		C_Constructor += "_new";
		
		if (!types.contents.Contains (" " + C_Constructor) && !types.contents.Contains ("*" + C_Constructor))
			C_Constructor = null;
		
		// Content property
		if (Code != null) {
			Match m = Regex.Match (Code, ".*@ContentProperty\\s*=\\s*\"(.*)\"");
			if (m.Success) {
				ContentProperty = m.Groups [1].Value;
			}
		}
		
		// Silverlight version
		SilverlightVersion = 1;

		if (Code != null) {
			Match m = Regex.Match (Code, ".*@SilverlightVersion\\s*=\\s*\"([0-9])\"");
			if (m.Success) {
				if (!Int32.TryParse (m.Groups[1].Value, out SilverlightVersion))
					Console.WriteLine ("failed to parse silverlight version {0}, please specify 1 or 2.", m.Groups[1].Value);
			}
		}

		// Events 
		foreach (string l in Code.Split ('\n')) {
			string line = l;
			line = line.Trim ();
			if ((line.StartsWith ("static int ") || line.StartsWith ("const static int")) && line.EndsWith ("Event;")) {
				line = line.Replace ("static int", "");
				line = line.Replace ("const", "");
				line = line.Replace ("Event;", "");
				line = line.Replace (" ", "");
				Events.Add (line);
				//Console.WriteLine ("Found event {0} for type {1}", line, Name);
			}
		}
		Events.Sort ();
		
		// ImplementsGetObjectType
		ImplementsGetObjectType = Code.Contains ("GetObjectType");
	}
		
	public bool Is (string name)
	{
		if (Name == name)
			return true;
		if (Name == string.Empty || Name == null || Base == string.Empty)
			return false;
		if (types.ContainsKey (Base))
			return types [Base].Is (name);
		else
			return false;
	}
	
	public bool IsEventObject ()
	{
		return Is ("EventObject");
	}
	
	public TypeInfo ()
	{
	}
	
	public TypeInfo (string Name, string KindName, string Base, bool Include)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = Base;
		this.Include = Include;
	}
	
}

class Types : Dictionary <string, TypeInfo> {
	public string contents;

	TypeInfo [] sorted;
	
	public Types () : base (StringComparer.InvariantCulture)
	{
		
	}
	
	public IEnumerable <TypeInfo> SortedList {
		get {
			if (sorted == null) {
				int i = 0;
				sorted = new TypeInfo [Count];
				foreach (TypeInfo type in this.Values)
					sorted [i++] = type;
				Array.Sort (sorted, delegate (TypeInfo x, TypeInfo y) {
					return string.Compare (x.KindName, y.KindName);
				});
			}
			return sorted;
		}
	}
	
	public void Add (TypeInfo tp)
	{
		if (!ContainsKey (tp.Name)) {
			base.Add (tp.Name, tp);
		} else {
			//We already found this type while parsing the headers, so just set the necessary info.
			this [tp.Name].Include = tp.Include;
			this [tp.Name].KindName = tp.KindName;
		}
		sorted = null;
	}
	
	public StringBuilder GetKindsForEnum ()
	{
		StringBuilder text = new StringBuilder ();
		foreach (TypeInfo type in SortedList) {
			if (!type.ImplementsGetObjectType && !type.Include)
				continue;

			text.AppendLine ("\t\t" + type.KindName + "," + (type.SilverlightVersion == 2 ? "// Silverlight 2.0 only" : ""));
		}
		return text;
	}
	
}

class Generator {
#region Helper methods
	static void Main ()
	{
		Types types = new Types ();

		GetTypes (types);

		GenerateTypeStaticCpp (types);
		
		GenerateValueH (types);	
		GenerateTypeH (types);
		GenerateKindCs (types);
	}
	
	static string RemoveComments (string v)
	{
		int a, b;
		a = v.IndexOf ("/*");
		while (a >= 0) {
			b = v.IndexOf ("*/", a + 2);
			if (v.IndexOf ("/* @", a) != a)
				v = v.Remove (a, b - a + 2);
			else
				a = b;
			a = v.IndexOf ("/*", a + 2);
		}
		return v;
	}

	public static string getU (string v)
	{
		if (v.Contains ("::"))
			v = v.Substring (v.IndexOf ("::") + 2);

		v = v.ToUpper ();
		v = v.Replace ("DEPENDENCYOBJECT", "DEPENDENCY_OBJECT");
		if (v.Length > "COLLECTION".Length)
			v = v.Replace ("COLLECTION", "_COLLECTION");
		if (v.Length > "DICTIONARY".Length)
			v = v.Replace ("DICTIONARY", "_DICTIONARY");
		return v;
	}
	
	static void GetTypes (Types types)
	{
		Stack<int> current_type_brackets = new Stack<int> ();
		Stack<TypeInfo> current_types = new Stack<TypeInfo> ();
		StringBuilder all = new StringBuilder ();
		string contents;
		bool ifdefed_out = false;
		int current_brackets = 0;
		string file = "";
		int linenumber = 0;
		TypeInfo current_type = null;

		foreach (string f in Directory.GetFiles (Environment.CurrentDirectory, "*.h")) {
			all.AppendLine ("#file " + f);
			all.AppendLine (File.ReadAllText (f));
		}

		types.contents = all.ToString ();
		
		contents = types.contents;
		contents = RemoveComments (contents);

		string [] lines = contents.Split (new char [] {'\n', '\r'});
		
		for (int i = 0; i < lines.Length; i++) {
			string l = lines [i];
			bool is_class;
			linenumber++;
			if (l.StartsWith ("#if 0") || l.StartsWith ("#if false")) {
				ifdefed_out = true;
				continue;
			} else if (ifdefed_out && l.StartsWith ("#endif")) {
				ifdefed_out = false;
				continue;
			} else if (ifdefed_out) {
				continue;
			} else if (l.StartsWith ("#file ")) {
				file = l.Substring (6);
				ifdefed_out = false;
				linenumber = 0;
				continue;
			}
			
			current_type = null;
			if (current_types.Count != 0)
				current_type = current_types.Peek ();
			if (current_type != null) {
				for (int k = 0; k < l.Length; k++) {
					if (l [k] == '{')
						current_brackets++;
					else if (l [k] == '}')
						current_brackets--;
				}
				current_type.Code += l + "\n";
				if (current_brackets <= 0) {
					current_types.Pop ();
					current_brackets = current_type_brackets.Pop ();
				}
			}
			
			l = l.Trim ();
			
			if (l.EndsWith (";"))
				continue;
			
			if (!l.StartsWith ("class ") && !l.StartsWith ("struct "))
				continue;
			

			is_class = l.StartsWith ("class ");
			
			l = l.Replace ("{", "");
			l = l.Replace ("class ", "");
			l = l.Replace ("struct ", "");
			l = l.Replace (";", "");
			l = l.Replace ("public", "");
			l = l.Replace ("\t", "");
			l = l.Replace (" ", "");

			string c, p;
			if (l.Contains (":")) {
				c = l.Substring (0, l.IndexOf (":")).Trim ();
				p = l.Substring (l.IndexOf (":") + 1).Trim ();
			} else {
				c = l;
				p = "";
			}
			
			if (c.Trim () == string.Empty) {
				//Console.WriteLine ("Found struct/class with no name: {0} at {1}:{2}", lines [i], file, linenumber);
				continue;
			}
			
			if (current_types.Count != 0)
				c = current_types.Peek ().Name + "::" + c;
			
			//Console.WriteLine ("/********* {0} " + c + " : public " +p + " *********/", is_class ? "class" : "struct");
			if (types.ContainsKey (c)) {
				TypeInfo ti = types [c];
				Console.WriteLine ("There is already a type named " + c + ", found in " + ti.Header + "(" + ti.Line.ToString () + "), this one in " + file + "(" + linenumber.ToString () + ")");
				continue;
			}
			
			TypeInfo t = new TypeInfo ();
			t.types = types;
			t.Name = c;
			t.Base = p;
			t.Header = System.IO.Path.GetFileName (file);
			t.Line = linenumber;
			t.IsStruct = !is_class;
			t.Code = lines [i] + "\n";
			for (int k = i - 1; k >= 0; k--) {
				// Look backwards for content properties, include them in the code
				if (lines [k].StartsWith ("/*"))
					t.Code = lines [k] + "\n" + t.Code;
				else
					break;
			}
			types.Add (t);
			current_type = t;
			current_brackets = 1;
			current_types.Push (t);
			current_type_brackets.Push (current_brackets);
		}
		
		foreach (TypeInfo t in types.Values)
			t.Generate ();
		
		// Add all the manual types
		types.Add (new TypeInfo ("bool", "BOOL", null, true));
		types.Add (new TypeInfo ("double", "DOUBLE", null, true));
		types.Add (new TypeInfo ("guint64", "UINT64", null, true));
		types.Add (new TypeInfo ("gint64", "INT64", null, true));
		types.Add (new TypeInfo ("guint32", "UINT32", null, true));
		types.Add (new TypeInfo ("gint32", "INT32", null, true));
		types.Add (new TypeInfo ("char*", "STRING", null, true));
		types.Add (new TypeInfo ("Color", "COLOR", null, true));
		types.Add (new TypeInfo ("Point", "POINT", null, true));
		types.Add (new TypeInfo ("Rect", "RECT", null, true));
		types.Add (new TypeInfo ("RepeatBehavior", "REPEATBEHAVIOR", null, true));
		types.Add (new TypeInfo ("Duration", "DURATION", null, true));
		types.Add (new TypeInfo ("TimeSpan", "TIMESPAN", null, true));
		types.Add (new TypeInfo ("KeyTime", "KEYTIME", null, true));
		types.Add (new TypeInfo ("double*", "DOUBLE_ARRAY", null, true));
		types.Add (new TypeInfo ("Point*", "POINT_ARRAY", null, true));
		types.Add (new TypeInfo ("NPObj", "NPOBJ", null, true));
		types.Add (new TypeInfo ("GridLength", "GRIDLENGTH", null, true));
	}
	
	static void WriteAllText (string filename, string contents)
	{
		if (File.ReadAllText (filename) != contents) {
			File.WriteAllText (filename, contents);
			Console.WriteLine ("Wrote {0}.", filename);
		} else {
			Console.WriteLine ("Skipped writing {0}, no changes.", filename);
		}
	}
#endregion
		
	static void GenerateTypeStaticCpp (Types types)
	{
		List<string> headers = new List<string> ();
		List<string> headers2 = new List<string> ();
		
		StringBuilder text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine (" */");
		text.AppendLine ("#include <config.h>");
		text.AppendLine ("#include <stdlib.h>");

		foreach (TypeInfo t in types.SortedList) {
			if (t.C_Constructor == string.Empty || t.C_Constructor == null)
				continue;

			if (t.SilverlightVersion == 1) {
				if (headers2.Contains (t.Header))
					throw new Exception (string.Format ("header {0} contains both a 1.0 and 2.0 class", t.Header));
				else if (!headers.Contains (t.Header))
					headers.Add (t.Header);
			}
			else if (t.SilverlightVersion == 2) {
				if (headers.Contains (t.Header))
					throw new Exception (string.Format ("header {0} contains both a 1.0 and 2.0 class", t.Header));
				else if (!headers2.Contains (t.Header))
					headers2.Add (t.Header);
			}
		}
		
		// Loop through all the classes and check which headers
		// are needed for the c constructors
		text.AppendLine ("");
		foreach (string h in headers)
			text.AppendLine ("#include \"" + h + "\"");
		
		text.AppendLine ("#if SL_2_0");
		foreach (string h in headers2)
			text.AppendLine ("#include \"" + h + "\"");
		text.AppendLine ("#endif");
		
		// Set the event ids for all the events
		text.AppendLine ("");
		foreach (TypeInfo t in types.SortedList) {
			if (t.Events == null || t.Events.Count == 0)
				continue;
			
			foreach (string e in t.Events) {
				text.AppendLine (string.Format ("const int {0}::{1}Event = {2};", t.Name, e, t.GetEventId (e)));
			}
		}

		// Create the arrays of event names for the classes which have events
		text.AppendLine ("");
		foreach (TypeInfo t in types.SortedList) {
			if (t.Events == null || t.Events.Count == 0)
				continue;
			
			string events = "NULL";
			if (t.Events != null && t.Events.Count != 0){
				for (int k = t.Events.Count - 1; k >= 0; k--) {
					events = "\"" + t.Events [k] + "\", " + events;
				}
			}
			text.AppendLine (string.Format ("const char *{0}_Events [] = {{ {1} }};", 
			                                t.Name, 
			                                events));
		}

		// Create the array of type data
		text.AppendLine ("");
		text.AppendLine ("Type type_infos [] = {");
		text.AppendLine ("\t{ Type::INVALID, Type::INVALID, false, \"INVALID\", NULL, 0, 0, NULL, NULL, NULL },");
		foreach (TypeInfo type in types.SortedList) {
			TypeInfo parent = null;
			string events = "NULL";
			
			if (!type.ImplementsGetObjectType && !type.Include)
				continue;
			
			if (type.Base != null && types.ContainsKey (type.Base))
				parent = types [type.Base];
			
			if (type.Events != null && type.Events.Count != 0)
				events = type.Name + "_Events";

			if (type.SilverlightVersion == 2)
				text.AppendLine ("#if SL_2_0");
			text.AppendLine (string.Format (@"	{{ {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9} }}, ",
			                                "Type::" + type.KindName, 
			                                "Type::" + (parent != null ? parent.KindName : "INVALID"),
			                                type.IsValueType ? "true" : "false",
			                                "\"" + type.Name + "\"", 
			                                "\"" + type.KindName + "\"", 
			                                type.Events.Count,
			                                type.GetTotalEventCount (),
			                                events,
			                                type.C_Constructor != null ? string.Concat ("(create_inst_func *) ", type.C_Constructor) : "NULL", 
			                                type.ContentProperty != null ? string.Concat ("\"", type.ContentProperty, "\"") : "NULL"
			                                )
			                 );
			if (type.SilverlightVersion == 2) {
				text.AppendLine ("#else");
				text.AppendLine (string.Format ("	{{ Type::INVALID, Type::INVALID, false, \"2.0 specific type '{0}'\", {1}, 0, 0, NULL, NULL, NULL }}, ",
								type.KindName,
								"\"" + type.KindName + "\""));
				text.AppendLine ("#endif");
			}
			
		}
		text.AppendLine ("\t{ Type::LASTTYPE, Type::INVALID, false, NULL, NULL, 0, 0, NULL, NULL, NULL }");
		text.AppendLine ("};");
				
		WriteAllText ("type-generated.cpp", text.ToString ());
	}
	
	static void GenerateTypeH (Types types)
	{
		const string file = "type.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");
		
		contents = contents.Replace ("/*DO_KINDS*/", types.GetKindsForEnum ().ToString ());

		text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from type.h.in, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine (" */");
		contents = text.ToString () + contents;

		WriteAllText (file, contents);
	}

	static void GenerateKindCs (Types types)
	{
		const string file = "type.h";
		StringBuilder text = new StringBuilder ();
		string contents = File.ReadAllText (file);
		int a = contents.IndexOf ("// START_MANAGED_MAPPING") + "// START_MANAGED_MAPPING".Length;
		int b = contents.IndexOf ("// END_MANAGED_MAPPING");
		string values = contents.Substring (a, b - a);		

		text.AppendLine ("/* \n\tthis file was autogenerated from moon/src/value.h.  do not edit this file \n */");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tpublic enum Kind {");
		text.AppendLine (values);
		text.AppendLine ("\t}");
/*
		text.AppendLine (@"
	public class NativeSupport {
		private static int [] native_to_managed;
		private static int [] managed_to_native;
		
		static NativeSupport ()
		{
			managed_to_native = new int [(int) Kind.LASTTYPE];
			native_to_managed = new int [NativeMethods.type_get_kind (""LASTTYPE"")];
		
			for (int i = 0; i < managed_to_native.Length; i++) {
				managed_to_native [i] = NativeMethods.type_get_kind (((Kind) i).ToString ());
			}
	
			for (int i = 0; i < native_to_managed.Length; i++) {
				Kind kind;
				try {
					kind = (Kind) System.Enum.Parse (typeof (Kind), NativeMethods.type_get_name ((Kind) i));
				} catch {
					kind = Kind.INVALID;
				}
				managed_to_native [i] = (int) kind;
			}
		}
	}
");
		                 
*/
		text.AppendLine ("}");
		File.WriteAllText ("Kind.cs", text.ToString ());

		string realfile = "../class/Mono.Moonlight/Mono/Kind.cs";
		realfile = realfile.Replace ('/', Path.DirectorySeparatorChar);
		realfile = Path.GetFullPath (realfile);
		try {
			File.Copy ("Kind.cs", realfile, true);
			File.Delete ("Kind.cs");

			string svn;
			svn =  Path.Combine (Path.GetDirectoryName (realfile), ".svn/text-base/Kind.cs.svn-base".Replace ('/', Path.DirectorySeparatorChar));
			if (!File.Exists (svn) || string.CompareOrdinal (File.ReadAllText (realfile), File.ReadAllText (svn)) != 0) {
				Console.WriteLine ("The file '{0}' has been updated, don't forget to commit the changes.", realfile);
			}
		} catch {
			Console.WriteLine ("You need to update the file 'Kind.cs' in the 'moon/class/Mono.Moonlight/Mono/' directory with the Kind.cs file generated here");
		}
	}
	
	static void GenerateValueH (Types types)
	{
		const string file = "value.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");

		text = new StringBuilder ();
		foreach (TypeInfo type in types.SortedList) {
			if (!type.ImplementsGetObjectType || type.IsNested)
				continue;
			
			if (type.IsStruct) {
				text.AppendLine ("struct " + type.Name + ";");				
			} else {
				text.AppendLine ("class " + type.Name + ";");				
			}
		}
		contents = contents.Replace ("/*DO_FWD_DECLS*/", text.ToString ());
		
		contents = contents.Replace ("/*DO_KINDS*/", types.GetKindsForEnum ().ToString ());

		text = new StringBuilder ();
		foreach (TypeInfo type in types.SortedList) {
			if (!type.ImplementsGetObjectType || type.IsNested)
				continue;

			text.AppendLine (string.Format ("	{1,-30} As{0} () {{ checked_get_subclass (Type::{2}, {0}) }}", type.Name, type.Name + "*", type.KindName));
		}
		contents = contents.Replace ("/*DO_AS*/", text.ToString ());
		
		text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from value.h.in, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine (" */");
		contents = text.ToString () + contents;

		WriteAllText (file, contents);
	}
}
