/*
 * ParameterInfo.cs.
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

class ParameterInfo : MemberInfo {
	public TypeReference ParameterType;
	
	public bool DisableWriteOnce;
	public string ManagedWrapperCode;// Used by GeneratePInvoke
	
	public void WriteSignature (StringBuilder text, SignatureType type)
	{
		if (type == SignatureType.PInvoke) {
			if (ParameterType.Value == "bool")
				text.Append ("[MarshalAs (UnmanagedType.U1)] ");
			if (Annotations.ContainsKey ("IsOut"))
				text.Append ("out ");
			if (Annotations.ContainsKey ("IsRef"))
				text.Append ("ref ");
		}

		if (type == SignatureType.PInvoke && Annotations.ContainsKey ("MarshalAs")) {
			text.Append (Annotations ["MarshalAs"].Value);
		} else {
			ParameterType.Write (text, type);
		}
		if (type != SignatureType.Native || !ParameterType.IsPointer)
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
		if (!ParameterType.IsPointer)
			text.Append (" ");
		text.Append (Name);
	}
}

class Parameters : List <ParameterInfo> {
	public void Write (StringBuilder text, SignatureType type, bool as_call)
	{
		bool first_done = false;
		text.Append (" (");
		if (Count > 0) {
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
		} else if (type == SignatureType.Native && !as_call) {
			text.Append ("void");
		}
		text.Append (")");
	}
}
