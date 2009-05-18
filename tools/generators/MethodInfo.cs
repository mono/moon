/*
 * MethodInfo.cs.
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

class MethodInfo : MemberInfo {
	public TypeReference ReturnType;
	public Parameters Parameters = new Parameters ();	
	public bool IsConstructor;
	public bool IsDestructor;
	public bool IsVirtual;
	public bool IsStatic;
	public bool IsAbstract;
	
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
			if (c_method == null) {
				c_method = new MethodInfo ();
				c_method.IsStatic = IsStatic;
				c_method.IsConstructor = IsConstructor;
				c_method.IsDestructor = IsDestructor;
				c_method.Name = Helper.CppToCName (Parent.Name, Name);
				c_method.Annotations = Annotations;
				c_method.ReturnType = ReturnType == null ? new TypeReference ("void") : ReturnType;
				c_method.Parent = Parent;
								
				if (!string.IsNullOrEmpty (Parent.Name) && !IsStatic && !IsConstructor) {
					ParameterInfo parameter = new ParameterInfo (c_method);
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
		if (!ReturnType.IsPointer)
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

	public override string ToString ()
	{
		StringBuilder st = new StringBuilder ();
		WriteFormatted (st);
		return st.ToString();
	}
}
