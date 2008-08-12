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
	private List<FieldInfo> dependency_properties;
	private List<MethodInfo> cppmethods_to_bind;
	
	public List<FieldInfo> DependencyProperties {
		get {
			if (dependency_properties == null) {
				// Check annotations against a list of known properties
				// to catch typos (DefaulValue, etc).
				Dictionary<string, string> known_properties = new Dictionary <string, string> ();
				
				known_properties.Add ("ReadOnly", null);
				known_properties.Add ("AlwaysChange", null);
				known_properties.Add ("Version", null);
				known_properties.Add ("PropertyType", null);
				known_properties.Add ("DefaultValue", null);
				known_properties.Add ("Access", null);
				known_properties.Add ("Nullable", null);
				known_properties.Add ("Attached", null);
				known_properties.Add ("ManagedDeclaringType", null);
				known_properties.Add ("ManagedPropertyType", null);
				
				dependency_properties = new List<FieldInfo>  ();
				foreach (MemberInfo member in Children.Values) {
					TypeInfo type = member as TypeInfo;
					
					if (type == null)
						continue;
					
					foreach (MemberInfo member2 in member.Children.Values) {
						FieldInfo field = member2 as FieldInfo;
						
						if (field == null)
							continue;
						
						if (field.FieldType == null || field.FieldType.Value != "DependencyProperty*")
							continue;
						
						if (!field.IsStatic)
							continue;
						
						if (!field.Name.EndsWith ("Property")) {
							Console.WriteLine ("GenerateDPs: Found the static field {0} which returns a DependencyProperty*, but the property name doesn't end with 'Property' (ignore this warning if the field isn't supposed to be a DP).", field.FullName);
							continue;
						}
						
						dependency_properties.Add (field);
						
						foreach (Property p in field.Properties.Values) {
							if (!known_properties.ContainsKey (p.Name))
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