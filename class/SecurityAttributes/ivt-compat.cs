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
using System.Collections;
using System.Collections.Generic;
using System.IO;

using Mono.Cecil;
using Mono.Cecil.Cil;

using Moonlight.SecurityModel;

class Program {

	static Dictionary<string, AssemblyDefinition> assemblies = new Dictionary<string, AssemblyDefinition> ();
	// don't report the same item for different assemblies
	static List<string> errors = new List<string> ();

	static void ProcessField (AssemblyDefinition assembly, string value)
	{
		throw new NotImplementedException ("none found yet");
	}

	static MethodDefinition FindMethod (string name, TypeDefinition type)
	{
		foreach (MethodDefinition method in type.Methods) {
			if (method.ToString () == name)
				return method;
		}
		return null;
	}

	static bool IsInternal (MethodDefinition method)
	{
		if (method.IsAssembly)
			return true;
		// public/protected/internal method in a non-public type (or a nested internal type)
		TypeDefinition type = method.DeclaringType;
		return ((method.IsPublic || method.IsFamily || method.IsAssembly) && (type.IsNotPublic || type.IsNestedAssembly));
		// note: can't use IsVisible here because this check the type accessibility too
	}

	static void ProcessMethod (AssemblyDefinition assembly, string value)
	{
		int s = value.IndexOf (' ') + 1;
		int e = value.IndexOf ("::");
		string type_name = value.Substring (s, e - s);
		TypeDefinition type = assembly.MainModule.GetType (type_name);
		if (type == null) {
			string msg = String.Format ("MISSING [{0}] T: {1} -> M: {2}", assembly.Name.Name, type_name, value);
			if (!errors.Contains (msg))
				errors.Add (msg);
		} else {
			MethodDefinition method = FindMethod (value, type);
			if ((method == null) || !IsInternal (method)) {
				string msg = String.Format ("MISSING [{0}] M: {1}", assembly.Name.Name, value);
				if (!errors.Contains (msg))
					errors.Add (msg);
			}
		}
	}

	static void ProcessType (AssemblyDefinition assembly, string value)
	{
		TypeDefinition type = assembly.MainModule.GetType (value);
		if ((type == null) || (!type.IsNotPublic && !type.IsNestedAssembly)) {
			string msg = String.Format ("MISSING [{0}] T: {1}", assembly.Name.Name, value);
			if (!errors.Contains (msg))
				errors.Add (msg);
		}
	}

	// line format is:
	// [assembly] x yyy
	// where 'x' is M (method), F (field) or T (type)
	// and yyy is the method, field or type name
	static void ProcessEntry (string line)
	{
		int p = line.IndexOf (']');
		string assembly_name = line.Substring (1, p - 1);
		AssemblyDefinition assembly;
		if (!assemblies.TryGetValue (assembly_name, out assembly)) {
			string full_path = Path.Combine (PlatformCode, assembly_name + ".dll");
			assembly = AssemblyDefinition.ReadAssembly (full_path);
		}

		string value = line.Substring (p + 5);
		char type = line [p + 2];
		switch (type) {
		case 'F':
			ProcessField (assembly, value);
			break;
		case 'M':
			ProcessMethod (assembly, value);
			break;
		case 'T':
			ProcessType (assembly, value);
			break;
		default:
			throw new FormatException (String.Format ("Unknown type '{0}'.", type.ToString ()));
		}
	}

	static void ProcessFile (string filename)
	{
		using (StreamReader sr = new StreamReader (filename)) {
			while (!sr.EndOfStream) {
				string line = sr.ReadLine ();
				if ((line.Length > 0) && (line [0] == '['))
					ProcessEntry (line);
			}
		}
	}

	static string PlatformCode;

	static int Main (string [] args)
	{
		if (args.Length < 2) {
			Console.WriteLine ("Usage: mono ivt-compat.exe platform_dir internals_file");
			return 1;
		}

		PlatformCode = args [0];
		if (!Directory.Exists (PlatformCode)) {
			Console.WriteLine ("Bad platform directory '{0}'.", PlatformCode);
			return 1;
		}

		string filename = args [1];
		if (!File.Exists (filename)) {
			Console.WriteLine ("Bad required internals file '{0}'.", filename);
			return 1;
		}

		int error = 0;
		try {
			ProcessFile (filename);
			if (errors.Count == 0) {
				Console.WriteLine ("Nothing is missing, go in peace");
			} else {
				for (int i=0; i < errors.Count; i++)
					Console.WriteLine ("{0}: {1}", i + 1, errors [i]);
				Console.WriteLine ();
				Console.WriteLine ("{0} missing entries!", errors.Count);
			}
			return 0;
		}
		catch (Exception e) {
			Console.WriteLine (e);
			return 1;
		}
	}
}

