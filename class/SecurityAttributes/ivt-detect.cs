// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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

	static List<string> internals = new List<string> ();

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
					string s = String.Format ("[{0}] M: {1}", ad.Name.Name, md);
					if (!internals.Contains (s))
						internals.Add (s);
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
					string s = String.Format ("[{0}] F: {1}", ad.Name.Name, fd);
					if (!internals.Contains (s))
						internals.Add (s);
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
					string s = String.Format ("[{0}] T: {1}", ad.Name.Name, td);
					if (!internals.Contains (s))
						internals.Add (s);
				}
				continue;
			}
		}
	}

	static void Check (string filename)
	{
		var resolver = new DefaultAssemblyResolver ();
		resolver.AddSearchDirectory (RuntimePath);
		resolver.AddSearchDirectory (SdkClientPath);

		AssemblyDefinition assembly = AssemblyDefinition.ReadAssembly (filename, new ReaderParameters { AssemblyResolver = resolver });

		foreach (ModuleDefinition module in assembly.Modules) {
			foreach (TypeDefinition type in module.GetAllTypes ()) {
				foreach (MethodDefinition md in type.Methods) {
					Check (assembly, md);
				}
			}
		}

		Console.WriteLine ("# Assembly: {0} [{1} items]", assembly, internals.Count);
		internals.Sort ();
		foreach (string s in internals)
			Console.WriteLine (s);
		Console.WriteLine ();

		internals.Clear ();
	}

	static string SdkClientPath = @"C:\Program Files\Microsoft SDKs\Silverlight\v3.0\Libraries\Client";
	static string RuntimePath = @"C:\Program Files\Microsoft Silverlight\3.0.40818.0";

	static int Main (string [] args)
	{
		int retval = 0;

		if (args.Length > 0)
			SdkClientPath = args [0];
		if (args.Length > 1)
			RuntimePath = args [1];

		string [] files = Directory.GetFiles (SdkClientPath, "*.dll");
		Array.Sort<string> (files);
		foreach (string file in files) {
			Check (file);
		}
		return retval;
	}
}

