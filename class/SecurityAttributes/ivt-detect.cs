// Copyright (C) 2010-2011 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using System;
using System.IO;
using System.Collections.Generic;

using Mono.Cecil;
using Mono.Cecil.Cil;

using Moonlight.SecurityModel;

class Program {

	const string FriendAccessAllowedAttribute = "System.Runtime.CompilerServices.FriendAccessAllowedAttribute";

	static List<string> internals = new List<string> ();
	static List<string> friends = new List<string> ();

	static bool HasFriendAccessAllowedAttribute (MemberReference member, bool declaringType)
	{
		if (member == null)
			return false;

		ICustomAttributeProvider cap = (member as ICustomAttributeProvider);
		if (cap.HasCustomAttributes) {
			if (cap.HasAttribute (FriendAccessAllowedAttribute))
				return true;
		}
		return declaringType ? HasFriendAccessAllowedAttribute (member.DeclaringType, declaringType) : false;
	}

	static void CheckMember (string assemblyName, char memberType, MemberReference member)
	{
		if (Array.IndexOf (PlatformCode.Assemblies, assemblyName) == -1)
			return;

		bool friend = HasFriendAccessAllowedAttribute (member, true);
		string s = String.Format ("[{0}] {2}{3}: {1}", assemblyName, member, memberType,
			friend ? "f" : String.Empty);

		if (friend) {
			if (!friends.Contains (s)) {
				friends.Add (s);
			}
		} else if (!internals.Contains (s)) {
			internals.Add (s);
		}
	}

	static void Check (AssemblyDefinition assembly, MethodDefinition method)
	{
		if (!method.HasBody)
			return;

		foreach (Instruction ins in method.Body.Instructions) {
			MethodReference mr = (ins.Operand as MethodReference);
			if (mr != null) {
				MethodDefinition md = mr.Resolve ();
				if (md == null)
					continue;
				AssemblyDefinition ad = md.DeclaringType.Resolve ().Module.Assembly;
				if ((ad != assembly) && !md.IsVisible ()) {
					string aname = ad.Name.Name;
					CheckMember (aname, 'M', md);
				}
				continue;
			}
			// Fields
			FieldReference fr = (ins.Operand as FieldReference);
			if (fr != null) {
				FieldDefinition fd = fr.Resolve ();
				if (fd == null)
					continue;
				AssemblyDefinition ad = fd.DeclaringType.Resolve ().Module.Assembly;
				if ((ad != assembly) && !fd.IsVisible ()) {
					string aname = ad.Name.Name;
					CheckMember (aname, 'F', fd);
				}
				continue;
			}
			// Type
			TypeReference tr = (ins.Operand as TypeReference);
			if (tr != null) {
				TypeDefinition td = tr.Resolve ();
				if (td == null)
					continue;
				AssemblyDefinition ad = td.Module.Assembly;
				if ((ad != assembly) && !td.IsVisible ()) {
					string aname = ad.Name.Name;
					CheckMember (aname, 'T', td);
				}
				continue;
			}
		}
	}

	static void Check (string filename)
	{
		AssemblyDefinition assembly = AssemblyDefinition.ReadAssembly (filename, new ReaderParameters { AssemblyResolver = resolver });

		foreach (ModuleDefinition module in assembly.Modules) {
			foreach (TypeDefinition type in module.GetAllTypes ()) {
				foreach (MethodDefinition md in type.Methods) {
					Check (assembly, md);
				}
			}
		}

		Console.WriteLine ("# Assembly: {0} [{1} items]", assembly, internals.Count + friends.Count);
		if (friends.Count > 0) {
			Console.WriteLine ("# decorated with [{0}]", FriendAccessAllowedAttribute);
			friends.Sort ();
			foreach (string s in friends)
				Console.WriteLine (s);
			friends.Clear ();
		}
		if (internals.Count > 0) {
			Console.WriteLine ("# non-decorated");
			internals.Sort ();
			foreach (string s in internals)
				Console.WriteLine (s);
			internals.Clear ();
		}
		Console.WriteLine ();
	}

	static string SdkClientPath = @"C:\Program Files\Microsoft SDKs\Silverlight\v4.0\Libraries\Client";
	static string RuntimePath = @"C:\Program Files\Microsoft Silverlight\4.0.60129.0";

	static DefaultAssemblyResolver resolver = new DefaultAssemblyResolver ();

	static int Main (string [] args)
	{
		int retval = 0;

		if (args.Length > 0)
			SdkClientPath = args [0];
		if (args.Length > 1)
			RuntimePath = args [1];

		resolver.AddSearchDirectory (RuntimePath);
		resolver.AddSearchDirectory (SdkClientPath);

		string [] files = Directory.GetFiles (SdkClientPath, "*.dll");
		Array.Sort<string> (files);
		foreach (string file in files) {
			Check (file);
		}
		return retval;
	}
}

