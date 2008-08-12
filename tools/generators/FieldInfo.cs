/*
 * FieldInfo.cs.
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


class FieldInfo : MemberInfo {
	public TypeReference FieldType;
	public string BitField;
	public bool IsConst;
	public bool IsStatic;
	public bool IsExtern;
	
	public string EventName {
		get {
			if (!Name.EndsWith ("Event"))
				throw new System.Exception (string.Format ("The field '{0}' doesn't represent an event", FullName));
			return Name.Substring (0, Name.LastIndexOf ("Event"));
		}
	}
	
	public bool IsDPReadOnly {
		get { return Annotations.ContainsKey ("ReadOnly"); }
	}
	
	public bool IsDPAlwaysChange {
		get { return Annotations.ContainsKey ("AlwaysChange"); }
	}
	
	public bool IsDPAttached  {
		get { return Annotations.ContainsKey ("Attached"); }
	}
	
	public bool IsDPNullable {
		get { return Annotations.ContainsKey ("Nullable"); }
	}
	
	public string DPPropertyType {
		get { 
			string result = Annotations.GetValue ("PropertyType");
			
			if (result != null) {
				switch (result) {
				case "string":
					return "char*";
				case "FontStretch":	
					return "FontStretches";
				case "FontWeight":
					return "FontWeights";
				case "FontStyle":
					return "FontStyles";
				}
			}
			
			return result;
		}
	}
		
	public string DPDefaultValue {
		get { return Annotations.GetValue ("DefaultValue"); }
	}
	
	public TypeInfo GetDPPropertyType (GlobalInfo all)
	{
		string property_type = DPPropertyType;
		TypeInfo propertyType = null;
		
		if (!string.IsNullOrEmpty (property_type)) {
			if (all.Children.ContainsKey (property_type)) {
				propertyType = (TypeInfo) all.Children [property_type];
			} else {
				Console.WriteLine ("{0}'s PropertyType '{1}' was not recognized. Do not use the Kind value, but the real type name.", FullName, property_type);
			}
		} else {
			Console.WriteLine ("{0} does not have a PropertyType defined.", FullName);
		}
		
		return propertyType;
	}
	
	public string GetDPManagedPropertyType (GlobalInfo all) 
	{
		string property_type = Annotations.GetValue ("ManagedPropertyType");
		
		if (property_type != null)
			return property_type;
		
		property_type = Annotations.GetValue ("PropertyType");
		
		if (property_type == null)
			return null;
		
		switch (property_type) {
		case "char*":
			return "string";
		case "gint32":
			return "int";
		case "Managed":
			return "object";
		default:
			if (IsDPNullable)
				return "Nullable<" + property_type + ">";
			else
				return property_type;
		}
	}
}