/*
 * MemberInfo.cs.
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

class MemberInfo {
	public MemberInfo Parent;
	public string Name;
	
	public bool IsPublic;
	public bool IsPrivate;
	public bool IsProtected;
	
	private string header; // The .h file where the member is defined
	private Members children;
	private Annotations annotations;
	private string fullname;
	private string managed_fullname;
	private Nullable<int> silverlight_version;
	private string managed_name;
	
	public GlobalInfo GlobalInfo {
		get {
			GlobalInfo result = Parent as GlobalInfo;
			
			if (result != null)
				return result;
			
			if (Parent == null)
				return this as GlobalInfo;
			
			return Parent.GlobalInfo;
		}
	}
	
	public string ManagedName {
		get {
			if (managed_name == null) {
				managed_name = Annotations.GetValue ("ManagedName");
				if (string.IsNullOrEmpty (managed_name))
					managed_name = Name;
			}
			return managed_name;
		}
	}
	
	public TypeInfo ParentType {
		get {
			return (TypeInfo) Parent;
		}
	}
	
	public void WriteVersionIf (StringBuilder text, bool end)
	{
		if (SilverlightVersion > 1) {
			if (!end) {
				text.AppendLine ("#if ");
				Helper.WriteVersion (text, SilverlightVersion);
			} else {
				text.AppendLine ("#endif");
			}
		}
	}
	
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
	
	public Annotations Annotations {
		get {
			if (annotations == null)
				annotations = new Annotations ();
			return annotations;
		}
		set {
			annotations = value;
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
				if (Parent != null && !string.IsNullOrEmpty (Parent.FullName)) {
					fullname = Parent.FullName + "." + Name;
				} else {
					fullname = Name;
				}
			}
			return fullname;
		}
	}
	
	public string ManagedFullName {
		get {
			if (managed_fullname == null) {
				if (Parent != null && !string.IsNullOrEmpty (Parent.ManagedFullName)) {
					managed_fullname = Parent.ManagedFullName + "." + Name;
				} else if (Namespace != null) {
					managed_fullname = Namespace + "." + FullName;
				} else {
					managed_fullname = FullName;
				}
			}
			return managed_fullname;				                            
		}
	}
	
	public string Namespace {
		get { return Annotations.GetValue ("Namespace"); }			
	}
	
	public int SilverlightVersion {
		get {
			string value = null;
			Annotation property;
			if (!silverlight_version.HasValue) {
				if (Annotations.TryGetValue ("Version", out property)) {
					value = property.Value;
				} else if (Annotations.TryGetValue ("SilverlightVersion", out property)) {
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
		if (annotations != null)
			annotations.Dump ();
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
	
	public class MembersSortedByName <T> : IComparer<T> where T : MemberInfo  {
		public int Compare (T a, T b)
		{
			return string.Compare (a.Name, b.Name);
		}
	}
	
	public class MembersSortedByFullName <T> : IComparer<T> where T : MemberInfo  {
		public int Compare (T a, T b)
		{
			return string.Compare (a.FullName, b.FullName);
		}
	}
	
	public class MembersSortedByManagedFullName <T>  : IComparer<T> where T : MemberInfo {
		public int Compare (T a, T b)
		{
			int result = string.Compare (a.Namespace, b.Namespace);
			if (result != 0)
				return result;
			return string.Compare (a.ManagedName, b.ManagedName);
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
			Console.WriteLine (string.Format ("Could not add the member: {0}.{1} in parent {3}: There already is a member with the same signature ({2}).", parent.Name, value.Name, signature, parent.GetType ().FullName));
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
				
				if (!type.Annotations.ContainsKey ("IncludeInKinds")) {
					continue;
				}
	
			 	kinds_for_enum.Append ("\t\t");
				kinds_for_enum.Append (type.KindName);
				kinds_for_enum.Append (",");
				if (type.Annotations.ContainsKey ("SilverlightVersion"))// && type.Annotations ["SilverlightVersion"].Value == "\"2\"")
					kinds_for_enum.Append ("// Silverlight 2.0 only");
				kinds_for_enum.AppendLine ();
			}
		}
		return kinds_for_enum;
	}
}
