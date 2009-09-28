/*
 * TypeInfo.cs.
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

class TypeInfo : MemberInfo {
	private string _KindName; // The name as it appears in the Kind enum (STRING, POINT_ARRAY, etc)
	private string c_constructor; // The C constructor
	private List<TypeInfo> interfaces;
	private List<FieldInfo> events;
	private List<FieldInfo> properties;
	private bool? is_abstract;
	
	public TypeReference Base; // The parent type
	public bool IsStruct;
	public bool IsClass;
	public bool IsEnum;

	public bool Include; // Force inclusion of this type into the type system (for manual types, char, point[], etc)
	public bool IsValueType;
	public bool IsInterface;
	public bool SkipValue;
	
	public bool IsAbstract {
		get {
			if (!is_abstract.HasValue) {
				foreach (MemberInfo member in Children.Values) {
					MethodInfo method = member as MethodInfo;
					
					if (method == null)
						continue;
					
					if (method.IsAbstract) {
						is_abstract = new bool? (true);
						break;
					}
				}
				
				if (!is_abstract.HasValue)
					is_abstract = new bool? (false);
			}
			return is_abstract.Value;
		}
	}

	public List<TypeInfo> Interfaces {
		get {
			if (interfaces == null) {
				interfaces = new List<TypeInfo> ();
			}
			return interfaces;
		}
	}

	public List<FieldInfo> Events {
		get {
			if (events == null) {
				events = new List<FieldInfo> ();

				foreach (MemberInfo member in Children.Values) {
					FieldInfo field = member as FieldInfo;
					if (field == null)
						continue;
				
					if (!field.Name.EndsWith ("Event"))
						continue;
					
					if (!field.IsStatic)
						continue;
						
					events.Add (field);
				}
				events.Sort (new Members.MembersSortedByFullName <FieldInfo>());
			}
			return events;
		}
	}

	public List<FieldInfo> Properties {
		get {
			if (properties == null) {
				properties = new List<FieldInfo> ();

				foreach (MemberInfo member in Children.Values) {
					FieldInfo property = member as FieldInfo;
					if (property == null)
						continue;
				
					if (!property.Name.EndsWith ("Property"))
						continue;
					
					if (!property.IsStatic)
						continue;
						
					properties.Add (property);
				}
				properties.Sort (new Members.MembersSortedByFullName <FieldInfo>());
			}
			return properties;
		}
	}
	
	public bool GenerateCBindingCtor {
		get {
			bool result = false;
			Annotation property;
			
			if (Annotations.TryGetValue ("GenerateCBindingCtor", out property))
				return property.Value != null;
			
			foreach (MemberInfo member in Children.Values) {
				MethodInfo method = member as MethodInfo;
				
				if (method == null)
					continue;
				
				if (method.Parameters.Count != 0)
					continue;
				
				if (!method.Annotations.ContainsKey ("GenerateCBinding"))
					continue;
				
				if (method.IsConstructor) {
					result = true;
					break;
				}
			}
			Annotations.Add (new Annotation ("GenerateCBindingCtor", result ? "true" : null));
			return result;
		}
	}

	public bool DefaultCtorVisible {
		get {
			bool result = IsValueType;
			Annotation property;

			if (Annotations.TryGetValue ("DefaultCtorVisible", out property))
				return property.Value != null;

			foreach (MemberInfo member in Children.Values) {
				MethodInfo method = member as MethodInfo;
				
				if (method == null)
					continue;
				
				if (!method.IsConstructor)
					continue;

				if (method.Parameters.Count != 0)
					continue;

				string access = "Public";
				if (method.Annotations.ContainsKey ("ManagedAccess"))
					access = method.Annotations.GetValue ("ManagedAccess");

				if (access == "Public") {
					result = true;
					break;
				}
			}
			Annotations.Add (new Annotation ("DefaultCtorVisible", result ? "true" : null));
			return result;
		}
	}

	public string C_Constructor {
		get {
			if (IsEnum || IsAbstract)
				return string.Empty;
			
			if (c_constructor == null)
				c_constructor = Helper.CppToCName (Name, Name);
			
			return c_constructor;
		}
	}
	
	public bool NeedsQualifiedGetValue (GlobalInfo all)
	{
		foreach (MemberInfo child in Children.Values) {
			if (child is FieldInfo && child.Name == "ValueProperty")
				return true;
		}
		if (Base != null)
			return ((TypeInfo)all.Children[Base.Value]).NeedsQualifiedGetValue (all);
		return false;
	}

	public bool IsNested {
		get {
			return Name.Contains ("::");
		}
	}
	
	public int GetEventId (FieldInfo Event)
	{
		for (int i = 0; i < Events.Count; i++) {
			FieldInfo field = Events [i];
			
			if (field == Event)
				return i + GetBaseEventCount ();
		}
		return ((TypeInfo) Parent.Children [Base.Value]).GetEventId (Event);
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
	
	
	/// <summary>
	/// Returns the number of events in this class, not counting base classes.
	/// </summary>
	/// <param name="version">
	/// A <see cref="System.Int32"/>
	/// </param>
	/// <returns>
	/// A <see cref="System.Int32"/>
	/// </returns>
	public int GetEventCount ()
	{
		return Events.Count;
	}
	
	public int GetTotalEventCount ()
	{
		return GetEventCount () + GetBaseEventCount ();
	}
	
	public string ContentProperty {
		get {
			if (Annotations.ContainsKey ("ContentProperty"))
				return Annotations ["ContentProperty"].Value;
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
			Annotations.Add (new Annotation ("IncludeInKinds"));
	}

	public TypeInfo (string Name, string KindName, string Base, bool Include, bool SkipValue)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = new TypeReference (Base);
		this.Include = Include;
		if (Include)
			Annotations.Add (new Annotation ("IncludeInKinds"));
		this.SkipValue = SkipValue;
		if (SkipValue)
			Annotations.Add (new Annotation ("SkipValue"));
	}

	public TypeInfo (string Name, string KindName, string Base, bool Include, bool SkipValue, bool is_value_type, bool is_interface) : this (Name, KindName, Base, Include, SkipValue)
	{
		this.IsValueType = is_value_type;
		this.IsInterface = is_interface;
	}

	public TypeInfo (string Name, string KindName, string Base, bool Include, int SLVersion)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = new TypeReference (Base);
		this.Include = Include;
		if (Include)
			Annotations.Add (new Annotation ("IncludeInKinds"));
		Annotations.Add (new Annotation ("SilverlightVersion", "\"" + SLVersion.ToString () + "\""));
	}

	public TypeInfo (string Name, string KindName, string Base, bool Include, int SLVersion, bool SkipValue)
	{
		this.Name = Name;
		this.KindName = KindName;
		this.Base = new TypeReference (Base);
		this.Include = Include;
		if (Include)
			Annotations.Add (new Annotation ("IncludeInKinds"));
		Annotations.Add (new Annotation ("SilverlightVersion", "\"" + SLVersion.ToString () + "\""));
		this.SkipValue = SkipValue;
		if (SkipValue)
			Annotations.Add (new Annotation ("SkipValue"));
	}
	
}
