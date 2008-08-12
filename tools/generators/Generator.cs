/*
 * Generator.cs
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


class Generator {
	public void Generate ()
	{
		GlobalInfo info = GetTypes2 ();
		//info.Dump (0);
		
		GenerateValueH (info);	
		GenerateTypeH (info);
		GenerateKindCs ();
		
		GenerateTypeStaticCpp (info);
		GenerateCBindings (info);
		GeneratePInvokes (info);
		GenerateTypes_G ();
		
		GenerateDPs (info);
		GenerateManagedDPs (info);
	}
	
	static void GenerateManagedDPs (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		string class_dir = Path.Combine (base_dir, "class");
		string sys_win_dir = Path.Combine (Path.Combine (class_dir, "System.Windows"), "System.Windows");
		string filename = Path.Combine (sys_win_dir, "DependencyProperty.g.cs");
		string previous_namespace = "";
		List<TypeInfo> sorted_types = new List<TypeInfo>  ();
		StringBuilder text = new StringBuilder ();
		Dictionary <TypeInfo, List<FieldInfo>> types = new Dictionary<TypeInfo,List<FieldInfo>> ();
		
		foreach (FieldInfo field in all.DependencyProperties) {
			TypeInfo parent = field.Parent as TypeInfo;
			List <FieldInfo> fields;
			string managed_parent = field.Properties.GetValue ("ManagedDeclaringType");
			
			if (managed_parent != null) {
				parent = all.Children [managed_parent] as TypeInfo;
				
				if (parent == null)
					throw new Exception (string.Format ("Could not find the type '{0}' set as ManagedDeclaringType of '{1}'", managed_parent, field.FullName));
			}
			
			if (parent == null)
				throw new Exception (string.Format ("The field '{0}' does not have its parent set.", field.FullName));
			
			
			if (!types.TryGetValue (parent, out fields)) {
				fields = new List<FieldInfo> ();
				types.Add (parent, fields);
				sorted_types.Add (parent);
			}
			fields.Add (field);
		}
		
		Helper.WriteWarningGenerated (text);
		text.AppendLine ("using Mono;");
		text.AppendLine ("using System;");
		text.AppendLine ("using System.Collections.Generic;");
		text.AppendLine ("using System.Windows;");
		text.AppendLine ("using System.Windows.Controls;");
		text.AppendLine ("using System.Windows.Documents;");
		text.AppendLine ("using System.Windows.Ink;");
		text.AppendLine ("using System.Windows.Input;");
		text.AppendLine ("using System.Windows.Markup;");
		text.AppendLine ("using System.Windows.Media;");
		text.AppendLine ("using System.Windows.Media.Animation;");
		text.AppendLine ("using System.Windows.Shapes;");
		text.AppendLine ();
		
		sorted_types.Sort (new Members.MembersSortedByManagedFullName <TypeInfo> ());
		for (int i = 0; i < sorted_types.Count; i++) {
			TypeInfo type = sorted_types [i];
			List<FieldInfo> fields = types [type];
			TypeInfo parent = type;
			string ns;
			
			ns = parent.Namespace;
			
			if (string.IsNullOrEmpty (ns)) {
				Console.WriteLine ("The type '{0}' does not have a namespace annotation.", parent.FullName);
				continue;
			}
			
			if (ns == "None")
				continue;
			
			Console.WriteLine ("{0}.{1}", type.Namespace, type.FullName);
			
			string check_ns = Path.Combine (Path.Combine (Path.Combine (class_dir, "System.Windows"), ns), parent.Name + ".cs");
			if (!File.Exists (check_ns))
				Console.WriteLine ("The file {0} does not exist, did you annotate the class with the wrong namespace?", check_ns);
			
			if (previous_namespace != ns) {
				if (previous_namespace != string.Empty) {
					text.AppendLine ("}");
					text.AppendLine ();
				}
				text.Append ("namespace ");
				text.Append (ns);
				text.AppendLine (" {");
				previous_namespace = ns;
			} else {
				text.AppendLine ();
			}
			text.Append ("\tpartial class ");
			text.Append (parent.Name);
			text.AppendLine (" {");
			
			fields.Sort (new Members.MembersSortedByName <FieldInfo> ());
			foreach (FieldInfo field in fields) {
				text.Append ("\t\t");
				switch (field.Properties.GetValue ("Access")) {
				case "Internal":
					text.Append ("private ");
					break;
				default:
					text.Append ("public ");
					break;
				}
				text.Append ("static readonly DependencyProperty ");
				text.Append (field.Name);
				text.Append (" = DependencyProperty.Lookup (Kind.");
				text.Append (field.ParentType.KindName);
				text.Append (", \"");
				text.Append (field.Name.Substring (0, field.Name.LastIndexOf ("Property")));
				text.Append ("\", typeof (");
				text.Append (field.GetDPManagedPropertyType (all));
				text.AppendLine ("));");
			}			
			text.AppendLine ("\t}");
		}
		text.AppendLine ("}");		
		
		Helper.WriteAllText (filename, text.ToString ());
	}
	
	static void GenerateDPs (GlobalInfo all)
	{	
		string base_dir = Environment.CurrentDirectory;
		string moon_dir = Path.Combine (base_dir, "src");
		string last_type = string.Empty;
		int version_previous = 0;
		StringBuilder text = new StringBuilder ();
		List<FieldInfo> fields = all.DependencyProperties;
		HeaderCollection headers = new HeaderCollection ();
		
		headers.Add ("dependencyproperty.h", 1);
		headers.Add ("color.h", 1);
		foreach (FieldInfo field in fields)
			headers.Add (field.Header, field.SilverlightVersion);
				
		Helper.WriteWarningGenerated (text);
		text.AppendLine ();
		text.AppendLine ("#ifdef HAVE_CONFIG_H");
		text.AppendLine ("#include <config.h>");
		text.AppendLine ("#endif");
		text.AppendLine ();
		headers.Write (text);
		text.AppendLine ();
		text.AppendLine ("bool dependency_properties_initialized = false;");
		text.AppendLine ("void");
		text.AppendLine ("dependency_property_g_init (void)");
		text.AppendLine ("{");
		text.AppendLine ("\tif (dependency_properties_initialized)");
		text.AppendLine ("\t\treturn;");
		text.AppendLine ("\tdependency_properties_initialized = true;");
		
		for (int i = 0; i < fields.Count; i++) {
			FieldInfo field = fields [i];
			TypeInfo type = field.ParentType;
			TypeInfo propertyType = null;
			string property_type = field.DPPropertyType;
			string default_value = field.DPDefaultValue;
			bool has_default_value = !string.IsNullOrEmpty (default_value);
			bool is_nullable = field.IsDPNullable;
			bool is_attached = field.IsDPAttached;
			bool is_readonly = field.IsDPReadOnly;
			bool is_always_change = field.IsDPAlwaysChange;
			bool is_full = is_attached || is_readonly || is_always_change;
			int version = field.SilverlightVersion;
			int version_next = (i + 1 < fields.Count) ? fields [i + 1].SilverlightVersion : -1;
			
			if (version > 1 && version_previous != version) {
				text.Append ("#if ");
				Helper.WriteVersion (text, version);
				text.AppendLine ();
			}
			
			propertyType = field.GetDPPropertyType (all);
			
			text.Append ("\t");
			
			if (propertyType == null) {
				text.Append ("// (no PropertyType was found for this DependencyProperty) ");
			} else {
				headers.Add (propertyType.Header, version);
			}
			
			text.Append (type.Name);
			text.Append ("::");
			text.Append (field.Name);
			text.Append (" = DependencyProperty::Register");
			if (is_nullable)
				text.Append ("Nullable");
			else if (is_full)
				text.Append ("Full");
			text.Append (" (Type::");
			text.Append (type.KindName);
			text.Append (", \"");
			
			text.Append (field.Name, 0, field.Name.LastIndexOf ("Property"));
			text.Append ("\"");
			text.Append (", ");
			
			if (is_full) {
				if (has_default_value) {
					text.Append ("new Value (");
					text.Append (default_value);
					text.Append (")");
				} else {
					text.Append ("NULL");
				}
			} else {
				if (has_default_value) {
					text.Append ("new Value (");
					text.Append (default_value);
					text.Append (")");
				}
			}
			
			if (is_full || !has_default_value) {
				if (has_default_value || (is_full && !has_default_value))
					text.Append (", ");
				if (propertyType != null) {
					text.Append ("Type::");
					text.Append (propertyType.KindName);
				} else {
					text.Append ("Type::INVALID");
					//Console.WriteLine ("{0} does not define its property type.", field.FullName);
				}
			}
			
			if (is_full) {
				text.Append (", ");
				text.Append (is_attached ? "true" : "false");
				text.Append (", ");
				text.Append (is_readonly ? "true" : "false");
				if (is_always_change) {
					text.Append (", ");
					text.Append (is_always_change ? "true" : "false");
					text.Append (", ");
					text.Append ("NULL");
				}
			}
			
			text.AppendLine (");");
			if (version > 1 && version_next != version)
				text.AppendLine ("#endif");
			version_previous = version;
		}
		text.AppendLine ("}");
		text.AppendLine ();
				
		for (int i = 0; i < fields.Count; i++) {
			FieldInfo field = fields [i];
			int version = field.SilverlightVersion;
			int version_next = i + 1 < fields.Count ? fields [i + 1].SilverlightVersion : -1;
			if (version > 1 && version_previous != version) {
				text.Append ("#if SL_");
				text.Append (version);
				text.AppendLine ("_0");
			}
			text.Append ("DependencyProperty *");
			text.Append (field.Parent.Name);
			text.Append ("::");
			text.Append (field.Name);
			text.AppendLine (" = NULL;");
			if (version > 1 && version_next != version)
				text.AppendLine ("#endif");
			version_previous = version;
		}
		text.AppendLine ();
		
		Helper.WriteAllText (Path.Combine (moon_dir, "dependencyproperty.g.cpp"), text.ToString ());
		
	}
	
	static GlobalInfo GetTypes2 ()
	{
		string srcdir = Path.Combine (Environment.CurrentDirectory, "src");
		string[] files = Directory.GetFiles (srcdir, "*.h");
		Tokenizer tokenizer = new Tokenizer (files);
		GlobalInfo all = new GlobalInfo ();
		
		tokenizer.Advance (false);
		
		try {
			while (ParseMembers (all, tokenizer)) {
			}
		} catch (Exception ex) {
			throw new Exception (string.Format ("{0}({1}): {2}", tokenizer.CurrentFile, tokenizer.CurrentLine, ex.Message), ex);
		}
		
		// Add all the manual types
		all.Children.Add (new TypeInfo ("bool", "BOOL", null, true));
		all.Children.Add (new TypeInfo ("double", "DOUBLE", null, true));
		all.Children.Add (new TypeInfo ("guint64", "UINT64", null, true));
		all.Children.Add (new TypeInfo ("gint64", "INT64", null, true));
		all.Children.Add (new TypeInfo ("guint32", "UINT32", null, true));
		all.Children.Add (new TypeInfo ("gint32", "INT32", null, true));
		all.Children.Add (new TypeInfo ("char*", "STRING", null, true));
		all.Children.Add (new TypeInfo ("NPObj", "NPOBJ", null, true));
		all.Children.Add (new TypeInfo ("Managed", "MANAGED", null, true, 2));
		all.Children.Add (new TypeInfo ("TimeSpan", "TIMESPAN", null, true));
		
		return all;
	}
	
	// Returns false if there are no more tokens (reached end of code)
	static bool ParseClassOrStruct (Properties properties, MemberInfo parent, Tokenizer tokenizer)
	{
		TypeInfo type = new TypeInfo ();
		
		type.Properties = properties;
		type.Header = tokenizer.CurrentFile;
		type.Parent = parent;
		
		type.IsPublic = tokenizer.Accept (Token2Type.Identifier, "public");

		if (tokenizer.Accept (Token2Type.Identifier, "class")) {
			type.IsStruct = false;
		} else if (tokenizer.Accept (Token2Type.Identifier, "struct")) {
			type.IsStruct = true;
		} else if (tokenizer.Accept (Token2Type.Identifier, "union")) {
			type.IsStruct = true; // Not entirely correct, but a union can be parsed as a struct
		} else {
			throw new Exception (string.Format ("Expected 'class' or 'struct', not '{0}'", tokenizer.CurrentToken.value));
		}
		
		if (tokenizer.CurrentToken.type == Token2Type.Identifier) {
			type.Name = tokenizer.GetIdentifier ();
		} else {
			type.Name = "<anonymous>";
		}
		
		if (tokenizer.Accept (Token2Type.Punctuation, ";")) {
			// A forward declaration.
			//Console.WriteLine ("ParseType: Found a forward declaration to {0}", type.Name);
			return true;
		}
		
		if (tokenizer.Accept (Token2Type.Punctuation, ":")) {
			if (!tokenizer.Accept (Token2Type.Identifier, "public"))
				throw new Exception (string.Format ("The base class of {0} is not public.", type.Name));
			
			type.Base = ParseTypeReference (tokenizer);
			
			if (tokenizer.CurrentToken.value == ",")
				throw new Exception (string.Format ("Multiple inheritance is not supported"));
			
			//Console.WriteLine ("ParseType: Found {0}'s base class: {1}", type.Name, type.Base);
		}
		
		tokenizer.AcceptOrThrow (Token2Type.Punctuation, "{");
		
		//Console.WriteLine ("ParseType: Found a type: {0} in {1}", type.Name, type.Header);
		parent.Children.Add (type);
		ParseMembers (type, tokenizer);
		
		tokenizer.AcceptOrThrow (Token2Type.Punctuation, "}");
		
		if (tokenizer.CurrentToken.type == Token2Type.Identifier)
			tokenizer.Advance (true);
		
		if (tokenizer.CurrentToken.value != ";")
			throw new Exception (string.Format ("Expected ';', not '{0}'", tokenizer.CurrentToken.value));
		
		return tokenizer.Advance (false);
	}
	
	static bool ParseMembers (MemberInfo parent, Tokenizer tokenizer)
	{
		Properties properties = new Properties ();
		TypeInfo parent_type = parent as TypeInfo;
		string accessibility;
		TypeReference returntype;
		bool is_dtor;
		bool is_ctor;
		bool is_virtual;
		bool is_static;
		bool is_const;
		bool is_abstract;
		bool is_extern;
		string name;
		
		//Console.WriteLine ("ParseMembers ({0})", type.Name);
		
	 	do {
			returntype = null;
			is_dtor = is_ctor = is_virtual = is_static = false;
			is_extern = is_abstract = is_const = false;
			name = null;
			properties = new Properties ();
			
			if (parent_type != null)
				accessibility = parent_type.IsStruct ? "public" : "private";
			else
				accessibility = "public";
			
			if (tokenizer.Accept (Token2Type.Punctuation, ";"))
				continue;
			
			if (tokenizer.CurrentToken.value == "}")
				return true;
			
			while (tokenizer.CurrentToken.type == Token2Type.CommentProperty) {
				properties.Add (tokenizer.CurrentToken.value);
				tokenizer.Advance (true);
			}
			
			//Console.WriteLine ("ParseMembers: Current token: {0}", tokenizer.CurrentToken);
			
			if (tokenizer.CurrentToken.type == Token2Type.Identifier) {
				string v = tokenizer.CurrentToken.value;
				switch (v) {
				case "public":
				case "protected":
				case "private":
					accessibility = v;
					tokenizer.Advance (true);
					tokenizer.Accept (Token2Type.Punctuation, ":");
					continue;
				case "enum":
					ParseEnum (parent, tokenizer);
					continue;
				case "struct":
				case "class":
				case "union":
					if (!ParseClassOrStruct (properties, parent, tokenizer))
						return false;
					continue;
				case "typedef":
					tokenizer.Advance (true);
					while (!tokenizer.Accept (Token2Type.Punctuation, ";")) {
						if (tokenizer.CurrentToken.value == "{")
							tokenizer.SyncWithEndBrace ();
						tokenizer.Advance (true);
					}
					continue;
				}
			}
			
			do {
				if (tokenizer.Accept (Token2Type.Identifier, "virtual")) {
					is_virtual = true;
					continue;
				}
				
				if (tokenizer.Accept (Token2Type.Identifier, "static")) {
					is_static = true;
					continue;
				}
			   
				if (tokenizer.Accept (Token2Type.Identifier, "const")) {
					is_const = true;
					continue;
				}
				
				if (tokenizer.Accept (Token2Type.Identifier, "extern")) {
					is_extern = true;
					continue;
				}
				
			    break;
			} while (true);
			
			if (tokenizer.Accept (Token2Type.Punctuation, "~"))
				is_dtor = true;
					
			if (is_dtor) {
				name = "~" + tokenizer.GetIdentifier ();
				returntype = new TypeReference ("void");
			} else {
				returntype = ParseTypeReference (tokenizer);
				
				if (returntype.Value == parent.Name && tokenizer.CurrentToken.value == "(") {
					is_ctor = true;
					name = returntype.Value;
					returntype.Value += "*";
				} else {
					name = tokenizer.GetIdentifier ();
				}
			}
			returntype.IsConst = is_const;
			returntype.IsReturnType = true;

			//Console.WriteLine ("ParseMembers: found member '{0}' is_ctor: {1}", name, is_ctor);
			
			if (tokenizer.Accept (Token2Type.Punctuation, "(")) {
				// Method
				MethodInfo method = new MethodInfo ();
				method.Parent = parent;
				method.Properties = properties;
				method.Name = name;
				method.IsConstructor = is_ctor;
				method.IsDestructor = is_dtor;
				method.IsVirtual = is_virtual;
				method.IsStatic = is_static;
				method.IsAbstract = is_abstract;
				method.IsPublic = accessibility == "public";
				method.IsPrivate = accessibility == "private";
				method.IsProtected = accessibility == "protected";
				method.ReturnType = returntype;
				
				//Console.WriteLine ("ParseMembers: found method '{0}' is_ctor: {1}", name, is_ctor);
				
				if (!tokenizer.Accept (Token2Type.Punctuation, ")")) {
					string param_value = null;
					do {
						ParameterInfo parameter = new ParameterInfo ();
						parameter.ParameterType = ParseTypeReference (tokenizer);
						if (tokenizer.CurrentToken.value != "," && tokenizer.CurrentToken.value != ")") {
							parameter.Name = tokenizer.GetIdentifier ();
							if (tokenizer.Accept (Token2Type.Punctuation, "["))
								tokenizer.AcceptOrThrow (Token2Type.Punctuation, "]");
							if (tokenizer.Accept (Token2Type.Punctuation, "=")) {
								param_value = string.Empty;
								if (tokenizer.Accept (Token2Type.Punctuation, "-"))
									param_value = "-";
								param_value += tokenizer.GetIdentifier ();
							}
						}
						method.Parameters.Add (parameter);
						//Console.WriteLine ("ParseMember: got parameter, type: '{0}' name: '{1}' value: '{2}'", parameter.ParameterType.Value, parameter.Name, param_value);
					} while (tokenizer.Accept (Token2Type.Punctuation, ","));
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ")");
				}
				
				parent.Children.Add (method);
				
				if (tokenizer.CurrentToken.value == "{") {
					//Console.WriteLine ("ParseMember: member has body, skipping it");
					tokenizer.SyncWithEndBrace ();
				} else if (is_ctor && tokenizer.Accept (Token2Type.Punctuation, ":")) {
					// ctor method implemented in header with field initializers and/or base class ctor call
					tokenizer.FindStartBrace ();
					tokenizer.SyncWithEndBrace ();
					//Console.WriteLine ("ParseMember: skipped ctor method implementation");				
				} else if (tokenizer.Accept (Token2Type.Punctuation, "=")) {
					// pure virtual method
					tokenizer.AcceptOrThrow (Token2Type.Identifier, "0");
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ";");
					is_abstract = true;
				} else {
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ";");
				}
			} else {
				if (is_ctor || is_dtor)
					throw new Exception (string.Format ("Expected '(', not '{0}'", tokenizer.CurrentToken.value));
				
				if (name == "operator") {
					while (true) {
						if (tokenizer.CurrentToken.value == ";") {
							// End of operator
							break;
						} else if (tokenizer.CurrentToken.value == "{") {
							// In-line implementation
							tokenizer.SyncWithEndBrace ();
							break;
						}
						tokenizer.Advance (true);
					}
					//Console.WriteLine ("ParseMembers: skipped operator");
				} else {
					FieldInfo field = new FieldInfo ();
					field.IsConst = is_const;
					field.IsStatic = is_static;
					field.IsExtern = is_extern;
					field.Name = name;
					field.FieldType = returntype;
					field.IsPublic = accessibility == "public";
					field.IsPrivate = accessibility == "private";
					field.IsProtected = accessibility == "protected";
					field.Properties = properties;
					
					// Field
					do {
						//Console.WriteLine ("ParseMembers: found field '{0}'", name);
						field.Parent = parent;
						parent.Children.Add (field);
						
						if (tokenizer.Accept (Token2Type.Punctuation, "[")) {
							while (!tokenizer.Accept (Token2Type.Punctuation, "]")) {
								tokenizer.Advance (true);
							}
						}
						if (tokenizer.Accept (Token2Type.Punctuation, ":")) {
							field.BitField = tokenizer.GetIdentifier ();
						}
						if (tokenizer.Accept (Token2Type.Punctuation, ",")) {
							field = new FieldInfo ();
							if (tokenizer.Accept (Token2Type.Punctuation, "*")) {
								// ok
							}
							field.Name = tokenizer.GetIdentifier ();
							field.FieldType = returntype;
							continue;
						}
						break;
					} while (true);
					
					tokenizer.Accept (Token2Type.Punctuation, ";");
				}
			}
		} while (true);
	}
	
	static void ParseEnum (MemberInfo parent, Tokenizer tokenizer)
	{
		FieldInfo field;
		StringBuilder value = new StringBuilder ();
		TypeInfo type = new TypeInfo ();
		
		type.IsEnum = true;
		
		tokenizer.AcceptOrThrow (Token2Type.Identifier, "enum");
		if (tokenizer.CurrentToken.type == Token2Type.Identifier) {
			type.Name = tokenizer.GetIdentifier ();
		} else {
			type.Name = "<anonymous>";
		}
		parent.Children.Add (type);
		
		tokenizer.AcceptOrThrow (Token2Type.Punctuation, "{");
		
		//Console.WriteLine ("ParseEnum: {0}", name);
		
		while (tokenizer.CurrentToken.type == Token2Type.Identifier) {
			field = new FieldInfo ();
			field.Name = tokenizer.GetIdentifier ();
			value.Length = 0;
			if (tokenizer.Accept (Token2Type.Punctuation, "=")) {
				while (tokenizer.CurrentToken.value != "," && tokenizer.CurrentToken.value != "}") {
					value.Append (" ");
					value.Append (tokenizer.CurrentToken.value);
					tokenizer.Advance (true);
				}
			}
			type.Children.Add (field);
			//Console.WriteLine ("ParseEnum: {0}: {1} {2} {3}", name, field, value.Length != 0 != null ? "=" : "", value);
						
			if (!tokenizer.Accept (Token2Type.Punctuation, ","))
				break;
		}
		
		tokenizer.AcceptOrThrow (Token2Type.Punctuation, "}");
		tokenizer.AcceptOrThrow (Token2Type.Punctuation, ";");
	}
	
	public static TypeReference ParseTypeReference (Tokenizer tokenizer)
	{
		TypeReference tr = new TypeReference ();
		StringBuilder result = new StringBuilder ();
		
		if (tokenizer.Accept (Token2Type.Identifier, "const"))
			tr.IsConst = true;
		
		result.Append (tokenizer.GetIdentifier ());
		
		if (tokenizer.Accept (Token2Type.Punctuation, ":")) {
			tokenizer.AcceptOrThrow (Token2Type.Punctuation, ":");
			result.Append ("::");
			result.Append (tokenizer.GetIdentifier ());
		}
		
		while (tokenizer.Accept (Token2Type.Punctuation, "*"))
			result.Append ("*");
		
		if (tokenizer.Accept (Token2Type.Punctuation, "&"))
			result.Append ("&");
		
		//Console.WriteLine ("ParseTypeReference: parsed '{0}'", result.ToString ());
		
		tr.Value = result.ToString ();

		if (tr.Value == "Value*")
			tr.IsRef = true;
		return tr;
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
	
	public void GenerateTypes_G ()
	{
		string base_dir = Environment.CurrentDirectory;
		string class_dir = Path.Combine (base_dir, "class");
		string sys_win_dir = Path.Combine (class_dir, "System.Windows");
		string moon_moonlight_dir = Path.Combine (class_dir, "Mono.Moonlight");
		
		string magic = "return Kind.";
		StringBuilder text = new StringBuilder ();
		
		Helper.WriteWarningGenerated (text);
					
		text.AppendLine ("using Mono;");
		text.AppendLine ("using System;");
		text.AppendLine ("using System.Reflection;");
		text.AppendLine ("using System.Collections.Generic;");
		text.AppendLine ("");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tstatic partial class Types {");
		text.AppendLine ("\t\tprivate static void CreateNativeTypes ()");
		text.AppendLine ("\t\t{");
		text.AppendLine ("\t\t\tAssembly agclr = Helper.GetAgclr ();");
		text.AppendLine ("\t\t\tType t;");
		text.AppendLine ("\t\t\ttry {");
				
		foreach (string dir in Directory.GetDirectories (sys_win_dir)) {
			Log.WriteLine ("Checking: {0}", dir);
			string ns = Path.GetFileName (dir);
			foreach (string file in Directory.GetFiles (dir, "*.cs")) {
				string contents = File.ReadAllText (file);
				string type;
				string kind;
				int get_kind_pos = contents.IndexOf (magic);
				int end_kind_pos;
				
				if (get_kind_pos < 0)
					continue;
				
			 	end_kind_pos = contents.IndexOf (";", get_kind_pos + magic.Length);
				
				if (end_kind_pos < 0)
					throw new Exception (string.Format ("Found 'return Kind.' in {0} at pos {1}, but not any ';' after that", file, get_kind_pos));
				
				kind = contents.Substring (get_kind_pos + magic.Length, end_kind_pos - (get_kind_pos + magic.Length));
				type = Path.GetFileNameWithoutExtension (file);
				
				if (type == "PresentationFrameworkCollection")
					type = "PresentationFrameworkCollection`1";
				
				Log.WriteLine ("Found Kind.{0} in {1} which result in type: {2}.{3}", kind, file, ns, type);
				
				text.Append ("\t\t\t\tt = agclr.GetType (\"");
				text.Append (ns);
				text.Append (".");
				text.Append (type);
				text.AppendLine ("\", true); ");
				
				
				text.Append ("\t\t\t\ttypes.Add (t, new ManagedType (t, Kind.");
				text.Append (kind);
				text.AppendLine ("));");
			}
		}
		
		text.AppendLine ("\t\t\t} catch (Exception ex) {");
		text.AppendLine ("\t\t\t\tConsole.WriteLine (\"There was an error while loading native types: \" + ex.Message);");
		text.AppendLine ("\t\t\t}");
		text.AppendLine ("\t\t}");
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
	 	Log.WriteLine ("typeandkidngen done");
		
		Helper.WriteAllText (Path.Combine (Path.Combine (moon_moonlight_dir, "Mono"), "Types.g.cs"), text.ToString ());
	}
	
	public static void GenerateCBindings (GlobalInfo info)
	{
		string base_dir = Environment.CurrentDirectory;
		string plugin_dir = Path.Combine (base_dir, "plugin");
		string moon_dir = Path.Combine (base_dir, "src");
		List<MethodInfo> methods;
		StringBuilder header = new StringBuilder ();
		StringBuilder impl = new StringBuilder ();
		List <string> headers = new List<string> ();
		string last_type = string.Empty;
		
		if (!Directory.Exists (plugin_dir))
			throw new ArgumentException (string.Format ("cgen must be executed from the base directory of the moon module ({0} does not exist).", plugin_dir));
		
		if (!Directory.Exists (moon_dir))
			throw new ArgumentException (string.Format ("methodgen must be executed from the base directory of the moon module ({0} does not exist).", moon_dir));
		
		methods = info.CPPMethodsToBind;
		
		Helper.WriteWarningGenerated (header);;
		Helper.WriteWarningGenerated (impl);

		header.AppendLine ("#ifndef __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ("#define __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ();
		header.AppendLine ("#include <glib.h>");
		header.AppendLine ("// This should probably be changed to somehow not include c++ headers.");
		foreach (MemberInfo method in methods) {
			string h;
			if (string.IsNullOrEmpty (method.Header))
				continue;
			h = Path.GetFileName (method.Header);
			
			if (!headers.Contains (h))
				headers.Add (h);
		}
		headers.Sort ();
		foreach (string h in headers) {
			header.Append ("#include \"");
			header.Append (h);
			header.AppendLine ("\"");
		}
		
		header.AppendLine ();
		header.AppendLine ("G_BEGIN_DECLS");
		header.AppendLine ();
		
		impl.AppendLine ("#ifdef HAVE_CONFIG_H");
		impl.AppendLine ("#include <config.h>");
		impl.AppendLine ("#endif");
		impl.AppendLine ();
		impl.AppendLine ("#include <stdio.h>");
		impl.AppendLine ("#include <stdlib.h>");
		impl.AppendLine ();
		impl.AppendLine ("#include \"cbinding.h\"");
		impl.AppendLine ();
		
		foreach (MemberInfo member in methods) {
			MethodInfo method = (MethodInfo) member;			
			
			WriteMethodIfVersion (method.CMethod, header, false);
			WriteMethodIfVersion (method.CMethod, impl, false);
			
			if (last_type != method.Parent.Name) {
				last_type = method.Parent.Name;
				foreach (StringBuilder text in new StringBuilder [] {header, impl}) {
					text.AppendLine ("/**");
					text.Append (" * ");
					text.AppendLine (last_type);
					text.AppendLine (" **/");
				}
			}
			
			WriteHeaderMethod (method.CMethod, method, header);
			WriteMethodIfVersion (method.CMethod, header, true);
			header.AppendLine ();
			
			WriteImplMethod (method.CMethod, method, impl);
			WriteMethodIfVersion (method.CMethod, impl, true);
			impl.AppendLine ();
			impl.AppendLine ();
		}
		
		header.AppendLine ();
		header.AppendLine ("G_END_DECLS");
		header.AppendLine ();
		header.AppendLine ("#endif");
		
		Helper.WriteAllText (Path.Combine (moon_dir, "cbinding.h"), header.ToString ());
		Helper.WriteAllText (Path.Combine (moon_dir, "cbinding.cpp"), impl.ToString ());
	}
	
	static void WriteMethodIfVersion (MethodInfo method, StringBuilder text, bool end)
	{
		if (method.SilverlightVersion > 1) {
			if (!end) {
				switch (method.SilverlightVersion) {
				case 2:
					text.AppendLine ("#if SL_2_0");
					break;
				default:
					throw new Exception (string.Format ("Unknown SilverlightVersion: {0}", method.SilverlightVersion));
				}
			} else {
				text.AppendLine ("#endif");
			}
		}
	}
	
	static void WriteHeaderMethod (MethodInfo cmethod, MethodInfo cppmethod, StringBuilder text)	
	{
		Log.WriteLine ("Writing header: {0}::{1} (Version: '{2}', GenerateManaged: {3})", 
		               cmethod.Parent.Name, cmethod.Name, 
		               cmethod.Properties.GetValue ("Version"),
		               cmethod.Properties.ContainsKey ("GenerateManaged"));
		
		if (cmethod.Properties.ContainsKey ("GeneratePInvoke"))
			text.AppendLine ("/* @GeneratePInvoke */");
		cmethod.ReturnType.Write (text, SignatureType.Native);
		if (!cmethod.ReturnType.IsPointer)
			text.Append (" ");
		text.Append (cmethod.Name);
		cmethod.Parameters.Write (text, SignatureType.Native, false);
		text.AppendLine (";");
	}
	
	static void WriteImplMethod (MethodInfo cmethod, MethodInfo cppmethod, StringBuilder text)
	{
		bool is_void = cmethod.ReturnType.Value == "void";
		bool is_ctor = cmethod.IsConstructor;
		bool is_static = cmethod.IsStatic;
		bool is_dtor = cmethod.IsDestructor;
		bool check_instance = !is_static && !is_ctor;
		bool check_error = false;
		
		foreach (ParameterInfo parameter in cmethod.Parameters) {
			if (parameter.ParameterType.Value == "MoonError*") {
				check_error = true;
				break;
			}
		}
		
		cmethod.ReturnType.Write (text, SignatureType.Native);
		text.AppendLine ();
		text.Append (cmethod.Name);
		cmethod.Parameters.Write (text, SignatureType.Native, false);
		text.AppendLine ("");
		text.AppendLine ("{");
		
		if (is_ctor) {
			text.Append ("\treturn new ");
			text.Append (cmethod.Parent.Name);
			cmethod.Parameters.Write (text, SignatureType.Native, true);
			text.AppendLine (";");
		} else if (is_dtor) {
			text.AppendLine ("\tdelete instance;");
		} else {
			if (check_instance) {
				text.AppendLine ("\tif (instance == NULL)");
				
				if (cmethod.ReturnType.Value == "void") {
					text.Append ("\t\treturn");
				} else if (cmethod.ReturnType.Value.Contains ("*")) {	
					text.Append ("\t\treturn NULL");
				} else if (cmethod.ReturnType.Value == "Type::Kind") {
					text.Append ("\t\treturn Type::INVALID");
				} else if (cmethod.ReturnType.Value == "bool") {
					text.Append ("\t\treturn false;");
				} else {
					text.AppendLine ("\t\t// Need to find a property way to get the default value for the specified type and return that if instance is NULL.");
					text.Append ("\t\treturn");
					text.Append (" (");
					text.Append (cmethod.ReturnType.Value);
					text.Append (") 0");
				}
				text.AppendLine (";");
				
				text.AppendLine ("\t");
			}
			
			if (check_error) {
				text.AppendLine ("\tif (error == NULL)");
				text.Append ("\t\tg_warning (\"Moonlight: Called ");
				text.Append (cmethod.Name);
				text.AppendLine (" () with error == NULL.\");");
			}
			
			text.Append ("\t");
			if (!is_void)
				text.Append ("return ");
			
			if (is_static) {
				text.Append (cmethod.Parent.Name);
				text.Append ("::");
			} else {
				text.Append ("instance->");
				cmethod.Parameters [0].DisableWriteOnce = true;
			}
			text.Append (cppmethod.Name);
			cmethod.Parameters.Write (text, SignatureType.Native, true);
			text.AppendLine (";");
		}
		
		text.AppendLine ("}");
	}
	
	static void GenerateTypeStaticCpp (GlobalInfo all)
	{
		string header;
		List<string> headers = new List<string> ();
		List<string> headers2 = new List<string> ();
		
		StringBuilder text = new StringBuilder ();
		
		Helper.WriteWarningGenerated (text);
					
		text.AppendLine ("#ifdef HAVE_CONFIG_H");
		text.AppendLine ("#include <config.h>");
		text.AppendLine ("#endif");
		text.AppendLine ();
		text.AppendLine ("#include <stdlib.h>");

		headers.Add ("cbinding.h");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.C_Constructor == string.Empty || t.C_Constructor == null || !t.GenerateCBindingCtor) {
				//Console.WriteLine ("{0} does not have a C ctor", t.FullName);
				if (t.GetTotalEventCount (int.MaxValue) == 0)
					continue;
			}
	
			if (string.IsNullOrEmpty (t.Header)) {
			//	Console.WriteLine ("{0} does not have a header", t.FullName);
				continue;
			}
			
			//Console.WriteLine ("{0}'s header is {1}", t.FullName, t.Header);
			
			header = Path.GetFileName (t.Header);
			if (t.SilverlightVersion == 1) {
				if (headers2.Contains (header))
					throw new Exception (string.Format ("header {0} contains both a 1.0 and 2.0 class", header));
				
				if (!headers.Contains (header))
					headers.Add (header);
			}
			else if (t.SilverlightVersion == 2) {
				if (headers.Contains (header))
					throw new Exception (string.Format ("header {0} contains both a 1.0 and 2.0 class", header));
				
				if (!headers2.Contains (header))
					headers2.Add (header);
			}
		}
		
		// Loop through all the classes and check which headers
		// are needed for the c constructors
		text.AppendLine ("");
		headers.Sort ();
		foreach (string h in headers) {
			text.Append ("#include \"");
			text.Append (h);
			text.AppendLine ("\"");
		}
		
		text.AppendLine ("#if SL_2_0");
		headers2.Sort ();
		foreach (string h in headers2) {
			text.Append ("#include \"");
			text.Append (h);
			text.AppendLine ("\"");
		}
		text.AppendLine ("#endif");
		
		int [] versions = new int [] {2, 1};
		
		for (int i = 0; i < versions.Length; i++) {
			int version = versions [i];
			// Set the event ids for all the events
			text.AppendLine ();
			
			text.Append (i == 0 ? "#if " : "#else ");
			if (i == 0)
				Helper.WriteVersion (text, version);
						
			text.AppendLine ();
			
			foreach (TypeInfo t in all.Children.SortedTypesByKind) {
				if (version < t.SilverlightVersion)
					continue;
				
				if (t.GetEventCount (version) == 0)
					continue;
				
				
				foreach (FieldInfo field in t.Events) {
					if (version < field.SilverlightVersion)
						continue;
					text.Append ("const int ");
					text.Append (t.Name);
					text.Append ("::");
					text.Append (field.EventName);
					text.Append ("Event = ");
					text.Append (t.GetEventId (field, version));
					text.AppendLine (";");
				}
			}
	
			// Create the arrays of event names for the classes which have events
			text.AppendLine ("");
			foreach (TypeInfo t in all.Children.SortedTypesByKind) {
				if (t.Events == null || t.Events.Count == 0)
					continue;
				
				if (version < t.SilverlightVersion)
					continue;
				
				text.Append ("const char *");
				text.Append (t.Name);
				text.Append ("_Events [] = { ");
				
				foreach (FieldInfo field in t.Events) {
					if (version < field.SilverlightVersion)
						continue;
					
					text.Append ("\"");
					text.Append (field.EventName);
					text.Append ("\", ");
				}
				
				text.AppendLine ("NULL };");
			}
	
			// Create the array of type data
			text.AppendLine ("");
			text.AppendLine ("Type type_infos [] = {");
			text.AppendLine ("\t{ Type::INVALID, Type::INVALID, false, \"INVALID\", NULL, 0, 0, NULL, NULL, NULL, NULL, NULL },");
			foreach (TypeInfo type in all.Children.SortedTypesByKind) {
				MemberInfo member;
				TypeInfo parent = null;
				string events = "NULL";
				
				if (!type.ImplementsGetObjectType && !type.Properties.ContainsKey ("IncludeInKinds"))
					continue;
				
				if (type.Base != null && type.Base.Value != null && all.Children.TryGetValue (type.Base.Value, out member))
					parent = (TypeInfo) member;
				
				if (type.Events != null && type.Events.Count != 0)
					events = type.Name + "_Events";
	
				if (version >= type.SilverlightVersion) {
					text.AppendLine (string.Format (@"	{{ {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, NULL, NULL }}, ",
					                                "Type::" + type.KindName, 
					                                "Type::" + (parent != null ? parent.KindName : "INVALID"),
					                                type.IsValueType ? "true" : "false",
					                                "\"" + type.Name + "\"", 
					                                "\"" + type.KindName + "\"", 
					                                type.GetEventCount (version),
					                                type.GetTotalEventCount (version),
					                                events,
					                                (type.C_Constructor != null && type.GenerateCBindingCtor) ? string.Concat ("(create_inst_func *) ", type.C_Constructor) : "NULL", 
					                                type.ContentProperty != null ? string.Concat ("\"", type.ContentProperty, "\"") : "NULL"
					                                )
					                 );
				} else {
					text.AppendLine (string.Format ("	{{ Type::INVALID, Type::INVALID, false, \"2.0 specific type '{0}'\", {1}, 0, 0, NULL, NULL, NULL, NULL, NULL }}, ",
									type.KindName,
									"\"" + type.KindName + "\""));
				}
				
			}
			text.AppendLine ("\t{ Type::LASTTYPE, Type::INVALID, false, NULL, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL }");
			text.AppendLine ("};");
		}
		text.AppendLine ("#endif");
		text.AppendLine ();
				
		Helper.WriteAllText ("src/type-generated.cpp", text.ToString ());
	}
	
	static void GenerateTypeH (GlobalInfo all)
	{
		const string file = "src/type.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");
		
		contents = contents.Replace ("/*DO_KINDS*/", all.Children.GetKindsForEnum ().ToString ());

		text = new StringBuilder ();
					
		Helper.WriteWarningGenerated (text);
		
		contents = text.ToString () + contents;

		Helper.WriteAllText (file, contents);
	}

	static void GenerateKindCs ()
	{
		const string file = "src/type.h";
		StringBuilder text = new StringBuilder ();
		string contents = File.ReadAllText (file);
		int a = contents.IndexOf ("// START_MANAGED_MAPPING") + "// START_MANAGED_MAPPING".Length;
		int b = contents.IndexOf ("// END_MANAGED_MAPPING");
		string values = contents.Substring (a, b - a);		
		
		Helper.WriteWarningGenerated (text);
					
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tpublic enum Kind {");
		text.AppendLine (values);
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
		string realfile = "class/Mono.Moonlight/Mono/Kind.cs".Replace ('/', Path.DirectorySeparatorChar);
		Helper.WriteAllText (realfile, text.ToString ());
	}
	
	static void GenerateValueH (GlobalInfo all)
	{
		const string file = "src/value.h";
		StringBuilder result = new StringBuilder ();	

		Helper.WriteWarningGenerated (result);

		using (StreamReader reader = new StreamReader (file + ".in")) {
			string line;
			line = reader.ReadLine ();
			while (line != null) {
				if (line.Contains ("/*DO_FWD_DECLS*/")) {
					foreach (TypeInfo type in all.Children.SortedTypesByKind) {
						if (!type.ImplementsGetObjectType || type.IsNested)
							continue;
						
						if (type.IsStruct) {
							result.Append ("struct ");
						} else {
							result.Append ("class ");
						}
						result.Append (type.Name);
						result.AppendLine (";");
					}
					result.AppendLine ();
				} else if (line.Contains ("/*DO_AS*/")) {
					foreach (TypeInfo type in all.Children.SortedTypesByKind) {
						if (!type.ImplementsGetObjectType || type.IsNested)
							continue;
			
						//do_as.AppendLine (string.Format ("	{1,-30} As{0} () {{ checked_get_subclass (Type::{2}, {0}) }}", type.Name, type.Name + "*", type.KindName));
						
						result.Append ('\t');
						result.Append (type.Name);
						result.Append ("*");
						result.Append (' ', 30 - type.Name.Length);
						result.Append ("As");
						result.Append (type.Name);
						result.Append (" () { checked_get_subclass (Type::");
						result.Append (type.KindName);
						result.Append (", ");
						result.Append (type.Name);
						result.Append (") }");
						result.AppendLine ();
					}
					result.AppendLine ();
				} else {
					result.AppendLine (line);
				}
				line = reader.ReadLine ();
			}
		}
		
		Helper.WriteAllText (file, result.ToString ());
	}
	
	static bool IsManuallyDefined (string NativeMethods_cs, string method)
	{
		if (NativeMethods_cs.Contains (" " + method + " "))
			return true;
		else if (NativeMethods_cs.Contains (" " + method + "("))
			return true;
		else if (NativeMethods_cs.Contains ("\"" + method + "\""))
			return true;
		else
			return false;
	}
	
	static void GeneratePInvokes (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		List <MethodInfo> methods = new List<MethodInfo> ();
		StringBuilder text = new StringBuilder ();
		string NativeMethods_cs;
		
		NativeMethods_cs = File.ReadAllText (Path.Combine (base_dir, "class/Mono.Moonlight/Mono/NativeMethods.cs".Replace ('/', Path.DirectorySeparatorChar)));

		methods = all.CPPMethodsToBind;		
	
		Helper.WriteWarningGenerated (text);
		text.AppendLine ("using System;");
		text.AppendLine ("using System.Runtime.InteropServices;");
		text.AppendLine ("");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tpublic static partial class NativeMethods");
		text.AppendLine ("\t{");
		text.AppendLine ("\t\t/* moonplugin methods */");
		text.AppendLine ("\t");
		foreach (MethodInfo method in methods) {
			if (!method.IsPluginMember || !method.Properties.ContainsKey ("GeneratePInvoke"))
				continue;
			WritePInvokeMethod (NativeMethods_cs, method, text, "moonplugin");
			text.AppendLine ();
		}
		
		text.AppendLine ("\t");
		text.AppendLine ("\t\t/* libmoon methods */");
		text.AppendLine ("\t");
		foreach (MethodInfo method in methods) {
			if (!method.IsSrcMember || !method.Properties.ContainsKey ("GeneratePInvoke"))
				continue;
			WritePInvokeMethod (NativeMethods_cs, method, text, "moon");
			text.AppendLine ();
		}
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
		Helper.WriteAllText (Path.Combine (base_dir, "class/Mono.Moonlight/Mono/GeneratedPInvokes.cs".Replace ('/', Path.DirectorySeparatorChar)), text.ToString ());
	}
	
	static void WritePInvokeMethod (string NativeMethods_cs, MethodInfo method, StringBuilder text, string library)
	{
		bool marshal_string_returntype = false;
		bool marshal_moonerror = false;
		bool generate_wrapper = false;
		bool is_manually_defined;
		bool comment_out;
		bool is_static;
		bool is_void = false;
		bool contains_unknown_types;
		string name;
		string managed_name;
		string tabs;
		TypeReference returntype;
		MethodInfo cmethod = method.CMethod;
		ParameterInfo surface_parameter = null;
		ParameterInfo types_parameter = null;
		ParameterInfo error_parameter = null;
		
		if (method.ReturnType == null)
			throw new Exception (string.Format ("Method {0} in type {1} does not have a return type.", method.Name, method.Parent.Name));
		
		if (method.ReturnType.Value == "char*") {
			marshal_string_returntype = true;
			generate_wrapper = true;
		} else if (method.ReturnType.Value == "void") {
			is_void = true;
		}
		
		// Check for parameters we can automatically generate code for.
		foreach (ParameterInfo parameter in cmethod.Parameters) {
			if (parameter.Name == "surface" && parameter.ParameterType.Value == "Surface*") {
				parameter.ManagedWrapperCode = "Mono.Xaml.XamlLoader.SurfaceInDomain";
				generate_wrapper = true;
				surface_parameter = parameter;
			} else if (parameter.Name == "additional_types" && parameter.ParameterType.Value == "Types*") {
				parameter.ManagedWrapperCode = "Mono.Types.Native";
				generate_wrapper = true;
				types_parameter = parameter;
			} else if (parameter.Name == "error" && parameter.ParameterType.Value == "MoonError*") {
				marshal_moonerror = true;
				generate_wrapper = true;
				error_parameter = parameter;
			}
		}
		
		name = method.CMethod.Name;
		managed_name = name;
		if (marshal_moonerror)
			managed_name = managed_name.Replace ("_with_error", "");
		
		returntype = method.ReturnType;
		is_manually_defined = IsManuallyDefined (NativeMethods_cs, managed_name);
		contains_unknown_types = method.ContainsUnknownTypes;
		comment_out = is_manually_defined || contains_unknown_types;
		is_static = method.IsStatic;
		tabs = comment_out ? "\t\t// " : "\t\t";
				
		if (is_manually_defined)
			text.AppendLine ("\t\t// This method is already defined manually in NativeMethods.cs. Remove the import from there, and regenerate.");

		if (contains_unknown_types)
			text.AppendLine ("\t\t// This method contains types the generator didn't know about. Fix the generator (find the method 'GetManagedType' in typegen.cs and add the missing case) and try again.");
			
		text.Append (tabs);
		text.Append ("[DllImport (\"");
		text.Append (library);
		if (generate_wrapper) {
			text.Append ("\", EntryPoint=\"");
			text.Append (name);
		}
		text.AppendLine ("\")]");
		
		// Always output the native signature too, makes it easier to check if the generation is wrong.
		text.Append ("\t\t// ");
		cmethod.WriteFormatted (text);
		text.AppendLine ();
		
		text.Append (tabs);
		text.Append (generate_wrapper ? "private " : "public ");
		text.Append ("extern static ");
		if (marshal_string_returntype)
			text.Append ("IntPtr");
		else
			returntype.Write (text, SignatureType.PInvoke);
		text.Append (" ");
		text.Append (name);
		if (generate_wrapper)
			text.Append ("_");
		cmethod.Parameters.Write (text, SignatureType.PInvoke, false);
		text.AppendLine (";");
		
		if (generate_wrapper) {
			text.Append (tabs);
			text.Append ("public static ");
			returntype.Write (text, SignatureType.Managed);
			text.Append (" ");
			text.Append (managed_name);
			
			foreach (ParameterInfo parameter in cmethod.Parameters)
				parameter.DisableWriteOnce = parameter.ManagedWrapperCode != null;

			if (error_parameter != null)
				error_parameter.DisableWriteOnce = true;
			
			cmethod.Parameters.Write (text, SignatureType.Managed, false);
			text.AppendLine ();
			
			text.Append (tabs);
			text.Append ("{");
			text.AppendLine ();
			
			text.Append (tabs);
			
			if (marshal_string_returntype) {
				text.AppendLine ("\tIntPtr result;");
			} else if (!is_void) {
				text.Append ("\t");
				returntype.Write (text, SignatureType.Managed);
				text.AppendLine (" result;");
			}
			
			if (marshal_moonerror) {
				text.Append (tabs);
				text.AppendLine ("\tMoonError error;");
			}

			text.Append (tabs);
			text.Append ("\t");
			if (!is_void)
				text.Append ("result = ");
				
			text.Append (cmethod.Name);
			text.Append ("_");
			cmethod.Parameters.Write (text, SignatureType.Managed, true);
			
			text.AppendLine (";");
			
			if (marshal_moonerror) {
				text.Append (tabs);
				text.AppendLine ("\tif (error.Number != 0)");
				
				text.Append (tabs);
				text.AppendLine ("\t\tthrow CreateManagedException (error);");
			}
			
			if (marshal_string_returntype) {
				text.Append (tabs);
				text.AppendLine ("\treturn (result == IntPtr.Zero) ? null : Marshal.PtrToStringAnsi (result);");
			} else if (!is_void) {
				text.Append (tabs);
				text.AppendLine ("\treturn result;");
			}
		
			text.Append (tabs);
			text.Append ("}");
			text.AppendLine ();
		}
	}
}
