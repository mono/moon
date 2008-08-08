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