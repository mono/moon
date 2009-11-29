using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;

class asf
{
	class Field {
		public string name;
		public string type;
		public int get_size ()
		{
			switch (type) {
			case "asf_byte": return 1;
			case "asf_word": return 2;
			case "asf_dword": return 4;
			case "asf_qword": return 8;
			case "asf_guid": return 16;
			default:
				throw new NotImplementedException ("Don't know the size of " + type + "!");
			}
		}
		public string get_format ()
		{
			switch (type) {
			case "asf_byte":
			case "asf_word":
			case "asf_dword":
				return "%u";
			case "asf_qword":
				return "%\" G_GUINT64_FORMAT \"";
			case "asf_guid":
			case "const char*":
				return "%s";
			default:
				return "%p";
			}
		}
		public string get_access (string prefix)
		{
			
			switch (type) {
			case "asf_guid":
				return "asf_guid_tostring (&" + prefix + name + ")";
			case "asf_byte":
			case "asf_word":
			case "asf_dword":
				return "(asf_dword) " + prefix + name;
				//return "%i";
			case "asf_qword":
				//return "%lld";
			case "const char*":
				//return "%s";
			default:
				return prefix + name;
			}
		}
	}
	
	static int Main (string [] args)
	{
		try {
			return main (args);
		} catch (Exception ex) {
			Console.WriteLine (ex.Message);
			Console.WriteLine (ex.StackTrace);
			return 1;
		}
	}
	
	static int main (string [] args)
	{
		List<string> structs = ReadStructs (true);
		List<string> objects = ReadStructs (false);
		List<string> classes = ReadClasses ();
		
		objects.Add ("asf_object");
		
		List<string> debug_structs = new List<string> (objects);
		//debug_structs.Add ("WAVEFORMATEXTENSIBLE");
		debug_structs.Add ("WAVEFORMATEX");
		debug_structs.Add ("BITMAPINFOHEADER");
		debug_structs.Add ("asf_video_stream_data");
		
		using (StreamWriter writer = new StreamWriter ("asf-generated.h")) {
			writer.WriteLine (@"/*
 * asf-generated.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
");
			writer.WriteLine ("#ifndef _ASF_MOONLIGHT_GENERATED_H_");
			writer.WriteLine ("#define _ASF_MOONLIGHT_GENERATED_H_");
			
			writer.WriteLine ("");
			
			foreach (string str in structs) {
				writer.WriteLine ("struct " + str + ";");
			}
			writer.WriteLine ("");
			
			foreach (string str in classes) {
				writer.WriteLine ("class " + str + ";"); 
			}
			writer.WriteLine ("");

			writer.WriteLine (" /* Validation functions */ ");
			writer.WriteLine ("bool asf_object_validate_exact (const asf_object* obj, ASFParser* parser);");
			foreach (string str in structs) {
				writer.WriteLine ("bool " + str + "_validate (const " + str + "* obj, ASFParser* parser);");
			}
			writer.WriteLine ("");
			
			writer.WriteLine ("/* Debug functions */ ");
			
			foreach (string str in debug_structs) {
				if (!File.ReadAllText ("asf-structures.h").Contains ("void " + str + "_dump (")) {
					writer.WriteLine ("void " + str + "_dump (const " + str + "* obj);");
				}
			}
			writer.WriteLine ("");
			
			writer.WriteLine ("void print_sizes ();");
			writer.WriteLine ("void asf_object_dump_exact (const asf_object* obj);");
			writer.WriteLine ("");
			writer.WriteLine ("#endif");
			writer.WriteLine ("");
		}
		
		using (StreamWriter writer = new StreamWriter ("asf-generated.cpp")) {
			writer.WriteLine (@"/*
 * asf-generated.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
");
			writer.WriteLine ("#include <config.h>");
			writer.WriteLine ("#include \"asf.h\"");
			
			writer.WriteLine ("");
			writer.WriteLine ("void print_sizes () {");
			foreach (string str in objects) {
				writer.WriteLine ("\tprintf (\"sizeof ({0}) = %i.\\n\", (int) sizeof ({0}));", str);
			}
			writer.WriteLine ("}");
			writer.WriteLine ("");
			
			writer.WriteLine ("bool asf_object_validate_exact (const asf_object* obj, ASFParser* parser)");
			writer.WriteLine ("{");
			writer.WriteLine ("\tswitch (asf_get_guid_type (&obj->id)) {");
			foreach (string str in objects) {
				if (str == "asf_object")
					continue;
				writer.WriteLine ("\tcase {0}:", str.ToUpper ());
				writer.WriteLine ("\t\treturn {0}_validate (({0}*) obj, parser);", str);
			}
			// Special case a bit
			writer.WriteLine ("\tcase ASF_NONE:");
			writer.WriteLine ("\tcase ASF_LANGUAGE_LIST:");
			writer.WriteLine ("\tcase ASF_METADATA:");
			writer.WriteLine ("\tcase ASF_PADDING:");
			writer.WriteLine ("\tcase ASF_ADVANCED_MUTUAL_EXCLUSION:");
			writer.WriteLine ("\tcase ASF_STREAM_PRIORITIZATION:");
			writer.WriteLine ("\tcase ASF_INDEX_PARAMETERS:");
			writer.WriteLine ("\t\treturn true; // Do nothing, we don't use these objects at all, so there's no need to validate either.");
			writer.WriteLine ("\tdefault:");
			writer.WriteLine ("#if DEBUG");
			writer.WriteLine ("\t\tprintf (\"ASF warning: No validation implemented for %s.\\n\", asf_guid_get_name (&obj->id));");
			writer.WriteLine ("#endif");
			writer.WriteLine ("\t\treturn true;");
			writer.WriteLine ("\t}");
			writer.WriteLine ("}");
			writer.WriteLine ("");
			
			writer.WriteLine ("/* Debug functions */ ");
			foreach (string str in debug_structs) {
				if (!File.ReadAllText ("asf-structures.cpp").Contains ("void " + str + "_dump (")) {
					writer.WriteLine ("void " + str + "_dump (const " + str + "* obj)");
					writer.WriteLine ("{");
					writer.WriteLine ("\tASF_DUMP (\"" + str.ToUpper ()  + "\\n\");");
					foreach (Field field in ReadFields (str)) {
						writer.WriteLine ("\tASF_DUMP (\"\\t" + field.name + " = " + field.get_format () + "\\n\", " + field.get_access ("obj->") + ");");
					}
					writer.WriteLine ("}");
					writer.WriteLine ("");
				}
			}
			writer.WriteLine ("");

			writer.WriteLine ("void asf_object_dump_exact (const asf_object* obj)");
			writer.WriteLine ("{");
			writer.WriteLine ("\tswitch (asf_get_guid_type (&obj->id)) {");
			foreach (string str in objects) {
				if (str == "asf_object")
					continue;
				if (File.ReadAllText ("asf-structures.h").Contains ("void " + str + "_dump ("))
					continue;
				writer.WriteLine ("\tcase {0}:", str.ToUpper ());
				writer.WriteLine ("\t\t{0}_dump (({0}*) obj); break;", str);
			}
			writer.WriteLine ("\tdefault:");
			writer.WriteLine ("\t\tasf_object_dump (obj); break;");
			writer.WriteLine ("\t}");
			writer.WriteLine ("}");
			writer.WriteLine ("");

			writer.WriteLine ("/* ");
			writer.WriteLine ("  Some almost read to use copy-and-paste code.");
			writer.WriteLine ("");
			writer.WriteLine ("");
			foreach (string str in objects) {
				int size = 16 + 8;
				try {
					foreach (Field field in ReadFields (str)) {
							size += field.get_size ();
					}
				} catch {
					continue;
				}
				writer.WriteLine ("bool " + str + "_validate (const " + str + "* obj, ASFParser* parser)");
				writer.WriteLine ("{");
				writer.WriteLine ("\tif (!(asf_guid_validate (&obj->id, &" + str.Replace ("asf_", "asf_guids_") + ", parser))) {");
				writer.WriteLine ("\t\treturn false;");
				writer.WriteLine ("\t}");
				writer.WriteLine ("\t// FIXME: Verify that this size is correct.");
				writer.WriteLine ("\tif (obj->size < " + size.ToString () + ") {");
				writer.WriteLine ("\t\tparser->AddError (g_strdup_printf (\"Invalid size (expected >= " + size.ToString () + ", got %\" G_GUINT64_FORMAT \").\", obj->size));");
				writer.WriteLine ("\t\treturn false;");
				writer.WriteLine ("\t}");
				writer.WriteLine ("\t// TODO: More verifications?");
				writer.WriteLine ("\treturn true;");
				writer.WriteLine ("}");
				writer.WriteLine ("");
			}
			writer.WriteLine ("");
			writer.WriteLine ("*/");
			
			writer.WriteLine ("");
		}
		return 0;
	}
	
	static List<string> ReadAllLines (string pattern)
	{
		List<string> result = new List<string> ();
		foreach (string file in Directory.GetFiles (pattern)) {
			if (!file.Contains ("asf-generated."))
				result.AddRange (File.ReadAllLines (file));
		}
		return result;
	}
	
	static List<Field> ReadFields (string structure)
	{
		List<Field> result = new List<Field> ();
		bool in_struct = false;
		int line_number = 0;
		//bool is_obj = false;
		foreach (string l in ReadAllLines ("asf-structures.h")) {
			string line = l;
			line = line.Trim ();
			line_number++;
			
			if (line.Contains ("//"))
				line = line.Substring (0, line.IndexOf ("//"));

			line = line.Trim ();

			if (!in_struct) {
				if (line.EndsWith (";"))
					continue;
				
				if (!line.StartsWith ("struct " + structure + " "))
					continue;
				
				in_struct = true;
				
				if (line.Contains (" : public asf_object")) {
					Field field;
					field = new Field ();
					field.type = "asf_guid"; 
					field.name = "id";
					result.Add (field);
					field = new Field ();
					field.type = "asf_qword";
					field.name = "size";
					result.Add (field);
				}
			} else {
				if (line == string.Empty || line == "}" || line == "};")
					break;

				line = line.TrimEnd (';');
				string [] vars = line.Split (' ');
				if (vars.Length == 2) {
					Field field = new Field ();
					field.type = vars [0];
					field.name = vars [1];
					result.Add (field);
				} else {
					Console.WriteLine ("Weird line in asf-structures.h: {0} '{1}', '{2}'", line_number, l, line);
				}
			}
		}
		return result;
	}
	
	static List<string> ReadStructs (bool all)
	{
		List<string> result = new List<string> ();
		
		foreach (string l in ReadAllLines ("asf-structures.h")) {
			string line = l;
			line = line.Trim ();

			if (line.EndsWith (";"))
				continue;
			
			if (!line.StartsWith ("struct "))
				continue;
			
			if (!(all || line.EndsWith (" : public asf_object {")))
				continue;
			
			result.Add (line.Substring (7, line.IndexOf (' ', 8) - 7));
		}
		return result;
	}
	
	static List<string> ReadClasses ()
	{
		List<string> result = new List<string> ();
		foreach (string l in ReadAllLines ("asf.h")) {
			string line = l;
			line = line.Trim ();
			if (!(line.StartsWith ("class ") && !line.EndsWith (";")))
				continue;
			
			result.Add (line.Substring (6, line.IndexOf (' ', 8) - 6));
		}
		return result;
	}
}
