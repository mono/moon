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
using System.Text;

class GlobalInfo : MemberInfo {
	private List<MethodInfo> cppmethods_to_bind;
	
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