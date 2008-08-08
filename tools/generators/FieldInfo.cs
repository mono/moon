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


class FieldInfo : MemberInfo {
	public TypeReference FieldType;
	public string BitField;
	public bool IsConst;
	public bool IsStatic;
	public bool IsExtern;
}