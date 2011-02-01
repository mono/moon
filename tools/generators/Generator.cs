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
using System.Linq;
using System.Collections.Generic;
using System.IO;
using System.Text;


class Generator {
	delegate void output_native_type_delegate (string t, string k);
	static StringBuilder forward_decls = new StringBuilder ();
	static StringBuilder cbinding_requisites = new StringBuilder ();

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
		GenerateTypes_G (info);

		GenerateDPs (info);
		GenerateManagedDPs (info);
		GenerateManagedDOs (info);

		GenerateManagedEvents (info);

		GenerateJSBindings (info);
	}

	static void GenerateJSBindings (GlobalInfo all)
	{
		List<MethodInfo> methods;
		methods = all.JSMethodsToBind;
		StringBuilder mappings = new StringBuilder ();
		StringBuilder header = new StringBuilder ();
		StringBuilder body = new StringBuilder ();

		MemberInfo current = null;
		Dictionary<string, List<MethodInfo>> types = new Dictionary<string, List<MethodInfo>> ();

		foreach (MemberInfo m in methods) {
			MethodInfo method = (MethodInfo) m;
			string name = method.Annotations.GetValue ("GenerateJSBinding");
			if (name == null)
				name = method.Name;
			mappings.AppendLine ("\tMoonId_" + method.Parent.Name + "_" + name + ",");

			if (current != method.Parent) {
				current = method.Parent;
			}
			if (!types.ContainsKey (current.Name))
				types.Add (current.Name, new List<MethodInfo>());
			types[current.Name].Add (method);
		}


		foreach (KeyValuePair<string, List<MethodInfo>> t in types) {
			string parent = t.Key;

			header.AppendLine ("/*** Moonlight" + parent + "Class *********/");

			header.AppendLine ("struct Moonlight" + parent + "Type : MoonlightDependencyObjectType {");
			header.AppendLine ("\tMoonlight" + parent + "Type ();");
			header.AppendLine ("};");
			header.AppendLine ();
			header.AppendLine ("extern Moonlight" + parent + "Type *Moonlight" + parent + "Class;");
			header.AppendLine ();
			header.AppendLine ("struct Moonlight" + parent + "Object : MoonlightDependencyObjectObject {");
			header.AppendLine ("\tMoonlight" + parent + "Object (NPP instance) : MoonlightDependencyObjectObject (instance)");
			header.AppendLine ("\t{");
			header.AppendLine ("\t\tmoonlight_type = Type::" + parent.ToUpper() + ";");
			header.AppendLine ("\t}");
			header.AppendLine ();
			header.AppendLine ("\tvirtual bool Invoke (int id, NPIdentifier name,");
			header.AppendLine ("\t\tconst NPVariant *args, guint32 argCount, NPVariant *result);");
			header.AppendLine ();
			header.AppendLine ("};");

			body.AppendLine ("/*** Moonlight" + parent + "Class *********/");
			body.AppendLine ("static NPObject *");
			body.AppendLine ("moonlight_" + parent.ToLower() + "_allocate (NPP instance, NPClass *klass)");
			body.AppendLine ("{");
			body.AppendLine ("	return new Moonlight" + parent + "Object (instance);");
			body.AppendLine ("}");
			body.AppendLine ();
			body.AppendLine ("static const MoonNameIdMapping moonlight_" + parent.ToLower() + "_mapping[] = {");
			
			for (int i = 0; i < t.Value.Count; i++) {
				MethodInfo method = t.Value[i];
				string name = method.Annotations.GetValue ("GenerateJSBinding");
				if (name == null)
					name = method.Name;
				string id = "MoonId_" + parent + "_" + name;
				body.Append ("	{\"" + name.ToLower () + "\", " + id + "}");

				if (i < t.Value.Count - 1)
					body.Append (",");
				body.AppendLine ("");

			}

			body.AppendLine ("};");
			body.AppendLine ("");
			body.AppendLine ("bool");
			body.AppendLine ("Moonlight" + parent + "Object::Invoke (int id, NPIdentifier name,");
			body.AppendLine ("				   const NPVariant *args, guint32 argCount,");
			body.AppendLine ("				   NPVariant *result)");
			body.AppendLine ("{");
			body.AppendLine ("	" + parent + " *dob = (" + parent + "*)GetDependencyObject ();");
			body.AppendLine ("");
			body.AppendLine ("	switch (id) {");

			foreach (MethodInfo method in t.Value) {
				string name = method.Annotations.GetValue ("GenerateJSBinding");
				if (name == null)
					name = method.Name;
				string id = "MoonId_" + parent + "_" + name;
				body.AppendLine ();
				body.AppendLine ("\t\tcase " + id + ": {");

				bool errorcheck = false;
				string argcodes = "";
				List<string> args = new List<string>();
				List<string> parms = new List<string>();
				for (int i = 0; i < method.Parameters.Count; i++) {
					ParameterInfo parameter = method.Parameters[i];

					if (parameter.ParameterType.Value == "MoonError*") {
						errorcheck = true;
					} else {
						argcodes += parameter.ParameterType.GetNPType ();

						switch (parameter.ParameterType.GetNPType ()) {
							case "i":
								args.Add ("\t\t\tint arg" + i + " = NPVARIANT_TO_INT32 (args[" + i + "]);");
								parms.Add ("arg" + i);
								break;
							case "s":
								args.Add ("\t\t\tchar *arg" + i + " = STRDUP_FROM_VARIANT (args[" + i + "]);");
								parms.Add ("arg" + i);
								break;
							case "o":
								args.Add ("\t\t\tNPObject *obj" + i + " = NPVARIANT_TO_OBJECT (args[" + i + "]);");
								args.Add ("\t\t\tif (!npobject_is_dependency_object (obj" + i + "))");
								args.Add ("\t\t\t\tTHROW_JS_EXCEPTION (\"" + name + "\");");
								args.Add ("\t\t\tDependencyObject *arg" + i + " = ((MoonlightDependencyObjectObject *) obj" + i + ")->GetDependencyObject();");
								parms.Add ("(" + parameter.ParameterType.WriteFormatted () + ") arg" + i);
								break;
							case "d":
								args.Add ("\t\t\tdouble arg" + i + " = NPVARIANT_TO_DOUBLE (args[" + i + "]);");
								parms.Add ("arg" + i);
								break;
							case "b":
								args.Add ("\t\t\tbool arg" + i + " = NPVARIANT_TO_BOOLEAN (args[" + i + "]);");
								parms.Add ("arg" + i);
								break;
						}
					}
				}

				if (argcodes != "") {
					body.AppendLine ("\t\t\tif (!check_arg_list (\"" + argcodes + "\", argCount, args))");
					body.AppendLine ("\t\t\t\tTHROW_JS_EXCEPTION (\"" + name + "\");");
				}

				if (errorcheck) {
					body.AppendLine ("\t\t\tMoonError err;");
					parms.Add ("&err");
				}

				if (args.Count > 0)
					body.AppendLine (String.Join ("\n", args.ToArray()));
				
				body.Append ("\t\t\t");

				if (method.ReturnType.GetNPType () != "v") {
					method.ReturnType.WriteFormatted (body);
					body.AppendLine (" ret = dob->" + method.Name + "(" + String.Join (",", parms.ToArray ()) + ");");
				} else
					body.AppendLine ("dob->" + method.Name + "(" + String.Join (",", parms.ToArray ()) + ");");

				for (int i = 0; i < method.Parameters.Count; i++) {
					ParameterInfo parameter = method.Parameters[i];
					if (parameter.ParameterType.GetNPType () == "s")
						body.AppendLine ("g_free (arg" + i + ");");
				}

				if (errorcheck)
					body.AppendLine ("\t\t\tif (err.number != 0) THROW_JS_EXCEPTION (err.message);");

				switch (method.ReturnType.GetNPType ()) {
					case "i":
						body.AppendLine ("\t\t\tINT32_TO_NPVARIANT (ret, *result);");
						break;
					case "s":
						body.AppendLine ("\t\t\tstring_to_npvariant (ret, *result);");
						break;
					case "o":
						body.AppendLine ("\t\t\tif (ret)");
						body.AppendLine ("\t\t\t\tOBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), ret), *result);");
						body.AppendLine ("\t\t\telse");
						body.AppendLine ("\t\t\t\tNULL_TO_NPVARIANT (*result);");
						break;
					case "d":
						body.AppendLine ("\t\t\tDOUBLE_TO_NPVARIANT (ret, *result);");
						break;
					case "b":
						body.AppendLine ("\t\t\tBOOLEAN_TO_NPVARIANT (ret, *result);");
						break;
					case "v":
						body.AppendLine ("\t\t\tVOID_TO_NPVARIANT (*result);");
						break;
				}

				body.AppendLine ("\t\t\treturn true;");
				body.AppendLine ("\t\t\tbreak;");
				body.AppendLine ("\t\t}");
			}
			
			body.AppendLine ("\t}");
			body.AppendLine ();
			
			body.AppendLine ("\treturn MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);");
			body.AppendLine ("}");
			body.AppendLine ();
			
			body.AppendLine ("Moonlight" + parent + "Type::Moonlight" + parent + "Type ()");
			body.AppendLine ("{");
			body.AppendLine ("	AddMapping (moonlight_" + parent.ToLower() + "_mapping, G_N_ELEMENTS (moonlight_" + parent.ToLower() + "_mapping));");
			body.AppendLine ();
			body.AppendLine ("	allocate = moonlight_" + parent.ToLower() + "_allocate;");
			body.AppendLine ("}");
		}




		string file = "plugin/plugin-class.h";

		string contents = File.ReadAllText (file + ".in");
		contents = contents.Replace ("/*MAP_IDS*/", mappings.ToString());
		contents = contents.Replace ("/*MAP_HEADERS*/", header.ToString());

		StringBuilder text = new StringBuilder ();
		Helper.WriteWarningGenerated (text);
		contents = text.ToString () + contents;
		Helper.WriteAllText (file, contents);

		file = "plugin/plugin-class.g.cpp";
		contents = File.ReadAllText (file + ".in");
		contents = contents.Replace ("/*MAP_BODY*/", body.ToString());

		text = new StringBuilder ();
		Helper.WriteWarningGenerated (text);
		contents = text.ToString () + contents;
		Helper.WriteAllText (file, contents);
	}

	static void GenerateManagedEvents (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		string class_dir = Path.Combine (base_dir, "class");
		string sys_win_dir = Path.Combine (Path.Combine (class_dir, "System.Windows"), "System.Windows");
		string filename = Path.Combine (sys_win_dir, "Events.g.cs");
		string previous_namespace = "";
		List<TypeInfo> sorted_types = new List<TypeInfo>  ();
		StringBuilder text = new StringBuilder ();
		Dictionary <TypeInfo, List<FieldInfo>> types = new Dictionary<TypeInfo,List<FieldInfo>> ();
		
		foreach (FieldInfo field in all.Events) {
			TypeInfo parent = field.Parent as TypeInfo;
			List <FieldInfo> fields;
			string managed_parent = field.Annotations.GetValue ("ManagedDeclaringType");
			
			if (!field.IsEvent || !field.GenerateManagedEvent)
				continue;
			
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

		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tinternal static class EventIds {");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.GetEventCount () == 0)
				continue;
				
				
			foreach (FieldInfo field in t.Events) {
				text.Append ("\t\tpublic const int ");
				text.Append (t.Name);
				text.Append ("_");
				text.Append (field.EventName);
				text.Append ("Event = ");
				text.Append (t.GetEventId (field));
				text.AppendLine (";");
			}
		}
		text.AppendLine ("\t}");

		text.AppendLine ("\tinternal static partial class Events {");
		text.AppendLine ("\t\tpublic static UnmanagedEventHandler CreateDispatcherFromEventId (int eventId, Delegate value) {");
		text.AppendLine ("\t\t\tswitch (eventId) {");

		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.GetEventCount () == 0)
				continue;


			foreach (FieldInfo field in t.Events) {
				if (field.GenerateManagedEventField == false)
					continue;
				text.Append ("\t\t\t\tcase EventIds.");
				text.Append (t.Name);
				text.Append ("_");
				text.Append (field.EventName);
				text.Append ("Event: return Events.");

				text.Append (GetDispatcherMethodName(field.EventDelegateType));
				text.Append (" ((");
				text.Append (field.EventDelegateType);
				text.Append (") value)");
				text.AppendLine (";");
			}
		}
		text.AppendLine ("\t\t\t\tdefault: throw new NotSupportedException ();");
		text.AppendLine ("\t\t\t}");
		text.AppendLine ("\t\t}");
		text.AppendLine ("\t}");

		text.AppendLine ("}");
		
		sorted_types.Sort (new Members.MembersSortedByManagedFullName <TypeInfo> ());
		for (int i = 0; i < sorted_types.Count; i++) {
			TypeInfo type = sorted_types [i];
			List<FieldInfo> fields = types [type];
			TypeInfo parent = type;
			string ns;
			
			ns = parent.Namespace;
			
			if (string.IsNullOrEmpty (ns)) {
				Console.WriteLine ("The type '{0}' in {1} does not have a namespace annotation.", parent.FullName, parent.Header);
				continue;
			}

			if (type.Annotations.ContainsKey ("ManagedEvents")) {
				string event_mode = type.Annotations.GetValue ("ManagedEvents");
				switch (event_mode) {
				case "None":
				case "Manual":
					continue;
				case "Generate":
					break;
				default:
					throw new Exception (string.Format ("Invalid value '{0}' for ManagedEvents in '{1}'", event_mode, type.FullName));
				}
			}

			if (ns == "None") {
				Console.WriteLine ("'{0}''s Namespace = 'None', this type should have set @ManagedEvents=Manual to not create events.", type.FullName);
				continue;
			}
			
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
			text.Append (parent.ManagedName);
			text.AppendLine (" {");
			
			fields.Sort (new Members.MembersSortedByName <FieldInfo> ());
			
			
			foreach (FieldInfo field in fields) {
				if (!field.IsEvent)
					continue;

				text.AppendLine ();
				
				// property accessor
				text.Append ("\t\t");
				Helper.WriteAccess (text, field.GetManagedAccessorAccess ());
				text.Append (" event ");
				text.Append (field.EventDelegateType);
				text.Append (" ");
				text.Append (field.EventName);
				text.AppendLine (" {");
				
				// property getter
				text.Append ("\t\t\t");
				if (field.GetManagedAccessorAccess () != field.GetManagedGetterAccess ()) {
					Helper.WriteAccess (text, field.GetManagedGetterAccess ());
					text.Append (" ");
				}
				
				text.Append ("add { RegisterEvent (EventIds.");
				text.Append (field.ParentType.Name);
				text.Append ("_");
				text.Append (field.EventName);
				text.Append ("Event, value, Events.");
				text.Append (GetDispatcherMethodName(field.EventDelegateType));
				text.Append (" (value)");
				text.AppendLine ("); }");
				
				text.Append ("\t\t\t");
				text.Append ("remove { UnregisterEvent (EventIds.");
				text.Append (field.ParentType.Name);
				text.Append ("_");
				text.Append (field.EventName);
				text.Append ("Event, value);");
				text.AppendLine (" }");

				text.AppendLine ("\t\t}");

				if (field.GenerateManagedEventField) {
					text.Append ("\t\t");
					text.Append (string.Format ("public static readonly RoutedEvent {0}Event = new RoutedEvent (EventIds.{1}_{2}Event);", field.EventName, field.ParentType.Name, field.EventName));
					text.AppendLine ();
				}
			}
			
			text.AppendLine ("\t}");		
		}

		text.AppendLine ("}");

		Helper.WriteAllText (filename, text.ToString ());
	}

	static string GetDispatcherMethodName (string delegateType)
	{
		if (delegateType.Contains ("<")) {
			string[] delegate_types = delegateType.Split ('<', '>');
			return string.Format ("Create{0}{1}Dispatcher", delegate_types[1], delegate_types[0]);
		}
		else
			return string.Format ("Create{0}Dispatcher", delegateType);
	}

	static void GenerateManagedDOs (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		string class_dir = Path.Combine (base_dir, "class");
		string sys_win_dir = Path.Combine (Path.Combine (class_dir, "System.Windows"), "System.Windows");
		string filename = Path.Combine (sys_win_dir, "DependencyObject.g.cs");
		string previous_namespace = "";
		StringBuilder text = new StringBuilder ();
		List<TypeInfo> types = all.GetDependencyObjects ();
				
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
		text.AppendLine ("using System.Windows.Media.Media3D;");
		text.AppendLine ("using System.Windows.Media.Animation;");
		text.AppendLine ("using System.Windows.Shapes;");
		text.AppendLine ();
		
		for (int i = 0; i < types.Count; i++) {
			TypeInfo type = types [i];
			bool call_initialize = type.Annotations.ContainsKey ("CallInitialize");
			string ns;
			
			ns = type.Namespace;
			
			if (string.IsNullOrEmpty (ns)) {
				Console.WriteLine ("The type '{0}' in {1} does not have a namespace annotation.", type.FullName, Path.GetFileName (type.Header));
				continue;
			}
			
			if (ns == "None") {
				//Console.WriteLine ("The type '{0}''s Namespace annotation is 'None'.", type.FullName);
				continue;
			}
			
			string check_ns = Path.Combine (Path.Combine (Path.Combine (class_dir, "System.Windows"), ns), type.ManagedName.Replace ("`1", "") + ".cs");
			if (!File.Exists (check_ns)) {
				Console.WriteLine ("The file {0} does not exist, did you annotate the class with the wrong namespace?", check_ns);
				continue;
			}
			
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

			if (type.ContentProperty != null)
				text.AppendFormat ("\t[ContentProperty (\"{0}\")]\n", type.ContentProperty);
			text.Append ("\tpartial class ");
			text.Append (type.ManagedName.Replace ("`1", "<T>"));
			text.AppendLine (" {");
			
			// Public ctor
			if (!string.IsNullOrEmpty (type.C_Constructor)) {
				string access = "Public";
				foreach (MemberInfo member in type.Children.Values) {
					MethodInfo method = member as MethodInfo;
					
					if (method == null || !method.IsConstructor || method.IsStatic)
						continue;
					
					if (method.Parameters.Count != 0)
						continue;
					
					if (method.Annotations.ContainsKey ("ManagedAccess"))
						access = method.Annotations.GetValue ("ManagedAccess");
					break;
				}
				
				
				text.Append ("\t\t");
				Helper.WriteAccess (text, access);
				text.Append (" ");
				text.Append (type.ManagedName.Replace ("`1", ""));
				text.Append (" () : base (NativeMethods.");
				text.Append (type.C_Constructor);
				text.Append (" (), true)");
				if (call_initialize) {
					text.AppendLine ();
					text.AppendLine ("\t\t{");
					text.AppendLine ("\t\t\tInitialize ();");
					text.AppendLine ("\t\t}");
				} else {
					text.AppendLine (" {}");
				}
			}
			
			// Internal ctor
			text.Append ("\t\tinternal ");
			text.Append (type.ManagedName.Replace ("`1", ""));
			text.Append (" (IntPtr raw, bool dropref) : base (raw, dropref)");
			if (call_initialize) {
				text.AppendLine ();
				text.AppendLine ("\t\t{");
				text.AppendLine ("\t\t\tInitialize ();");
				text.AppendLine ("\t\t}");
			} else {
				text.AppendLine (" {}");
			}

			text.AppendLine ("\t}");
		}
		text.AppendLine ("}");		
		
		Helper.WriteAllText (filename, text.ToString ());
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
			string managed_parent = field.Annotations.GetValue ("ManagedDeclaringType");
			
			if (field.Annotations.GetValue ("GenerateManagedDP") == "false")
				continue;
			
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
		text.AppendLine ("using System.Windows.Media.Media3D;");
		text.AppendLine ("using System.Windows.Media.Animation;");
		text.AppendLine ("using System.Windows.Media.Effects;");
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
				Console.WriteLine ("The type '{0}' in {1} does not have a namespace annotation.", parent.FullName, parent.Header);
				continue;
			}

			if (type.Annotations.ContainsKey ("ManagedDependencyProperties")) {
				string dp_mode = type.Annotations.GetValue ("ManagedDependencyProperties");
				switch (dp_mode) {
				case "None":
				case "Manual":
					continue;
				case "Generate":
					break;
				default:
					throw new Exception (string.Format ("Invalid value '{0}' for ManagedDependencyProperties in '{1}'", dp_mode, type.FullName));
				}
			}

			if (ns == "None") {
				Console.WriteLine ("'{0}''s Namespace = 'None', this type should have set @ManagedDependencyProperties=Manual to not create DPs.", type.FullName);
				continue;
			}
			
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
			text.Append (parent.ManagedName);
			text.AppendLine (" {");
			
			fields.Sort (new Members.MembersSortedByName <FieldInfo> ());
			
			// The DP registration
			foreach (FieldInfo field in fields) {
				bool conv_int_to_double = field.GetDPManagedPropertyType (all) == "int" && field.GetDPPropertyType (all).Name == "double";
				
				text.Append ("\t\t");
				Helper.WriteAccess (text, field.GetManagedFieldAccess ());
				text.Append (" static readonly DependencyProperty ");
				text.Append (field.Name);
				text.Append (" = DependencyProperty.Lookup (Kind.");
				text.Append (field.ParentType.KindName);
				text.Append (", \"");
				text.Append (field.GetDependencyPropertyName ());
				text.Append ("\", typeof (");
				if (conv_int_to_double)
					text.Append ("double");
				else
					text.Append (field.GetDPManagedPropertyType (all));
				text.AppendLine ("));");
			}
			
			foreach (FieldInfo field in fields) {
				bool conv_int_to_double = field.GetDPManagedPropertyType (all) == "int" && field.GetDPPropertyType (all).Name == "double";
				
				if (field.IsDPAttached || !field.GenerateManagedAccessors)
					continue;
				
				text.AppendLine ();
				
				// property accessor
				text.Append ("\t\t");
				Helper.WriteAccess (text, field.GetManagedAccessorAccess ());
				text.Append (" ");
				text.Append (field.GetDPManagedPropertyType (all));
				text.Append (" ");
				text.Append (field.GetDependencyPropertyName ());
				text.AppendLine (" {");
				
				// property getter
				text.Append ("\t\t\t");
				if (field.GetManagedAccessorAccess () != field.GetManagedGetterAccess ()) {
					Helper.WriteAccess (text, field.GetManagedGetterAccess ());
					text.Append (" ");
				}
				
				text.Append ("get { return (");
				text.Append (field.GetDPManagedPropertyType (all));
				if (conv_int_to_double)
					text.Append (") (double");
				text.Append (") GetValue (");
				text.Append (field.Name);
				text.AppendLine ("); }");
				
				// property setter
				if (!field.IsDPReadOnly) {
					text.Append ("\t\t\t");
					if (field.GetManagedAccessorAccess () != field.GetManagedSetterAccess ()) {
						Helper.WriteAccess (text, field.GetManagedSetterAccess ());
						text.Append (" ");
					}
					text.Append ("set { SetValue (");
					text.Append (field.Name);
					text.AppendLine (", value); }");
				}
				text.AppendLine ("\t\t}");
			}
			
			text.AppendLine ("\t}");
		}
		text.AppendLine ("}");		
		
		Helper.WriteAllText (filename, text.ToString ());
	}

	class TypeEdgeCount {
		public TypeEdgeCount (TypeInfo type)
		{
			Type = type;
			Inbound = new List<TypeInfo>();
			Outbound = new List<TypeInfo>();
		}

		public TypeInfo Type {
			get; private set;
		}
		public List<TypeInfo> Inbound {
			get; private set;
		}
		public List<TypeInfo> Outbound {
			get; private set;
		}
	};

	static List<FieldInfo> TopoSortedProperties (GlobalInfo all, List<TypeInfo> types)
	{
		Dictionary<TypeInfo,TypeEdgeCount> typeHash = new Dictionary<TypeInfo,TypeEdgeCount>();

		List<TypeInfo> remainingTypes = new List<TypeInfo>();

		foreach (TypeInfo type in types) {
			typeHash.Add (type, new TypeEdgeCount (type));
			remainingTypes.Add (type);
		}

		// build up edges for our graph
		foreach (TypeInfo type in typeHash.Keys) {

			// every property defines an edge from the declaring type to the property type
			foreach (FieldInfo prop in type.Properties) {
				if (string.IsNullOrEmpty (prop.DPDefaultValue))
					continue;

				TypeInfo propType = prop.GetDPPropertyType (all);
				if (propType == type)
					continue;
				if (typeHash.ContainsKey (propType) && !typeHash[propType].Inbound.Contains (type)) {
					typeHash[propType].Inbound.Add (type);
					typeHash[type].Outbound.Add (propType);
				}
			}

			// every base class has an edge to subclass
			// (this is kind of a hack to deal with
			// property types which are listed as base
			// types when the default values are
			// subclasses.

			TypeInfo ourType = type;
			TypeReference baseRef = ourType.Base;
			while (baseRef != null && !string.IsNullOrEmpty (baseRef.Value)) {
				TypeInfo baseType = (TypeInfo) all.Children [baseRef.Value];
				if (baseType == null)
					break;
				if (typeHash.ContainsKey (baseType) && !typeHash[baseType].Outbound.Contains (ourType)) {
					typeHash[baseType].Outbound.Add (ourType);
					typeHash[ourType].Inbound.Add (baseType);
				}
				ourType = baseType;
				if (!typeHash.ContainsKey (ourType))
					break;
				baseRef = ourType.Base;
			}
		}

		List<TypeInfo> sorted = new List<TypeInfo>();
		List<TypeInfo> roots = new List<TypeInfo>();

		foreach (TypeEdgeCount tec in typeHash.Values) {
			if (tec.Inbound.Count == 0)
				roots.Add (tec.Type);
		}

		while (roots.Count > 0) {
			TypeInfo type = roots[0];
			roots.RemoveAt (0);

			sorted.Add (type);
			remainingTypes.Remove (type);

			foreach (TypeInfo targetType in typeHash[type].Outbound) {
				if (!typeHash.ContainsKey (targetType))
					continue;
				typeHash[targetType].Inbound.Remove (type);
				if (typeHash[targetType].Inbound.Count == 0) {
					roots.Add (targetType);
				}
			}
		}

		if (remainingTypes.Count > 0) {
			throw new Exception (string.Format ("cycle in the DO/DP graph ({0} types left)", remainingTypes.Count));
		}

		List<FieldInfo> fields = new List<FieldInfo>();
		foreach (TypeInfo type in sorted) {
			foreach (FieldInfo field in type.Properties)
				fields.Insert (0, field);
		}
		return fields;
	}

	static void GenerateDPs (GlobalInfo all)
	{	
		string base_dir = Environment.CurrentDirectory;
		string moon_dir = Path.Combine (base_dir, "src");
// 		int version_previous = 0;
		StringBuilder text = new StringBuilder ();
		List<FieldInfo> fields = all.DependencyProperties;
		List<string> headers = new List<string> ();

		List<TypeInfo> types = new List<TypeInfo> ();
		foreach (FieldInfo field in fields) {
			if (!types.Contains ((TypeInfo)field.Parent))
				types.Add ((TypeInfo)field.Parent);
		}
		fields = TopoSortedProperties (all, types);

		headers.Add ("dependencyproperty.h");
		headers.Add ("validators.h");
		headers.Add ("provider.h");
		headers.Add ("color.h");
		headers.Add ("managedtypeinfo.h");
		foreach (FieldInfo field in fields) {
			string h;
			if (string.IsNullOrEmpty (field.Header))
				continue;
			h = Path.GetFileName (field.Header);
			
			if (!headers.Contains (h))
				headers.Add (h);
		}
		
		Helper.WriteWarningGenerated (text);
		text.AppendLine ();
		text.AppendLine ("#include <config.h>");
		text.AppendLine ();
		headers.Sort ();
		foreach (string h in headers) {
			text.Append ("#include \"");
			text.Append (h);
			text.AppendLine ("\"");
		}
		text.AppendLine ();
		text.AppendLine ("void");
		text.AppendLine ("Types::RegisterNativeProperties ()");
		text.AppendLine ("{");
		
		for (int i = 0; i < fields.Count; i++) {
			FieldInfo field = fields [i];
			TypeInfo type = field.ParentType;
			TypeInfo propertyType = null;
			string default_value = field.DPDefaultValue;
			bool has_default_value = !string.IsNullOrEmpty (default_value);
			string autocreator = field.DPAutoCreator;
			bool is_nullable = field.IsDPNullable;
			bool is_attached = field.IsDPAttached;
			bool is_readonly = field.IsDPReadOnly;
			bool is_always_change = field.IsDPAlwaysChange;
			string validator = field.DPValidator;
			bool is_full = is_attached || is_readonly || is_always_change || validator != null || autocreator != null || is_nullable;

			propertyType = field.GetDPPropertyType (all);
			
			text.Append ("\t");
			
			if (propertyType == null) {
				text.Append ("// (no PropertyType was found for this DependencyProperty) ");
			} else {
				headers.Add (propertyType.Header);
			}

			text.Append ("DependencyProperty::Register");
			if (is_full)
				text.Append ("Full");
			
			text.Append (" (");
			text.Append ("this, ");
			text.Append ("Type::");
			text.Append (type.KindName);
			text.Append (", \"");
			
			text.Append (field.GetDependencyPropertyName ());
			text.Append ("\"");
			text.Append (", ");

			text.Append (field.IsCustom ? "true" : "false");
			text.Append (", ");

			if (is_full) {
				if (has_default_value) {
					if (default_value.StartsWith ("new "))
						text.Append ("Value::CreateUnrefPtr (");
					else
						text.Append ("new Value (");
					text.Append (default_value);
					text.Append (")");
				} else {
					text.Append ("NULL");
				}
			} else {
				if (has_default_value) {
					if (default_value.StartsWith ("new "))
						text.Append ("Value::CreateUnrefPtr (");
					else
						text.Append ("new Value (");
					text.Append (default_value);
					text.Append (")");
				}
			}

			if ((has_default_value || is_full))
				text.Append (", ");
			
			if (propertyType != null) {
				if (propertyType.IsEnum) {
					text.Append ("Type::INT32");
				} else {
					text.Append ("Type::");
					text.Append (propertyType.KindName);
				}
			} else if (!has_default_value) {
				text.Append ("Type::INVALID");
				Console.WriteLine ("{0} does not define its property type.", field.FullName);
			}
			
			if (is_full) {
				text.Append (", ");
				text.Append (is_attached ? "true" : "false");
				text.Append (", ");
				text.Append (is_readonly ? "true" : "false");
				text.Append (", ");
				text.Append (is_always_change ? "true" : "false");
				text.Append (", ");
				text.Append ("NULL");
				text.Append (", ");
				text.Append (validator != null ? ("Validators::" + validator) : "NULL");
				text.Append (", ");
				text.Append (autocreator != null
					     ? (autocreator.Contains("::") ? autocreator : "AutoCreators::" + autocreator)
					     : "NULL");
				text.Append (", ");
				text.Append (is_nullable ? "true" : "false");
			}

			text.AppendLine (");");
		}
		text.AppendLine ("}");
		text.AppendLine ();
			
		// Static initializers
		for (int i = 0; i < fields.Count; i++) {
			FieldInfo field = fields [i];
			text.Append ("const int ");
			text.Append (field.Parent.Name);
			text.Append ("::");
			text.Append (field.Name);
			text.Append (" = ");
			text.Append (i);
			text.AppendLine (";");
		}
		text.AppendLine ();
		
		// C++ Accessors
		for (int i = 0; i < fields.Count; i++) {
			FieldInfo field = fields [i];
			TypeInfo prop_type;
			string prop_type_str;
			string value_str;
			string prop_default = null;
			bool both = field.Annotations.ContainsKey ("GenerateAccessors");
			bool setter = both || field.Annotations.ContainsKey ("GenerateSetter");
			bool getter = both || field.Annotations.ContainsKey ("GenerateGetter");
			bool is_attached = field.IsDPAttached;
			bool nullable_setter = setter && field.IsDPNullable;
			bool doing_nullable_setter = false;
			
			if (!setter && !getter)
				continue;
			
			prop_type = field.GetDPPropertyType (all);
			
			switch (prop_type.Name) {
			case "char*": 
				prop_type_str = "const char *"; 
				value_str = "String";
				break;
			case "int":
			case "gint32":
				value_str = "Int32";
				prop_type_str = prop_type.Name;
				prop_default = "0";
				break;
			case "double":
				value_str = "Double";
				prop_type_str = prop_type.Name;
				prop_default = "0.0";
				break;
			case "bool":
				prop_type_str = prop_type.Name;
				value_str = "Bool";
				prop_default = "false";
				break;
			case "char":
				prop_type_str = "gunichar";
				value_str = "Char";
				prop_default = "0";
				break;
			case "object":
				prop_type_str = "Value *";
				prop_default = "NULL";
				value_str = null;
				break;
			default:
				prop_type_str = prop_type.Name; 
				value_str = prop_type.Name;
				break;
			}

			string GetterName = string.Format ("{0}::Get{1}", field.ParentType.Name, field.GetDependencyPropertyName());
			string SetterName = string.Format ("{0}::Set{1}", field.ParentType.Name, field.GetDependencyPropertyName());

			if (getter) {
				text.Append (prop_type_str);
				if (field.IsDPNullable || (prop_type.IsClass || prop_type.IsStruct))
					text.Append (" *");
				text.AppendLine ();
				text.Append (GetterName);
				if (is_attached)
					text.AppendLine (" (DependencyObject *obj)");
				else
					text.AppendLine (" ()");
				text.AppendLine ("{");


				if (value_str == null) {
					text.Append ("\treturn ");
				} else if (is_attached) {
					text.Append ("\tValue *value = (!obj) ? NULL : ");
				} else {
					text.Append ("\tValue *value = ");
				}

				text.AppendFormat ("{0}{1}GetValue ({2}::{3});\n",
						   is_attached ? "obj->" : "",
						   field.ParentType.NeedsQualifiedGetValue(all) ? "DependencyObject::" : "",
						   field.ParentType.Name, field.Name);

				if (is_attached) {
					text.AppendFormat ("\tif (!value) value = Deployment::GetCurrent ()->GetTypes ()->GetProperty ({0}::{1})->GetDefaultValue();\n",
							   field.ParentType.Name, field.Name);
				}

				if (value_str == null) {
					// Skip this
				} else if (field.IsDPNullable || (prop_type.IsClass || prop_type.IsStruct || prop_type.Name == "char*")) {
					text.Append ("\treturn value ? ");
					if (prop_type.IsEnum) {
						text.AppendFormat ("({0}) value->AsInt32() : ({0}) 0", prop_type.Name);
					} else {
						if (!field.IsDPNullable && (/*prop_type.IsStruct || */prop_default != null))
						    if (string.IsNullOrEmpty (prop_default))
							throw new NotImplementedException (
								string.Format ("Generation of DependencyProperties with struct values ({0}.{1})",
									       field.ParentType.Name, field.Name));

						text.AppendFormat ("value->As{0}{1} () : {2}",
								   field.IsDPNullable && !(prop_type.IsStruct || prop_type.IsClass) ? "Nullable" : "",
								   value_str,
								   !field.IsDPNullable && prop_default != null ? prop_default : "NULL");
					}
				} else {
					// Value cannot be null, so don't need to check for it
					text.Append ("\treturn ");
					if (prop_type.IsEnum) {
						text.AppendFormat ("({0}) value->AsInt32 ()", prop_type.Name);
					} else {
						text.AppendFormat ("value->As{0} ()", value_str);
					}
				}

				if (value_str != null)
					text.AppendLine (";");
				text.AppendLine ("}");
				text.AppendLine ();
			}
			
		 do_nullable_setter:
			if (setter) {		
				text.AppendLine ("void");
				text.Append (SetterName);
				text.Append (" (");
				if (is_attached)
					text.Append ("DependencyObject *obj, ");
				text.Append (prop_type_str);
				if (prop_type.Name != "char*")
					text.Append (' ');
				if (!nullable_setter && (prop_type.IsClass || prop_type.IsStruct))
					text.Append ('*');
				text.AppendLine ("value)");
				
				text.AppendLine ("{");
				if (is_attached)
					text.AppendLine ("\tif (!obj) return;");
				if (doing_nullable_setter) {
					text.AppendLine ("\tif (!value)");
					text.Append ("\t\t");
					text.AppendFormat ("{0}{1}SetValue ({2}::{3}, NULL);\n",
							   is_attached ? "obj->" : "",
							   field.ParentType.NeedsQualifiedGetValue(all) ? "DependencyObject::" : "",
							   field.ParentType.Name, field.Name);
					text.AppendLine ("\telse");
					text.Append ("\t\t");
					text.AppendFormat ("{0}{1}SetValue ({2}::{3}, Value (*value));\n",
							   is_attached ? "obj->" : "",
							   field.ParentType.NeedsQualifiedGetValue(all) ? "DependencyObject::" : "",
							   field.ParentType.Name, field.Name);
				} else {
					if (!nullable_setter && prop_type.IsStruct)
						text.AppendLine ("\tif (!value) return;");
					text.Append ("\t");
					text.AppendFormat ("{0}{1}SetValue ({2}::{3}, ",
							   is_attached ? "obj->" : "",
							   field.ParentType.NeedsQualifiedGetValue(all) ? "DependencyObject::" : "",
							   field.ParentType.Name, field.Name);

					if (prop_type.Name == "guint64" || prop_type.Name == "TimeSpan") {
						text.AppendFormat ("Value (value, Type::{0}));\n",
								   prop_type.KindName);
					}
					else if (prop_type.Name == "char") {
						text.AppendLine ("Value (value, Type::CHAR));");
					}
					else if ((value_str == null) || (!nullable_setter && prop_type.IsStruct)) {
						text.AppendLine ("Value (*value));");
					}
					else if (prop_type.IsClass) {
						text.AppendLine ("Value::CreateUnrefPtr (value));");
					}
					else {
						text.AppendLine ("Value (value));");
					}
				}
				text.AppendLine ("}");
				text.AppendLine ();
			}
			
			if (nullable_setter) {
				if (!prop_type.IsStruct)
					prop_type_str += " *";
				nullable_setter = false;
				doing_nullable_setter = true;
				goto do_nullable_setter;
			}
		}
		
		Helper.WriteAllText (Path.Combine (moon_dir, "dependencyproperty.g.cpp"), text.ToString ());
		
	}
	
	static GlobalInfo GetTypes2 ()
	{
		string srcdir = Path.Combine (Environment.CurrentDirectory, "src");
		string asfdir = Path.Combine (srcdir, "asf");
		string plugindir = Path.Combine (Environment.CurrentDirectory, "plugin");
		List<string> all_files = new List<string> ();

		all_files.AddRange (Directory.GetFiles (srcdir, "*.h"));
		all_files.AddRange (Directory.GetFiles (asfdir, "*.h"));
		all_files.AddRange (Directory.GetFiles (plugindir, "*.h"));

		RemoveExcludedSrcFiles (srcdir, all_files);

		Tokenizer tokenizer = new Tokenizer (all_files.ToArray ());
		GlobalInfo all = new GlobalInfo ();
		
		tokenizer.Advance (false);
		
		try {
			while (ParseMembers (all, tokenizer)) {
			}
		} catch (Exception ex) {
			throw new Exception (string.Format ("{0}({1}): {2}", tokenizer.CurrentFile, tokenizer.CurrentLine, ex.Message), ex);
		}
		
		// Add all the manual types
		TypeInfo t;
		TypeInfo IComparableInfo;
		TypeInfo IFormattableInfo;
		TypeInfo IConvertibleInfo;
		TypeInfo IEquatableBoolInfo;
		TypeInfo IComparableBoolInfo;
		TypeInfo IEquatableDoubleInfo;
		TypeInfo IComparableDoubleInfo;
		TypeInfo IEquatableFloatInfo;
		TypeInfo IComparableFloatInfo;
		TypeInfo IEquatableCharInfo;
		TypeInfo IComparableCharInfo;
		TypeInfo IEquatableIntInfo;
		TypeInfo IComparableIntInfo;
		TypeInfo IEquatableLongInfo;
		TypeInfo IComparableLongInfo;
		TypeInfo IEquatableStringInfo;
		TypeInfo IComparableStringInfo;
		TypeInfo IEquatableTimeSpanInfo;
		TypeInfo IComparableTimeSpanInfo;
		TypeInfo IEquatableUintInfo;
		TypeInfo IComparableUintInfo;
		TypeInfo IEquatableUlongInfo;
		TypeInfo IComparableUlongInfo;

		all.Children.Add (new TypeInfo ("object", "OBJECT", "INVALID", true, true));

		all.Children.Add (IComparableInfo = new TypeInfo ("IComparable", "ICOMPARABLE", "OBJECT", true, true, false, true));
		all.Children.Add (IFormattableInfo = new TypeInfo ("IFormattable", "IFORMATTABLE", "OBJECT", true, true, false, true));
		all.Children.Add (IConvertibleInfo = new TypeInfo ("IConvertible", "ICONVERTIBLE", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableBoolInfo = new TypeInfo ("IEquatable<bool>", "IEQUATABLE_BOOL", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableBoolInfo = new TypeInfo ("IComparable<bool>", "ICOMPARABLE_BOOL", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableDoubleInfo = new TypeInfo ("IEquatable<double>", "IEQUATABLE_DOUBLE", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableDoubleInfo = new TypeInfo ("IComparable<double>", "ICOMPARABLE_DOUBLE", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableFloatInfo = new TypeInfo ("IEquatable<float>", "IEQUATABLE_FLOAT", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableFloatInfo = new TypeInfo ("IComparable<float>", "ICOMPARABLE_FLOAT", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableCharInfo = new TypeInfo ("IEquatable<char>", "IEQUATABLE_CHAR", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableCharInfo = new TypeInfo ("IComparable<char>", "ICOMPARABLE_CHAR", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableIntInfo = new TypeInfo ("IEquatable<int>", "IEQUATABLE_INT", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableIntInfo = new TypeInfo ("IComparable<int>", "ICOMPARABLE_INT", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableLongInfo = new TypeInfo ("IEquatable<long>", "IEQUATABLE_LONG", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableLongInfo = new TypeInfo ("IComparable<long>", "ICOMPARABLE_LONG", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableStringInfo = new TypeInfo ("IEquatable<string>", "IEQUATABLE_STRING", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableStringInfo = new TypeInfo ("IComparable<string>", "ICOMPARABLE_STRING", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableTimeSpanInfo = new TypeInfo ("IEquatable<TimeSpan>", "IEQUATABLE_TIMESPAN", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableTimeSpanInfo = new TypeInfo ("IComparable<TimeSpan>", "ICOMPARABLE_TIMESPAN", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableUintInfo = new TypeInfo ("IEquatable<uint>", "IEQUATABLE_UINT", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableUintInfo = new TypeInfo ("IComparable<uint>", "ICOMPARABLE_UINT", "OBJECT", true, true, false, true));

		all.Children.Add (IEquatableUlongInfo = new TypeInfo ("IEquatable<ulong>", "IEQUATABLE_ULONG", "OBJECT", true, true, false, true));
		all.Children.Add (IComparableUlongInfo = new TypeInfo ("IComparable<ulong>", "ICOMPARABLE_ULONG", "OBJECT", true, true, false, true));

		all.Children.Add (t = new TypeInfo ("bool", "BOOL", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableBoolInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableBoolInfo);

		all.Children.Add (t = new TypeInfo ("float", "FLOAT", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableFloatInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableFloatInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("double", "DOUBLE", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableDoubleInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableDoubleInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("guint64", "UINT64", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableUlongInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableUlongInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("gint64", "INT64", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableLongInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableLongInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("guint32", "UINT32", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableUintInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableUintInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("gint32", "INT32", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableIntInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableIntInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("char*", "STRING", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableStringInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableStringInfo);
		t.Interfaces.Add (IFormattableInfo);

		all.Children.Add (t = new TypeInfo ("TimeSpan", "TIMESPAN", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableTimeSpanInfo);
		t.Interfaces.Add (IEquatableTimeSpanInfo);

		all.Children.Add (t = new TypeInfo ("char", "CHAR", "OBJECT", true, true, true, false));
		t.Interfaces.Add (IComparableInfo);
		t.Interfaces.Add (IComparableCharInfo);
		t.Interfaces.Add (IConvertibleInfo);
		t.Interfaces.Add (IEquatableCharInfo);

		all.Children.Add (new TypeInfo ("NPObj", "NPOBJ", "OBJECT", true, true, true, false));
		all.Children.Add (new TypeInfo ("Managed", "MANAGED", "OBJECT", true, 2, true));

		all.Children.Add (new TypeInfo ("System.Windows.Input.Cursor", "CURSOR", "OBJECT", true, true));
		all.Children.Add (new TypeInfo ("System.Windows.Markup.XmlLanguage", "XMLLANGUAGE", "OBJECT", true, true));

		// Set IncludeInKinds for all types which inherit from EventObject
		foreach (MemberInfo member in all.Children.Values) {
			TypeInfo type = member as TypeInfo;
			if (type == null)
				continue;
			if (type.Name == "EventObject")
				type.Annotations ["IncludeInKinds"] = null;
			
			TypeReference bR = type.Base;
			MemberInfo m;
			TypeInfo b;
			while (bR != null) {
				if (bR.Value == "EventObject") {
					member.Annotations ["IncludeInKinds"] = null;
				}

				if (!all.Children.TryGetValue (bR.Value, out m))
					break;
				
				b = m as TypeInfo;
				if (b != null)
					bR = b.Base;
				else
					bR = null;
			}
		}
		
		return all;
	}
	
	// Returns false if there are no more tokens (reached end of code)
	static bool ParseClassOrStruct (Annotations annotations, MemberInfo parent, Tokenizer tokenizer)
	{
		TypeInfo type = new TypeInfo ();
		
		type.Annotations = annotations;
		type.Header = tokenizer.CurrentFile;
		type.Parent = parent;
		
		type.IsPublic = tokenizer.Accept (Token2Type.Identifier, "public");

		if (tokenizer.Accept (Token2Type.Identifier, "class")) {
			type.IsClass = true;
		} else if (tokenizer.Accept (Token2Type.Identifier, "struct")) {
			type.IsStruct = true;
			type.IsValueType = true;
		} else if (tokenizer.Accept (Token2Type.Identifier, "union")) {
			type.IsStruct = true; // Not entirely correct, but a union can be parsed as a struct
			type.IsValueType = true;
		} else {
			throw new Exception (string.Format ("Expected 'class' or 'struct', not '{0}'", tokenizer.CurrentToken.value));
		}

                // permit our visibility tags
                tokenizer.Accept (Token2Type.Identifier, "MOON_API");
                tokenizer.Accept (Token2Type.Identifier, "MOON_LOCAL");

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
			if (!tokenizer.Accept (Token2Type.Identifier, "public") && type.IsClass)
				throw new Exception (string.Format ("The base class of {0} is not public.", type.Name));
			
			type.Base = ParseTypeReference (tokenizer);
			
			// accept multiple inheritance the easy way
			while (tokenizer.CurrentToken.value == ",") {
				tokenizer.Accept (Token2Type.Punctuation, ",");
				
				while (tokenizer.CurrentToken.value != "," &&
				       tokenizer.CurrentToken.value != "{")
					tokenizer.GetIdentifier ();
			}
			
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
		Annotations properties = new Annotations ();
		TypeInfo parent_type = parent as TypeInfo;
		string accessibility;
		TypeReference returntype;
		bool is_dtor;
		bool is_ctor;
		bool is_virtual;
		bool is_static;
		bool is_const;
		bool is_extern;
		string name;
		
		//Console.WriteLine ("ParseMembers ({0})", type.Name);
		
	 	do {
			returntype = null;
			is_dtor = is_ctor = is_virtual = is_static = false;
			is_extern = is_const = false;
			name = null;
			properties = new Annotations ();
			
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
					ParseEnum (properties, parent, tokenizer);
					continue;
				case "friend":
					while (!tokenizer.Accept (Token2Type.Punctuation, ";")) {
						tokenizer.Advance (true);
					}
					continue;
				case "struct":
				case "class":
				case "union":
					if (!ParseClassOrStruct (properties, parent, tokenizer))
						return false;
					continue;
				case "typedef":
					StringBuilder requisite = new StringBuilder ();
					requisite.Append (tokenizer.CurrentToken.value);
					requisite.Append (' ');
					tokenizer.Advance (true);
					while (!tokenizer.Accept (Token2Type.Punctuation, ";")) {
						requisite.Append (tokenizer.CurrentToken.value);
						requisite.Append (' ');
						if (tokenizer.CurrentToken.value == "{") {
							tokenizer.Advance (true);
							while (!tokenizer.Accept (Token2Type.Punctuation, "}")) {
								requisite.Append (tokenizer.CurrentToken.value);
								requisite.Append (' ');
								tokenizer.Advance (true);
							}
							requisite.Append (tokenizer.CurrentToken.value);
							requisite.Append (' ');
						}
						tokenizer.Advance (true);
					}
					requisite.Append (";");
					if (properties.ContainsKey ("CBindingRequisite"))
						cbinding_requisites.AppendLine (requisite.ToString ());
					
					continue;
				case "EVENTHANDLER":
					while (!tokenizer.Accept (Token2Type.Punctuation, ";"))
						tokenizer.Advance (true);
					continue;
				case "template":
					tokenizer.Advance (true);
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, "<");
					tokenizer.AcceptOrThrow (Token2Type.Identifier, "typename");
					tokenizer.GetIdentifier ();
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ">");
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

				if (tokenizer.Accept (Token2Type.Identifier, "volatile")) {
					continue;
				}

				if (tokenizer.Accept (Token2Type.Identifier, "G_GNUC_INTERNAL")) {
					continue;
				}

                                if (tokenizer.Accept (Token2Type.Identifier, "MOON_API")) {
                                        continue;
                                }

                                if (tokenizer.Accept (Token2Type.Identifier, "MOON_LOCAL")) {
                                        continue;
                                }

			    break;
			} while (true);

			if (is_extern && tokenizer.Accept (Token2Type.Literal, "C")) {
				tokenizer.SyncWithEndBrace ();
				continue;
			}
			
			if (tokenizer.Accept (Token2Type.Punctuation, "~")) {
				is_dtor = true;
				if (!is_virtual) {
					TypeInfo ti = parent as TypeInfo;
					if (ti != null && ti.Base != null)
						Console.WriteLine ("The class {0} has a non-virtual destructor, and it's base class is {2} ({1}).", parent.Name, parent.Header, ti != null && ti.Base != null ? ti.Base.Value : "<none>");
				}
			}
					
			if (is_dtor) {
				name = "~" + tokenizer.GetIdentifier ();
				returntype = new TypeReference ("void");
			} else {
				returntype = ParseTypeReference (tokenizer);
				
				if (tokenizer.CurrentToken.value == "<") {
					tokenizer.Advance (true);
					while (!tokenizer.Accept (Token2Type.Punctuation, ">"))
						tokenizer.Advance (true);
				}
				
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
				method.Header = tokenizer.CurrentFile;
				method.Parent = parent;
				method.Annotations = properties;
				method.Name = name;
				method.IsConstructor = is_ctor;
				method.IsDestructor = is_dtor;
				method.IsVirtual = is_virtual;
				method.IsStatic = is_static;
				method.IsPublic = accessibility == "public";
				method.IsPrivate = accessibility == "private";
				method.IsProtected = accessibility == "protected";
				method.ReturnType = returntype;
				
				//Console.WriteLine ("ParseMembers: found method '{0}' is_ctor: {1}", name, is_ctor);
				
				if (!tokenizer.Accept (Token2Type.Punctuation, ")")) {
					string param_value = null;
					do {
						ParameterInfo parameter = new ParameterInfo (method);
						
						while (tokenizer.CurrentToken.type == Token2Type.CommentProperty) {
							parameter.Annotations.Add (tokenizer.CurrentToken.value);
							tokenizer.Advance (true);
						}
						
						if (tokenizer.Accept (Token2Type.Punctuation, ".") && tokenizer.Accept (Token2Type.Punctuation, ".") && tokenizer.Accept (Token2Type.Punctuation, ".")) {
							// ... variable argument declaration
							parameter.ParameterType = new TypeReference ("...");
						} else {
							parameter.ParameterType = ParseTypeReference (tokenizer);
						}
						if (tokenizer.CurrentToken.value != "," && tokenizer.CurrentToken.value != ")") {
							parameter.Name = tokenizer.GetIdentifier ();
							if (tokenizer.Accept (Token2Type.Punctuation, "[")) {
								if (tokenizer.CurrentToken.type == Token2Type.Identifier)
									tokenizer.Advance (true);
								tokenizer.AcceptOrThrow (Token2Type.Punctuation, "]");
							}
							if (tokenizer.Accept (Token2Type.Punctuation, "=")) {
								param_value = string.Empty;
								if (tokenizer.Accept (Token2Type.Punctuation, "-"))
									param_value = "-";
								param_value += tokenizer.GetIdentifier ();
								if (tokenizer.Accept (Token2Type.Punctuation, ":")) {
									tokenizer.AcceptOrThrow (Token2Type.Punctuation, ":");
									param_value += "::" + tokenizer.GetIdentifier ();
								}
							}
						}
						method.Parameters.Add (parameter);
						//Console.WriteLine ("ParseMember: got parameter, type: '{0}' name: '{1}' value: '{2}'", parameter.ParameterType.Value, parameter.Name, param_value);
					} while (tokenizer.Accept (Token2Type.Punctuation, ","));
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ")");
				}
				
				parent.Children.Add (method);

				//Allow const member functions, ignore the const keyword
				tokenizer.Accept (Token2Type.Identifier, "const");
				
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
					method.IsAbstract = true;
				} else if (tokenizer.Accept (Token2Type.Identifier, "MOON_API") ||
					   tokenizer.Accept (Token2Type.Identifier, "MOON_LOCAL")) {
					tokenizer.AcceptOrThrow (Token2Type.Punctuation, ";");
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
					field.Annotations = properties;
					
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
						if (tokenizer.Accept (Token2Type.Punctuation, "=")) {
							tokenizer.Advance (true); /* this can be an arbitrary long expression, sync with the ';'?  */
						}
						break;
					} while (true);
					
					tokenizer.Accept (Token2Type.Punctuation, ";");
				}
			}
		} while (true);
	}
	
	static void ParseEnum (Annotations properties, MemberInfo parent, Tokenizer tokenizer)
	{
		FieldInfo field;
		StringBuilder value = new StringBuilder ();
		TypeInfo type = new TypeInfo ();
		
		type.Annotations = properties;
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
			field.Value = value.ToString ();
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

		if (tokenizer.Accept (Token2Type.Identifier, "unsigned"))
			result.Append ("unsigned");
		
		if (tokenizer.Accept (Token2Type.Identifier, "const"))
			tr.IsConst = true;
		
		result.Append (tokenizer.GetIdentifier ());
		
		if (tokenizer.Accept (Token2Type.Punctuation, ":")) {
			tokenizer.AcceptOrThrow (Token2Type.Punctuation, ":");
			result.Append ("::");
			result.Append (tokenizer.GetIdentifier ());
		}
		
		if (tokenizer.Accept (Token2Type.Identifier, "const"))
			tr.IsConst = true;
		
		while (tokenizer.Accept (Token2Type.Punctuation, "*"))
			result.Append ("*");
		
		if (tokenizer.Accept (Token2Type.Identifier, "const"))
			tr.IsConst = true;
		
		if (tokenizer.Accept (Token2Type.Punctuation, "&"))
			result.Append ("&");
		
		if (tokenizer.Accept (Token2Type.Identifier, "const"))
			tr.IsConst = true;
		
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
		if (v.Length > "COLLECTION".Length && !v.StartsWith ("COLLECTION"))
			v = v.Replace ("COLLECTION", "_COLLECTION");
		if (v.Length > "DICTIONARY".Length)
			v = v.Replace ("DICTIONARY", "_DICTIONARY");
		return v;
	}
	
	public void GenerateTypes_G (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		string class_dir = Path.Combine (base_dir, "class");
		string moon_moonlight_dir = Path.Combine (class_dir, "System.Windows");
		List<TypeInfo> types = new List<TypeInfo> (all.GetDependencyObjects ());
		
		StringBuilder text = new StringBuilder ();
		
		Helper.WriteWarningGenerated (text);
					
		text.AppendLine ("using Mono;");
		text.AppendLine ("using System;");
		text.AppendLine ("using System.Reflection;");
		text.AppendLine ("using System.Collections.Generic;");
		text.AppendLine ("");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tpartial class Types {");
		text.AppendLine ("\t\tprivate void CreateNativeTypes ()");
		text.AppendLine ("\t\t{");
		text.AppendLine ("\t\t\tType t;");
		text.AppendLine ("\t\t\ttry {");

		foreach (MemberInfo m in all.Children.Values) {
			TypeInfo t = m as TypeInfo;
			if (t == null)
				continue;
			if (types.Contains (t))
				continue;
			types.Add (t);
		}

		types.Sort (new Members.MembersSortedByManagedFullName <TypeInfo> ());
		
		for (int i = 0; i < types.Count; i++) {
			TypeInfo t = types [i];
			string type = t.ManagedName;
			
			if (String.IsNullOrEmpty (t.Namespace) || t.Namespace == "None" || t.Name.StartsWith ("MoonWindow"))
				continue;
			
			if (type == "PresentationFrameworkCollection`1")
				type = "PresentationFrameworkCollection<>";
				
			//Log.WriteLine ("Found Kind.{0} in {1} which result in type: {2}.{3}", kind, file, ns, type);
			
			text.Append ("\t\t\t\tt = typeof (");
			text.Append (t.Namespace);
			text.Append (".");
			text.Append (type);
			text.AppendLine ("); ");
			
			text.Append ("\t\t\t\ttypes.Add (t, new ManagedType (t, Kind.");
			text.Append (t.KindName);
			text.AppendLine ("));");
		}

		// now handle the primitive types
		output_native_type_delegate f = delegate (string t, string k) {
			text.Append ("\t\t\t\tt = typeof (");
			text.Append (t);
			text.AppendLine (");");
				
				
			text.Append ("\t\t\t\ttypes.Add (t, new ManagedType (t, Kind.");
			text.Append (k);
			text.AppendLine ("));");
		};

		f ("char", "UINT32");
		f ("object", "OBJECT");
		f ("bool", "BOOL");
		f ("double", "DOUBLE");
		f ("float", "FLOAT");
		f ("ulong", "UINT64");
		f ("long", "INT64");
		f ("uint", "UINT32");
		f ("int", "INT32");
		f ("string", "STRING");
		f ("TimeSpan", "TIMESPAN");

		// all the interfaces
		f ("IComparable", "ICOMPARABLE");
		f ("IFormattable", "IFORMATTABLE");
		f ("IConvertible", "ICONVERTIBLE");
		f ("IEquatable<bool>", "IEQUATABLE_BOOL");
		f ("IComparable<bool>", "ICOMPARABLE_BOOL");
		f ("IEquatable<double>", "IEQUATABLE_DOUBLE");
		f ("IComparable<double>", "ICOMPARABLE_DOUBLE");
		f ("IEquatable<float>", "IEQUATABLE_FLOAT");
		f ("IComparable<float>", "ICOMPARABLE_FLOAT");
		f ("IEquatable<char>", "IEQUATABLE_CHAR");
		f ("IComparable<char>", "ICOMPARABLE_CHAR");
		f ("IEquatable<int>", "IEQUATABLE_INT");
		f ("IComparable<int>", "ICOMPARABLE_INT");
		f ("IEquatable<long>", "IEQUATABLE_LONG");
		f ("IComparable<long>", "ICOMPARABLE_LONG");
		f ("IEquatable<string>", "IEQUATABLE_STRING");
		f ("IComparable<string>", "ICOMPARABLE_STRING");
		f ("IEquatable<TimeSpan>", "IEQUATABLE_TIMESPAN");
		f ("IComparable<TimeSpan>", "ICOMPARABLE_TIMESPAN");
		f ("IEquatable<uint>", "IEQUATABLE_UINT");
		f ("IComparable<uint>", "ICOMPARABLE_UINT");
		f ("IEquatable<ulong>", "IEQUATABLE_ULONG");
		f ("IComparable<ulong>", "ICOMPARABLE_ULONG");

		f ("System.Windows.Application", "APPLICATION");
		f ("System.Windows.Thickness", "THICKNESS");
		f ("System.Windows.CornerRadius", "CORNERRADIUS");
		f ("System.Windows.PropertyPath", "PROPERTYPATH");
		f ("System.Windows.Point", "POINT");
		f ("System.Windows.Rect", "RECT");
		f ("System.Windows.Size", "SIZE");
		f ("System.Windows.FontStretch", "FONTSTRETCH");
		f ("System.Windows.FontWeight", "FONTWEIGHT");
		f ("System.Windows.FontStyle", "FONTSTYLE");
		f ("System.Windows.Input.Cursor", "CURSOR");
		f ("System.Windows.Media.FontFamily", "FONTFAMILY");
		f ("System.Windows.Markup.XmlLanguage", "XMLLANGUAGE");

		text.AppendLine ("\t\t\t} catch (Exception ex) {");
		text.AppendLine ("\t\t\t\tConsole.WriteLine (\"There was an error while loading native types: \" + ex.ToString ());");
		text.AppendLine ("\t\t\t}");
		text.AppendLine ("\t\t}");
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
	 	Log.WriteLine ("typeandkidngen done");
		
		Helper.WriteAllText (Path.Combine (Path.Combine (moon_moonlight_dir, "Mono"), "Types.g.cs"), text.ToString ());
	}
	
	private static void GenerateCBindings (GlobalInfo info, string dir)
	{
		List<MethodInfo> methods;
		StringBuilder header = new StringBuilder ();
		StringBuilder impl = new StringBuilder ();
		List <string> headers = new List<string> ();
		List <string> classes = new List<string> ();
		List <string> structs = new List<string> ();
		
		string last_type = string.Empty;
		
		methods = info.CPPMethodsToBind;
		
		Helper.WriteWarningGenerated (header);;
		Helper.WriteWarningGenerated (impl);

		header.AppendLine ("#ifndef __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ("#define __MOONLIGHT_C_BINDING_H__");
		header.AppendLine ();
		header.AppendLine ("#include <glib.h>");
		header.AppendLine ("#include <cairo.h>");
		header.AppendLine ();
		header.AppendLine ("#include \"moonbuild.h\"");
		header.AppendLine ("#include \"enums.h\"");
		header.AppendLine ();
		foreach (MemberInfo member in info.Children.Values) {
			TypeInfo type = member as TypeInfo;
			if (type == null)
				continue;
			
			if (type.IsClass) {
				if (!classes.Contains (type.Name))
					classes.Add (type.Name);
			} else if (type.IsStruct) {
				if (!structs.Contains (type.Name))
					structs.Add (type.Name);
			}
		}
		
		foreach (MemberInfo method in methods) {
			string h;
			
			if (method.ParentType != null) {
				TypeInfo type = method.ParentType;
				if (type.IsClass) {
					if (!classes.Contains (type.Name))
						classes.Add (type.Name);
				} else if (type.IsStruct) {
					if (!structs.Contains (type.Name))
						structs.Add (type.Name);
				}
			}
			
			if (string.IsNullOrEmpty (method.Header))
				continue;
			if (!method.Header.StartsWith (dir))
				continue;
			
			h = Path.GetFileName (method.Header);
			
			if (!headers.Contains (h))
				headers.Add (h);
		}
		header.AppendLine (forward_decls.ToString ());
		classes.Sort ();
		structs.Sort ();
		foreach (string c in classes) {
			header.Append ("class ");
			header.Append (c);
			header.AppendLine (";");
		}
		header.AppendLine ();
		foreach (string s in structs) {
			header.Append ("struct ");
			header.Append (s);
			header.AppendLine (";");
		}
		header.AppendLine ();
		header.AppendLine (cbinding_requisites.ToString ());
		
		header.AppendLine ();
		header.AppendLine ("G_BEGIN_DECLS");
		header.AppendLine ();
		
		impl.AppendLine ("#include <config.h>");
		impl.AppendLine ();
		impl.AppendLine ("#include <stdio.h>");
		impl.AppendLine ("#include <stdlib.h>");
		impl.AppendLine ();
		impl.AppendLine ("#include \"cbinding.h\"");
		impl.AppendLine ();
		headers.Sort ();
		foreach (string h in headers) {
			impl.Append ("#include \"");
			impl.Append (h);
			impl.AppendLine ("\"");
		}
		
		foreach (MemberInfo member in methods) {
			MethodInfo method = (MethodInfo) member;			
			
			if (!method.Header.StartsWith (dir))
				continue;
			
			if (last_type != method.Parent.Name) {
				last_type = method.Parent.Name;
				foreach (StringBuilder text in new StringBuilder [] {header, impl}) {
					text.AppendLine ("/**");
					text.Append (" * ");
					text.AppendLine (last_type);
					text.AppendLine (" **/");
				}
			}
			
			WriteHeaderMethod (method.CMethod, method, header, info);
			header.AppendLine ();
			
			WriteImplMethod (method.CMethod, method, impl, info);
			impl.AppendLine ();
			impl.AppendLine ();
		}
		
		header.AppendLine ();
		header.AppendLine ("G_END_DECLS");
		header.AppendLine ();
		header.AppendLine ("#endif");
		
		Helper.WriteAllText (Path.Combine (dir, "cbinding.h"), header.ToString ());
		Helper.WriteAllText (Path.Combine (dir, "cbinding.cpp"), impl.ToString ());
	}
	
	public static void GenerateCBindings (GlobalInfo info)
	{
		string base_dir = Environment.CurrentDirectory;
		string plugin_dir = Path.Combine (base_dir, "plugin");
		string moon_dir = Path.Combine (base_dir, "src");
		
		GenerateCBindings (info, moon_dir);
		GenerateCBindings (info, plugin_dir);
	}
	
	static void WriteHeaderMethod (MethodInfo cmethod, MethodInfo cppmethod, StringBuilder text, GlobalInfo info)	
	{
		Log.WriteLine ("Writing header: {0}::{1} (Version: '{2}', GenerateManaged: {3})", 
		               cmethod.Parent.Name, cmethod.Name, 
		               cmethod.Annotations.GetValue ("Version"),
		               cmethod.Annotations.ContainsKey ("GenerateManaged"));
		
		if (cmethod.Annotations.ContainsKey ("GeneratePInvoke"))
			text.AppendLine ("/* @GeneratePInvoke */");
                text.Append ("MOON_API ");
		cmethod.ReturnType.Write (text, SignatureType.NativeC, info);
		if (!cmethod.ReturnType.IsPointer)
			text.Append (" ");
		text.Append (cmethod.Name);
		cmethod.Parameters.Write (text, SignatureType.NativeC, false);
		text.AppendLine (";");
	}
	
	static void WriteImplMethod (MethodInfo cmethod, MethodInfo cppmethod, StringBuilder text, GlobalInfo info)
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
		
		cmethod.ReturnType.Write (text, SignatureType.NativeC, info);
		text.AppendLine ();
		text.Append (cmethod.Name);
		cmethod.Parameters.Write (text, SignatureType.NativeC, false);
		text.AppendLine ("");
		text.AppendLine ("{");
		
		if (is_ctor) {
			text.Append ("\treturn new ");
			text.Append (cmethod.Parent.Name);
			cmethod.Parameters.Write (text, SignatureType.NativeC, true);
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
					text.Append ("\t\treturn false");
				} else if (cmethod.ReturnType.Value == "Point") {
					text.Append ("\t\treturn Point (0, 0)");
				} else {
					text.AppendLine ("\t\t// Need to find a proper way to get the default value for the specified type and return that if instance is NULL.");
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
			cmethod.Parameters.Write (text, SignatureType.NativeC, true);
			text.AppendLine (";");
		}
		
		text.AppendLine ("}");
	}
	
	static void GenerateTypeStaticCpp (GlobalInfo all)
	{
		string header;
		List<string> headers = new List<string> ();
		
		StringBuilder text = new StringBuilder ();
		
		Helper.WriteWarningGenerated (text);
					
		text.AppendLine ("#include <config.h>");
		text.AppendLine ();
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
			if (!headers.Contains (header))
				headers.Add (header);
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
		text.AppendLine ();
			
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {
			if (t.GetEventCount () == 0)
				continue;
				
				
			foreach (FieldInfo field in t.Events) {
				text.Append ("const int ");
				text.Append (t.Name);
				text.Append ("::");
				text.Append (field.EventName);
				text.Append ("Event = ");
				text.Append (t.GetEventId (field));
				text.AppendLine (";");
			}
		}
	
		// Create the arrays of event names for the classes which have events
		text.AppendLine ("");
		foreach (TypeInfo t in all.Children.SortedTypesByKind) {

			if (t.Events.Count > 0) {
				text.Append ("const char *");
				text.Append (t.KindName);
				text.Append ("_Events [] = { ");
				
				foreach (FieldInfo field in t.Events) {
					text.Append ("\"");
					text.Append (field.EventName);
					text.Append ("\", ");
				}

				text.AppendLine ("NULL };");
			}

			if (t.Interfaces.Count > 0) {
				text.Append ("const Type::Kind ");
				text.Append (t.KindName);
				text.Append ("_Interfaces[] = { ");

				for (int i = 0; i < t.Interfaces.Count; i ++) {
					text.Append ("Type::");
					text.Append (t.Interfaces[i].KindName);
					if (i < t.Interfaces.Count - 1)
						text.Append (", ");
				}

				text.AppendLine (" };");
			}
		}
	
		// Create the array of type data
		text.AppendLine ("");
		text.AppendLine ("void");
		text.AppendLine ("Types::RegisterNativeTypes ()");
		text.AppendLine ("{");
		text.AppendLine ("\tDeployment *deployment = Deployment::GetCurrent ();");
		text.AppendLine ("\ttypes [(int) Type::INVALID] = new Type (deployment, Type::INVALID, Type::INVALID, false, false, NULL, 0, 0, NULL, 0, NULL, false, NULL, NULL );");
		foreach (TypeInfo type in all.Children.SortedTypesByKind) {
			MemberInfo member;
			TypeInfo parent = null;
			string events = "NULL";
			string interfaces = "NULL";
				
			if (!type.Annotations.ContainsKey ("IncludeInKinds"))
				continue;
				
			if (type.Base != null && type.Base.Value != null && all.Children.TryGetValue (type.Base.Value, out member))
				parent = (TypeInfo) member;
				
			if (type.Events != null && type.Events.Count != 0)
				events = type.KindName + "_Events";

			if (type.Interfaces.Count != 0)
				interfaces = type.KindName + "_Interfaces";
	
			text.AppendLine (string.Format (@"	types [(int) {0}] = new Type (deployment, {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12});",
							"Type::" + type.KindName, 
							type.KindName == "OBJECT" ? "Type::INVALID" : ("Type::" + (parent != null ? parent.KindName : "OBJECT")),
							type.IsValueType ? "true" : "false",
							type.IsInterface ? "true" : "false",
							"\"" + type.Name + "\"", 
							type.GetEventCount (),
							type.GetTotalEventCount (),
							events,
							type.Interfaces.Count,
							interfaces,
							type.DefaultCtorVisible ? "true" : "false",
							(type.C_Constructor != null && type.GenerateCBindingCtor) ? string.Concat ("(create_inst_func *) ", type.C_Constructor) : "NULL", 
							type.ContentProperty != null ? string.Concat ("\"", type.ContentProperty, "\"") : "NULL"
							)
					 );
		}

		text.AppendLine ("\ttypes [(int) Type::LASTTYPE] = new Type (deployment, Type::LASTTYPE, Type::INVALID, false, false, NULL, 0, 0, NULL, 0, NULL, false, NULL, NULL);");
		
		text.AppendLine ("}");

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
		text.AppendLine ("#if NET_2_1");
		text.AppendLine ("\tinternal enum Kind {");
		text.AppendLine ("#else");
		text.AppendLine ("\tpublic enum Kind {");
		text.AppendLine ("#endif");
		text.AppendLine (values);
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
		string realfile = "class/System.Windows/Mono/Kind.cs".Replace ('/', Path.DirectorySeparatorChar);
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
						if (!type.Annotations.ContainsKey("IncludeInKinds") ||
						    type.Annotations.ContainsKey("SkipValue") ||
						    type.IsNested ||
						    type.IsStruct)
							continue;

						if (type.IsStruct) {
							forward_decls.Append ("struct ");
						} else {
							forward_decls.Append ("class ");
						}
						forward_decls.Append (type.Name);
						forward_decls.AppendLine (";");
					}
					forward_decls.AppendLine ();
					result.Append (forward_decls.ToString ());
				} else if (line.Contains ("/*DO_AS*/")) {
					foreach (TypeInfo type in all.Children.SortedTypesByKind) {
						if (!type.Annotations.ContainsKey("IncludeInKinds") ||
						    type.Annotations.ContainsKey("SkipValue") ||
						    type.IsNested ||
						    type.IsStruct)
							continue;
			
						//do_as.AppendLine (string.Format ("	{1,-30} As{0} () {{ checked_get_subclass (Type::{2}, {0}) }}", type.Name, type.Name + "*", type.KindName));
						
						result.Append ('\t');
						result.Append (type.Name);
						result.Append ("*");
						result.Append (' ', 40 - type.Name.Length);
						result.Append ("As");
						result.Append (type.Name);
						result.Append (" (Types *types = NULL) { checked_get_subclass (Type::");
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
	
#if false
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
#endif
	
	static void GeneratePInvokes (GlobalInfo all)
	{
		string base_dir = Environment.CurrentDirectory;
		List <MethodInfo> methods = new List<MethodInfo> ();
		StringBuilder text = new StringBuilder ();
		string NativeMethods_cs;
		
		NativeMethods_cs = File.ReadAllText (Path.Combine (base_dir, "class/System.Windows/Mono/NativeMethods.cs".Replace ('/', Path.DirectorySeparatorChar)));

		methods = all.CPPMethodsToBind;		

		foreach (MemberInfo info in all.Children.Values) {
			MethodInfo minfo = info as MethodInfo;
			if (minfo == null)
				continue;
			if (!minfo.Annotations.ContainsKey ("GeneratePInvoke"))
				continue;
			foreach (MethodInfo mi in methods) {
				if (mi.CMethod.Name == minfo.Name) {
					minfo = null;
					break;
				}
			}
			if (minfo == null)
				continue;
			//Console.WriteLine ("Added: {0} IsSrc: {1} IsPlugin: {2} Header: {3}", minfo.Name, minfo.IsSrcMember, minfo.IsPluginMember, minfo.Header);
			methods.Add (minfo);
		}
		
		Helper.WriteWarningGenerated (text);
		text.AppendLine ("using System;");
		text.AppendLine ("using System.Windows;");
		text.AppendLine ("using System.Runtime.InteropServices;");
		text.AppendLine ("");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tinternal static partial class NativeMethods");
		text.AppendLine ("\t{");
		text.AppendLine ("\t\t/* moonplugin methods */");
		text.AppendLine ("\t");
		foreach (MethodInfo method in methods) {
			if (!method.IsPluginMember || !method.Annotations.ContainsKey ("GeneratePInvoke"))
				continue;
			WritePInvokeMethod (NativeMethods_cs, method, text, "moonplugin");
			text.AppendLine ();
		}
		
		text.AppendLine ("\t");
		text.AppendLine ("\t\t/* libmoon methods */");
		text.AppendLine ("\t");
		foreach (MethodInfo method in methods) {
			if (!method.IsSrcMember || !method.Annotations.ContainsKey ("GeneratePInvoke"))
				continue;
			WritePInvokeMethod (NativeMethods_cs, method, text, "moon");
			text.AppendLine ();
		}
		text.AppendLine ("\t}");
		text.AppendLine ("}");
		
		Helper.WriteAllText (Path.Combine (base_dir, "class/System.Windows/Mono/GeneratedPInvokes.cs".Replace ('/', Path.DirectorySeparatorChar)), text.ToString ());
	}
	
	static void WritePInvokeMethod (string NativeMethods_cs, MethodInfo method, StringBuilder text, string library)
	{
		bool marshal_string_returntype = false;
		bool marshal_moonerror = false;
		bool generate_wrapper = false;
//		bool is_manually_defined;
		bool comment_out;
		bool is_void = false;
		bool contains_unknown_types;
		string name;
		string managed_name;
		string tabs;
		TypeReference returntype;
		MethodInfo cmethod = method.CMethod;
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
			if (parameter.Name == "error" && parameter.ParameterType.Value == "MoonError*") {
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
//		is_manually_defined = IsManuallyDefined (NativeMethods_cs, managed_name);
		contains_unknown_types = method.ContainsUnknownTypes;
		comment_out = contains_unknown_types;
		tabs = comment_out ? "\t\t// " : "\t\t";
				
//		if (is_manually_defined)
//			text.AppendLine ("\t\t// NOTE: There is a method in NativeMethod.cs with the same name.");

		if (contains_unknown_types)
			text.AppendLine ("\t\t// This method contains types the generator didn't know about. Fix the generator (find the method 'GetManagedType' in TypeReference.cs and add the missing case) and try again.");
			
		text.Append (tabs);
		text.Append ("[DllImport (\"");
		text.Append (library);
		if (generate_wrapper) {
			text.Append ("\", EntryPoint=\"");
			text.Append (name);
		}
		text.AppendLine ("\")]");

		if (method.ReturnType.Value == "bool") {
			text.Append (tabs);
			text.AppendLine ("[return: MarshalAs (UnmanagedType.U1)]");
		} else if (method.ReturnType.Value == "gboolean") {
			text.Append (tabs);
			text.AppendLine ("[return: MarshalAs (UnmanagedType.Bool)]");
		}
		
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
			returntype.Write (text, SignatureType.PInvoke, null);
		text.Append (" ");
		text.Append (name);
		if (generate_wrapper)
			text.Append ("_");
		cmethod.Parameters.Write (text, SignatureType.PInvoke, false);
		text.AppendLine (";");
		
		if (generate_wrapper) {
			text.Append (tabs);
			text.Append ("public static ");
			returntype.Write (text, SignatureType.Managed, null);
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
				returntype.Write (text, SignatureType.Managed, null);
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
				text.AppendLine ("\tif (result == IntPtr.Zero)");
				text.Append (tabs);
				text.AppendLine ("\t\treturn null;");
				text.Append (tabs);
				text.AppendLine ("\tstring s = Marshal.PtrToStringAnsi (result);\t// *copy* unmanaged string");
				text.Append (tabs);
				if (!method.ReturnType.IsConst) {
					text.AppendLine ("\tMarshal.FreeHGlobal (result);\t\t\t// g_free the unmanaged string");
					text.Append (tabs);
				}
				text.AppendLine ("\treturn s;");
			} else if (!is_void) {
				text.Append (tabs);
				text.AppendLine ("\treturn result;");
			}
		
			text.Append (tabs);
			text.Append ("}");
			text.AppendLine ();
		}
	}


	static void RemoveExcludedSrcFiles (string srcdir, List<string> files)
	{
		files.Remove (Path.Combine (srcdir, "cbinding.h"));
		files.Remove (Path.Combine (srcdir, "ptr.h"));
	}

}
