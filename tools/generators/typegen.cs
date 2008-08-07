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
using Generation;
using Generation.C;

class Property {
	public string Name;
	public string Value;
	public Property (string Name, string Value)
	{
		this.Name = Name;
		this.Value = Value;
	}
	public Property (string Name) : this (Name, null)
	{
	}
}

class Properties : Dictionary <string, Property> {
	public void Add (string args)
	{
		//
		// The format is like: /* @... */
		// Where ... is:
		//  key1[=value1],key2=[value2]
		//
		foreach (string pair in args.Split (new char [] {','}, StringSplitOptions.RemoveEmptyEntries)) {
			string [] v = pair.Split ('=');
			
			if (v.Length != 2 && v.Length != 1)
				throw new Exception (string.Format ("Invalid magic argument: '{0}', it must be of the format: key1[=value1],key2[=value2]", pair));
			
			if (ContainsKey (v [0]))
				throw new Exception (string.Format ("Invalid magic argument: '{0}' There already is a property with this name here.", v [0]));
			
			Property p = new Property (v [0]);
			if (v.Length == 2) {
				p.Value = v [1];
				if (p.Value.Length >= 2 && p.Value.StartsWith ("\"") && p.Value.EndsWith ("\"")) {
					p.Value = p.Value.Substring (1, p.Value.Length - 2);
				}
			}
			Add (p);
//			Console.WriteLine ("Got argument: '{0}' = '{1}'", v [0], v.Length > 1 ? v [1] : null);
		}
	}
	
	public void Add (Property p)
	{
		base.Add (p.Name, p);
	}
	
	public string GetValue (string name)
	{
		Property property;
		if (!TryGetValue (name, out property))
			return null;
		if (property != null)
			return property.Value;
		return null;
	}
	
	public void Dump ()
	{
		foreach (KeyValuePair <string, Property> p in this) {
			if (p.Value == null)
				Console.WriteLine ("/* @{0}*/", p.Key);
			else
				Console.WriteLine ("/* @{0}={1}*/", p.Key, p.Value.Value);
		}
	}
}

class GlobalInfo : MemberInfo {
	private List<MethodInfo> cppmethods_to_bind;
	
	public List<MethodInfo> CPPMethodsToBind {
		get {
			if (cppmethods_to_bind == null) {
				cppmethods_to_bind = new List<MethodInfo> ();
				foreach (MemberInfo member1 in Children.Values) {
					TypeInfo type = member1 as TypeInfo;
					if (type == null)
						continue;
					
					foreach (MemberInfo member2 in type.Children.Values) {
						MethodInfo method = member2 as MethodInfo;
						if (method == null)
							continue;
						if (method.Parent == null) {
							Console.WriteLine ("The method {0} in type {1} does not have its parent set.", method.Name, type.Name);
							continue;
						}
						if (!method.Properties.ContainsKey ("GenerateCBinding"))
							continue;
						cppmethods_to_bind.Add (method);
					}
				}
				cppmethods_to_bind.Sort (new Members.MembersSortedByFullName <MethodInfo> ());
			}
			return cppmethods_to_bind;
		}
	}
}

class MemberInfo {
	public MemberInfo Parent;
	public string Name;
	
	public bool IsPublic;
	public bool IsPrivate;
	public bool IsProtected;
	
	private string header; // The .h file where the member is defined
	private Members children;
	private Properties properties;
	private string fullname;
	private Nullable<int> silverlight_version;
	
	public bool IsPluginMember {
		get {
			if (Header == null || Header == string.Empty)
				return false;
			
			return Path.GetFileName (Path.GetDirectoryName (Header)) == "plugin";
		}
	}
	
	public bool IsSrcMember {
		get {
			if (Header == null || Header == string.Empty)
				return false;
			
			return Path.GetFileName (Path.GetDirectoryName (Header)) == "src";
		}
	}
	
	public virtual string Signature {
		get { return Name; }
	}
	
	public Members Children {
		get {
			if (children == null)
				children = new Members (this);
			return children;
		}
	}
	
	public Properties Properties {
		get {
			if (properties == null)
				properties = new Properties ();
			return properties;
		}
		set {
			properties = value;
		}
	}
	
	public string Header {
		get {
			if (header == null) {
				if (Parent != null)
					header = Parent.Header;
				else
					header = string.Empty;
			}
			return header;
		}
		set {
			header = value;
		}
	}
	
	public string FullName {
		get {
			if (fullname == null) {
				if (Parent != null) {
					fullname = Parent.FullName + "." + Name;
				} else {
					fullname = Name;
				}
			}
			return fullname;
		}
	}
	
	public int SilverlightVersion {
		get {
			string value = null;
			Property property;
			if (!silverlight_version.HasValue) {
				if (Properties.TryGetValue ("Version", out property)) {
					value = property.Value;
				} else if (Properties.TryGetValue ("SilverlightVersion", out property)) {
					value = property.Value;
				}
				
				if (value == null) {
					if (Parent != null)
						silverlight_version = new Nullable<int> (Parent.SilverlightVersion);
					else
						silverlight_version = new Nullable<int> (1);
				} else {
					if (value == "\"2\"" || value == "2" || value == "2.0")
						silverlight_version = new Nullable<int> (2);
					else if (value == "\"1\"" || value == "1" || value == "1.0")
						silverlight_version = new Nullable<int> (1);
					else
						throw new Exception (string.Format ("Invalid Version/SilverlightVersion: '{0}'", value));
				}
			}
			
			return silverlight_version.Value;
		}
	}
	
	public void Dump (int ident)
	{
		if (properties != null)
			properties.Dump ();
		Console.Write (new string ('\t', ident));
		Console.WriteLine ("{0} {1}", FullName, Header);
		if (children != null)
			foreach (MemberInfo info in children.Values)
				info.Dump (ident + 1);
	}
}

class Members : Dictionary <string, MemberInfo>{
	private MemberInfo [] sorted;
	private MemberInfo [] sorted_by_kind;
	private List<TypeInfo> sorted_types_by_kind;
	private StringBuilder kinds_for_enum;
	private MemberInfo parent;
	
	public Members (MemberInfo Parent) : base (StringComparer.Ordinal)
	{
		parent = Parent;
	}
	
	class TypeSortedByKind : IComparer <TypeInfo> {
		public int Compare (TypeInfo a, TypeInfo b)
		{
			return string.Compare (a.KindName, b.KindName);
		}
	}
	public class MembersSortedByFullName <T> : IComparer<T> where T : MemberInfo  {
		public int Compare (T a, T b)
		{
			return string.Compare (a.FullName, b.FullName);
		}
	}
	
	public IEnumerable <TypeInfo> SortedTypesByKind {
		get {
			if (sorted_types_by_kind == null) {
				sorted_types_by_kind = new List<TypeInfo> ();
				foreach (MemberInfo member in this.Values) {
					TypeInfo type = member as TypeInfo;
					if (type != null)
						sorted_types_by_kind.Add (type);
				}
				sorted_types_by_kind.Sort (new TypeSortedByKind ());
			}
			return sorted_types_by_kind;
		}
	}
	
	public IEnumerable <MemberInfo> SortedList {
		get {
			if (sorted == null) {
				int i = 0;
				sorted = new MemberInfo [Count];
				foreach (MemberInfo type in this.Values)
					sorted [i++] = type;
				Array.Sort (sorted, delegate (MemberInfo x, MemberInfo y) {
					return string.Compare (x.FullName, y.FullName);
				});
			}
			return sorted;
		}
	}
	public IEnumerable <MemberInfo> SortedByKindList {
		get {
			if (sorted_by_kind == null) {
				int i = 0;
				sorted_by_kind = new MemberInfo [Count];
				foreach (MemberInfo type in this.Values) {
					if (!(type is TypeInfo))
					    continue;
					if (type == null)
						continue;
					sorted_by_kind [i++] = type;
				}
				
				Array.Sort (sorted_by_kind, delegate (MemberInfo x, MemberInfo y) {
					if (x == null && y != null)
						return 1;
					else if (x != null && y == null)
						return -1;
					else if (x == null && y == null)
						return 0;
					else
						return string.Compare ((x as TypeInfo).KindName, (y as TypeInfo).KindName);
				});
			}
			return sorted_by_kind;
		}
	}
	
	public MemberInfo Add (MemberInfo value)
	{
		int counter = 1;
		string signature = value.Signature;
		
		if (!base.ContainsKey (signature)) {
			base.Add (signature, value);
		} else if (value.Name == "<anonymous>") {
			string tmp; 
			do {
				tmp = "<anonymous>" + counter.ToString ();
				counter++;
			} while (base.ContainsKey (tmp));
			value.Name = tmp;
			base.Add (value.Name, value);
		} else {
			throw new Exception (string.Format ("Could not add the member: {0}.{1} in parent {3}: There already is a member with the same signature ({2}).", parent.Name, value.Name, signature, parent.GetType ().FullName));
		}
		return value;
	}
	
	public StringBuilder GetKindsForEnum ()
	{
		if (kinds_for_enum == null) {
			kinds_for_enum = new StringBuilder ();
			foreach (MemberInfo info in SortedByKindList) {
				TypeInfo type = info as TypeInfo;
				
				if (type == null)
					continue;
				
				if (!type.Properties.ContainsKey ("IncludeInKinds")) {
					if (!type.ImplementsGetObjectType) {
						continue;
					}
				}
	
			 	kinds_for_enum.Append ("\t\t");
				kinds_for_enum.Append (type.KindName);
				kinds_for_enum.Append (",");
				if (type.Properties.ContainsKey ("SilverlightVersion"))// && type.Properties ["SilverlightVersion"].Value == "\"2\"")
					kinds_for_enum.Append ("// Silverlight 2.0 only");
				kinds_for_enum.AppendLine ();
			}
		}
		return kinds_for_enum;
	}
}

class MethodInfo : MemberInfo {
	public TypeReference ReturnType;
	public Parameters Parameters = new Parameters ();	
	public bool IsConstructor;
	public bool IsDestructor;
	public bool IsVirtual;
	public bool IsStatic;
	public bool IsAbstract;
	public bool IsCMethod;
	
	private MethodInfo c_method;
	private string signature;
	private Nullable<bool> contains_unknown_types;
	
	public bool ContainsUnknownTypes {
		get {
			if (!contains_unknown_types.HasValue) {
				if (!ReturnType.IsKnown) {
					contains_unknown_types = new Nullable<bool>(true);
				} else {
					foreach (ParameterInfo p in Parameters) {
						if (!p.ParameterType.IsKnown) {
							contains_unknown_types = new Nullable<bool> (true);
							break;
						}
					}
					if (!contains_unknown_types.HasValue)
						contains_unknown_types = new Nullable<bool> (false);
				}
			}
			return contains_unknown_types.Value;
		}
	}
	
	public MethodInfo CMethod {
		get {
			if (IsCMethod)
				return null;
			
			if (c_method == null) {
				c_method = new MethodInfo ();
				c_method.IsStatic = IsStatic;
				c_method.IsConstructor = IsConstructor;
				c_method.IsDestructor = IsDestructor;
				c_method.Name = Helper.CppToCName (Parent.Name, Name);
				c_method.Properties = Properties;
				c_method.ReturnType = ReturnType == null ? new TypeReference ("void") : ReturnType;
				c_method.Parent = Parent;
								
				if (!IsStatic && !IsConstructor) {
					ParameterInfo parameter = new ParameterInfo ();
					parameter.Name = "instance";
					parameter.ParameterType = new TypeReference (Parent.Name + "*");
					c_method.Parameters.Add (parameter);
				}
				foreach (ParameterInfo parameter in Parameters)
					c_method.Parameters.Add (parameter);
				
			}
			return c_method;
		}
	}
	
	public override string Signature {
		get { return GetSignature (); }
	}
	
	public string GetSignature ()
	{
		StringBuilder s;
		
		if (signature != null)
			return signature;
		
		s = new StringBuilder ();
		s.Append (Name);
		s.Append ("(");
		foreach (ParameterInfo parameter in Parameters) {
			if (parameter.ParameterType.IsConst)
				s.Append ("const ");
			s.Append (parameter.ParameterType.Value);
			s.Append (",");
		}
		if (s [s.Length - 1] == ',')
			s.Length--;
		s.Append (")");
		
		signature = s.ToString ();
		return signature;
	}
	
	public void WriteFormatted (StringBuilder text)
	{
		ReturnType.WriteFormatted (text);
		text.Append (" ");
		text.Append (Name);
		text.Append (" (");
		for (int i = 0; i < Parameters.Count; i++) {
			if (i > 0)
				text.Append (", ");
			Parameters [i].WriteFormatted (text);
		}
		text.Append (");");
		
	}
}

class ParameterInfo : MemberInfo {
	public TypeReference ParameterType;
	
	public bool DisableWriteOnce;
	public string ManagedWrapperCode;// Used by GeneratePInvoke
	
	public void WriteSignature (StringBuilder text, SignatureType type)
	{
		ParameterType.Write (text, type);
		text.Append (" ");
		text.Append (Name);
	}
	
	public void WriteCall (StringBuilder text, SignatureType type)
	{
		if (type != SignatureType.Native) {
			if (ParameterType.IsRef)
				text.Append ("ref ");
			if (ParameterType.IsOut)
				text.Append ("out ");
		}
		if (type == SignatureType.Managed && ManagedWrapperCode != null)
			text.Append (ManagedWrapperCode);
		else
			text.Append (Name);
	}
	
	public void WriteFormatted (StringBuilder text)
	{
		ParameterType.WriteFormatted (text);
		text.Append (" ");
		text.Append (Name);
	}
}

class Parameters : List <ParameterInfo> {
	public void Write (StringBuilder text, SignatureType type, bool as_call)
	{
		bool first_done = false;
		text.Append (" (");
		foreach (ParameterInfo parameter in this) {
			if (parameter.DisableWriteOnce) {
				parameter.DisableWriteOnce = false;
				continue;
			}
			
			if (first_done)
				text.Append (", ");
			first_done = true;
			
			if (as_call)
				parameter.WriteCall (text, type);
			else
				parameter.WriteSignature (text, type);
		}
		text.Append (")");
	}
}

enum SignatureType {
	Native,
	Managed,
	PInvoke
}

class TypeReference {
	public string Value;
	public bool IsConst;
	public bool IsRef;
	public bool IsOut;
	
	private string managed_type;
	private Nullable <bool> is_known;
	
	public TypeReference () {}
	public TypeReference (string value)
	{
		this.Value = value;
	}
	
	public void WriteFormatted (StringBuilder text)
	{
		if (IsConst)
			text.Append ("const ");
		text.Append (Value);
	}
	
	public void Write (StringBuilder text, SignatureType type)
	{
		if (IsConst && type == SignatureType.Native)
			text.Append ("const ");
		
		if (type != SignatureType.Native) {
			if (IsRef)
				text.Append ("ref ");
			if (IsOut)
				text.Append ("out ");
		}
		
		if (type == SignatureType.Native) {
			text.Append (Value);
		} else {
			text.Append (GetManagedType ());
		}
	}
	
	public bool IsKnown {
		get {
			if (!is_known.HasValue) {
				if (string.IsNullOrEmpty (GetManagedType ()))
					is_known = new Nullable<bool> (false);
				else if (GetManagedType ().Contains ("Unknown"))
					is_known = new Nullable<bool> (false);
				else
					is_known = new Nullable<bool> (true);
			}
			return is_known.Value;
		}
	}
	
	public string GetManagedType ()
	{
		if (managed_type == null) {
			switch (Value) {
			case "bool":
			case "void":
				managed_type = Value;
				break;
			case "MoonError*":
				IsOut = true;
				managed_type = "MoonError";
				break;
			case "void*":
			case "gpointer":
			case "DependencyObject*":
			case "DependencyProperty*":
			case "Types*":
			case "Type*":
			case "Value*":
				managed_type = "IntPtr";
				break;
			case "NativePropertyChangedHandler*":
				managed_type = "Mono.NativePropertyChangedHandler";
				break;
			case "char*":
				managed_type = "string";
				break;
			case "Type::Kind":
				managed_type = "Kind";
				break;
			default:
				managed_type = "/* Unknown: '" + Value + "' */";
				break;
			}
		}
		return managed_type;
	}
}

class FieldInfo : MemberInfo {
	public TypeReference FieldType;
	public string BitField;
	public bool IsConst;
	public bool IsStatic;
	public bool IsExtern;
}

class TypeInfo : MemberInfo {
	private string _KindName; // The name as it appears in the Kind enum (STRING, POINT_ARRAY, etc)
	private string c_constructor; // The C constructor
	private List<string> events;
	
	public TypeReference Base; // The parent type
	public bool IsStruct; // class or struct
	public int TotalEventCount;

	public bool Include; // Force inclusion of this type into the type system (for manual types, char, point[], etc)
	public bool IsValueType;
	public bool IsEnum;
	
	public List<string> Events {
		get {
			if (events == null) {
				events = new List<string> ();

				foreach (MemberInfo member in Children.Values) {
					FieldInfo field = member as FieldInfo;
					if (field == null)
						continue;
				
					if (!field.Name.EndsWith ("Event"))
						continue;
					
					if (!field.IsStatic)
						continue;
						
					events.Add (field.Name.Substring (0, field.Name.LastIndexOf ("Event")));
				}
				events.Sort ();
			}
			return events;
		}
	}
	
	public bool GenerateCBindingCtor {
		get {
			bool result = false;
			Property property;
			
			if (Properties.TryGetValue ("GenerateCBindingCtor", out property))
				return property.Value != null;
			
			foreach (MemberInfo member in Children.Values) {
				MethodInfo method = member as MethodInfo;
				
				if (method == null)
					continue;
				
				if (method.Parameters.Count != 0)
					continue;
				
				if (!method.Properties.ContainsKey ("GenerateCBinding"))
					continue;
				
				if (method.IsConstructor) {
					result = true;
					break;
				}
			}
			Properties.Add (new Property ("GenerateCBindingCtor", result ? "true" : null));
			return result;
		}
	}
	
	public string C_Constructor {
		get {
			if (IsEnum)
				return string.Empty;
			
			if (c_constructor == null)
				c_constructor = Helper.CppToCName (Name, Name);
			
			return c_constructor;
		}
	}
	
	public bool ImplementsGetObjectType {
		get {
			foreach (MemberInfo child in Children.Values) {
				if (child is MethodInfo && child.Name == "GetObjectType")
					return true;
			}
			return false;
		}
	}
	
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
			return ((TypeInfo) Parent.Children [Base.Value]).GetEventId (Event);
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
		int result;
		if (Base == null || string.IsNullOrEmpty (Base.Value)) {
			result = -1;
		} else if (Parent == null) {
			result = -2;
		} else if (!Parent.Children.ContainsKey (Base.Value)) {
			result = -3;
		} else
			result = ((TypeInfo) Parent.Children [Base.Value]).GetTotalEventCount ();
		//Console.WriteLine ("GetBaseEventCount '{2}' {0}, base: '{1}'", result, Base != null ? Base.Value : "<no base>", FullName);
		return result < 0 ? 0 : result;
	}
	
	public int GetTotalEventCount ()
	{
		return Events.Count + GetBaseEventCount ();
	}
	
	public string ContentProperty {
		get {
			if (Properties.ContainsKey ("ContentProperty"))
				return Properties ["ContentProperty"].Value;
			return null;
		}
	}
	
	public TypeInfo ()
	{
	}
	
	public TypeInfo (string Name, string KindName, string Base, bool Include)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = new TypeReference (Base);
		this.Include = Include;
		if (Include)
			Properties.Add (new Property ("IncludeInKinds"));
	}
	public TypeInfo (string Name, string KindName, string Base, bool Include, int SLVersion)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = new TypeReference (Base);
		this.Include = Include;
		if (Include)
			Properties.Add (new Property ("IncludeInKinds"));
		Properties.Add (new Property ("SilverlightVersion", "\"" + SLVersion.ToString () + "\""));
	}
	
}

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
	}
	
	static GlobalInfo GetTypes2 ()
	{
		GlobalInfo all = new GlobalInfo ();
		Tokenizer tokenizer = new Tokenizer (Directory.GetFiles (Path.Combine (Environment.CurrentDirectory, "src"), "*.h"));
		
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
		all.Children.Add (new TypeInfo ("double*", "DOUBLE_ARRAY", null, true));
		all.Children.Add (new TypeInfo ("Point*", "POINT_ARRAY", null, true));
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
					
					// Field
					do {
						//Console.WriteLine ("ParseMembers: found field '{0}'", name);
						
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
		
		text.AppendLine ("/* \n\tthis file was autogenerated. do not edit this file \n */\n");
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
		
		Generation.Helper.WriteAllText (Path.Combine (Path.Combine (moon_moonlight_dir, "Mono"), "Types.g.cs"), text.ToString ());
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
		
		header.AppendLine ("/* \n\tthis file was autogenerated. do not edit this file \n */\n");
		impl.AppendLine ("/* \n\tthis file was autogenerated. do not edit this file \n */\n");
		
		header.AppendLine ("#ifndef __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ("#define __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ("");
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
		
		impl.AppendLine ("#include \"config.h\"");
		impl.AppendLine ();
		impl.AppendLine ("#include <stdlib.h>");
		impl.AppendLine ("#include <stdio.h>");
		impl.AppendLine ();
		impl.AppendLine ("#include \"cbinding.h\"");
		impl.AppendLine ("");
		
		foreach (MemberInfo member in methods) {
			MethodInfo method = (MethodInfo) member;			
			
			if (last_type != method.Parent.Name) {
				last_type = method.Parent.Name;
				foreach (StringBuilder text in new StringBuilder [] {header, impl}) {
					text.AppendLine ("/* ");
					text.Append (" * ");
					text.AppendLine (last_type);
					text.AppendLine (" */ ");
					text.AppendLine ("");
				}
			}
			
			WriteHeaderMethod (method.CMethod, method, header);
			WriteImplMethod (method.CMethod, method, impl);
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
		
		WriteMethodIfVersion (cmethod, text, false);
		if (cmethod.Properties.ContainsKey ("GeneratePInvoke"))
			text.AppendLine ("/* @GeneratePInvoke */");
		cmethod.ReturnType.Write (text, SignatureType.Native);
		text.Append (" ");
		text.Append (cmethod.Name);
		cmethod.Parameters.Write (text, SignatureType.Native, false);
		text.AppendLine (";");
		WriteMethodIfVersion (cmethod, text, true);
	}
	
	static void WriteImplMethod (MethodInfo cmethod, MethodInfo cppmethod, StringBuilder text)
	{
		WriteMethodIfVersion (cmethod, text, false);
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
		WriteMethodIfVersion (cmethod, text, true);
		text.AppendLine ();
	}
	
	static void GenerateTypeStaticCpp (GlobalInfo all)
	{
		string header;
		List<string> headers = new List<string> ();
		List<string> headers2 = new List<string> ();
		
		StringBuilder text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated, do not edit this file directly");
		text.AppendLine (" */");
		text.AppendLine ("#include <config.h>");
		text.AppendLine ("#include <stdlib.h>");

		headers.Add ("cbinding.h");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.C_Constructor == string.Empty || t.C_Constructor == null || !t.GenerateCBindingCtor) {
				//Console.WriteLine ("{0} does not have a C ctor", t.FullName);
				if (t.GetTotalEventCount () == 0)
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
		
		// Set the event ids for all the events
		text.AppendLine ("");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.Events == null || t.Events.Count == 0)
				continue;
			
			foreach (string e in t.Events) {
				text.Append ("const int ");
				text.Append (t.Name);
				text.Append ("::");
				text.Append (e);
				text.Append ("Event = ");
				text.Append (t.GetEventId (e));
				text.AppendLine (";");
			}
		}

		// Create the arrays of event names for the classes which have events
		text.AppendLine ("");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.Events == null || t.Events.Count == 0)
				continue;
			
			text.Append ("const char *");
			text.Append (t.Name);
			text.Append ("_Events [] = { ");
			
			if (t.Events != null && t.Events.Count != 0){
				for (int k = 0; k < t.Events.Count; k++) {
					text.Append ("\"");
					text.Append (t.Events [k]);
					text.Append ("\", ");
				}
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

			if (type.SilverlightVersion == 2)
				text.AppendLine ("#if SL_2_0");
			text.AppendLine (string.Format (@"	{{ {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, NULL, NULL }}, ",
			                                "Type::" + type.KindName, 
			                                "Type::" + (parent != null ? parent.KindName : "INVALID"),
			                                type.IsValueType ? "true" : "false",
			                                "\"" + type.Name + "\"", 
			                                "\"" + type.KindName + "\"", 
			                                type.Events.Count,
			                                type.GetTotalEventCount (),
			                                events,
			                                (type.C_Constructor != null && type.GenerateCBindingCtor) ? string.Concat ("(create_inst_func *) ", type.C_Constructor) : "NULL", 
			                                type.ContentProperty != null ? string.Concat ("\"", type.ContentProperty, "\"") : "NULL"
			                                )
			                 );
			if (type.SilverlightVersion == 2) {
				text.AppendLine ("#else");
				text.AppendLine (string.Format ("	{{ Type::INVALID, Type::INVALID, false, \"2.0 specific type '{0}'\", {1}, 0, 0, NULL, NULL, NULL, NULL, NULL }}, ",
								type.KindName,
								"\"" + type.KindName + "\""));
				text.AppendLine ("#endif");
			}
			
		}
		text.AppendLine ("\t{ Type::LASTTYPE, Type::INVALID, false, NULL, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL }");
		text.AppendLine ("};");
				
		Helper.WriteAllText ("src/type-generated.cpp", text.ToString ());
	}
	
	static void GenerateTypeH (GlobalInfo all)
	{
		const string file = "src/type.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");
		
		contents = contents.Replace ("/*DO_KINDS*/", all.Children.GetKindsForEnum ().ToString ());

		text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from type.h.in, do not edit this file directly");
		text.AppendLine (" */");
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

		text.AppendLine ("/* \n\tthis file was autogenerated from moon/src/value.h.  do not edit this file \n */");
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

		result.AppendLine ("/*");
		result.AppendLine (" * Automatically generated from value.h.in, do not edit this file directly");
		result.AppendLine (" */");
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
		
		text.AppendLine ("/* \n\tthis file was autogenerated. do not edit this file \n */\n");
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
