/*
 * GlobalInfo.cs.
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

class GlobalInfo : MemberInfo {
	private List<FieldInfo> events;
	private List<FieldInfo> dependency_properties;
	private List<MethodInfo> cppmethods_to_bind;
	private List<MethodInfo> jsmethods_to_bind;
	private List<TypeInfo> dependency_objects;
	
	/// <value>
	/// A list of all the types that inherits from DependencyObject
	/// </value>
	public List<TypeInfo> GetDependencyObjects () {
		if (dependency_objects == null) {
			dependency_objects = new List<TypeInfo> ();
			
			foreach (MemberInfo member in Children.Values) {
				TypeInfo type = member as TypeInfo;
				TypeInfo current, parent;
				bool is_do = false;
				int limit = 20;
				
				if (type == null)
					continue;
				
				if (type.IsEnum || type.IsStruct)
					continue;
				
				current = type;
				
				while (limit-- > 0) {

					if (current.Base == null || string.IsNullOrEmpty (current.Base.Value))
						break;
					
					if (!Children.ContainsKey (current.Base.Value))
						continue;
					
					parent = Children [current.Base.Value] as TypeInfo;
					
					if (parent == null)
						break;
					
					if (parent.Name == "DependencyObject") {
						is_do = true;
						break;
					}
				
					current = parent;
				}
				
			//	if (limit <= 0)
			//		throw new Exception (string.Format ("Infinite loop while checking if '{0}' inherits from DependencyObject.", type.FullName));
				
				if (is_do)
					dependency_objects.Add (type);
			}
			
			dependency_objects.Sort (new Members.MembersSortedByManagedFullName <TypeInfo> ());
		}
		return dependency_objects;
	}

	public List<FieldInfo> Events {
		get {
			if (events == null) {
				// Check annotations against a list of known properties
				// to catch typos (DefaulValue, etc).
				Dictionary<string, string> known_annotations = new Dictionary <string, string> ();

				known_annotations.Add ("DelegateType", null);
				known_annotations.Add ("GenerateManagedEvent", null);
				known_annotations.Add ("GenerateManagedEventField", null);
				known_annotations.Add ("ManagedDeclaringType", null);
				known_annotations.Add ("ManagedAccessorAccess", null);

				events = new List<FieldInfo>  ();
				foreach (MemberInfo member in Children.Values) {
					TypeInfo type = member as TypeInfo;
					
					if (type == null)
						continue;
					
					foreach (MemberInfo member2 in member.Children.Values) {
						FieldInfo field = member2 as FieldInfo;
						
						if (field == null)
							continue; 
						
						if (!field.IsEvent)
							continue;
						
						events.Add (field);
						
						foreach (Annotation p in field.Annotations.Values) {
							if (!known_annotations.ContainsKey (p.Name))
								Console.WriteLine ("The field {0} in {3} has an unknown property: '{1}' = '{2}'", field.FullName, p.Name, p.Value, Path.GetFileName (field.Header));
						}
					}
				}
				events.Sort (new Members.MembersSortedByFullName <FieldInfo> ());
			}
			return events;
		}
	}
	
	public List<FieldInfo> DependencyProperties {
		get {
			if (dependency_properties == null) {
				// Check annotations against a list of known properties
				// to catch typos (DefaulValue, etc).
				Dictionary<string, string> known_annotations = new Dictionary <string, string> ();
				
				known_annotations.Add ("ReadOnly", null);
				known_annotations.Add ("AlwaysChange", null);
				known_annotations.Add ("Version", null);
				known_annotations.Add ("PropertyType", null);
				known_annotations.Add ("AutoCreateValue", null);
				known_annotations.Add ("DefaultValue", null);
				known_annotations.Add ("Access", null);
				known_annotations.Add ("ManagedAccess", null);
				known_annotations.Add ("Nullable", null);
				known_annotations.Add ("Attached", null);
				known_annotations.Add ("ManagedDeclaringType", null);
				known_annotations.Add ("ManagedPropertyType", null);
				known_annotations.Add ("ManagedFieldAccess", null);
				known_annotations.Add ("ManagedAccessorAccess", null);
				known_annotations.Add ("ManagedGetterAccess", null);
				known_annotations.Add ("ManagedSetterAccess", null);
				known_annotations.Add ("GenerateGetter", null);
				known_annotations.Add ("GenerateSetter", null);
				known_annotations.Add ("GenerateAccessors", null);
				known_annotations.Add ("GenerateManagedDP", null);
				known_annotations.Add ("GenerateManagedAccessors", null);
				known_annotations.Add ("Validator", null);
				known_annotations.Add ("AutoCreator", null);
				known_annotations.Add ("IsCustom", null);
				
				dependency_properties = new List<FieldInfo>  ();
				foreach (MemberInfo member in Children.Values) {
					TypeInfo type = member as TypeInfo;
					
					if (type == null)
						continue;
					
					foreach (MemberInfo member2 in member.Children.Values) {
						FieldInfo field = member2 as FieldInfo;
						
						if (field == null)
							continue; 
						
						if (field.FieldType == null || field.FieldType.Value != "int")
							continue;
						
						if (!field.IsStatic)
							continue;
						
						if (!field.Name.EndsWith ("Property"))
							continue;
						
						dependency_properties.Add (field);
						
						foreach (Annotation p in field.Annotations.Values) {
							if (!known_annotations.ContainsKey (p.Name))
								Console.WriteLine ("The field {0} in {3} has an unknown property: '{1}' = '{2}'", field.FullName, p.Name, p.Value, Path.GetFileName (field.Header));
						}
					}
				}
				dependency_properties.Sort (new Members.MembersSortedByFullName <FieldInfo> ());
			}
			return dependency_properties;
		}
	}
	
	
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
						if (!method.Annotations.ContainsKey ("GenerateCBinding"))
							continue;
						cppmethods_to_bind.Add (method);
					}
				}
				cppmethods_to_bind.Sort (new Members.MembersSortedByFullName <MethodInfo> ());
			}
			return cppmethods_to_bind;
		}
	}

	public List<MethodInfo> JSMethodsToBind {
		get {
			if (jsmethods_to_bind == null) {
				jsmethods_to_bind = new List<MethodInfo> ();
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
						if (!method.Annotations.ContainsKey ("GenerateJSBinding"))
							continue;

						jsmethods_to_bind.Add (method);
					}
				}
				jsmethods_to_bind.Sort (new Members.MembersSortedByFullName <MethodInfo> ());
			}
			return jsmethods_to_bind;
		}
	}

	public bool IsEnum (string type)
	{
		MemberInfo member;
		TypeInfo tp;
		
		type = type.Replace ("*", "");
		
		if (!Children.TryGetValue (type, out member)) {
			if (type.Contains ("::")) {
				string parent = type.Substring (0, type.IndexOf ("::"));
				string child = type.Substring (type.IndexOf ("::") + 2);
				
				if (!Children.TryGetValue (parent, out member))
					return false;
				         
				if (!member.Children.TryGetValue (child, out member))
					return false;
				
			} else {
				return false;
			}
		}
		
		tp = member as TypeInfo;
		
		if (tp == null)
			return false;
		
		return tp.IsEnum;
	}
}
