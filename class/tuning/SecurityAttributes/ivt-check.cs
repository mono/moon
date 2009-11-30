// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using Mono.Cecil;
using Moonlight.SecurityModel;

class Program {
	const string InternalsVisibleTo = "System.Runtime.CompilerServices.InternalsVisibleToAttribute";

	// like anything else there's always some exceptions
	static bool Filter (string aname, string ivtname)
	{
		bool retval = false;

		switch (aname) {
		case "mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			// note: Mono.CompilerServices.SymbolWriter.dll is shipped with Moonlight and is NOT
			// platform code so this entry will be refused by the plugin (coreclr) but will work 
			// normally on the desktop, e.g. for SMCS
			// note: MoonAtkBridge is NOT shipped with Moonlight and IS considered platform code
			// but only if it's installed at a specific (plugin) place
			retval = ((ivtname == "Mono.CompilerServices.SymbolWriter") || (ivtname == "MoonAtkBridge"));
			break;
		case "System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			// note: MoonAtkBridge is NOT shipped with Moonlight and IS considered platform code
			// but only if it's installed at a specific (plugin) place
			retval = (ivtname == "MoonAtkBridge");
			break;
		case "System.Xml, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			// note: right now our System.Xml is totally transparent (no SC nor SSC) and is not,
			// for moonlight, considered as platform code anymore (r146370)
			retval = (ivtname == "System.Xml.Serialization");
			break;
		}

		if (retval)
			Console.WriteLine (" * special exception made for '{0}'", ivtname);
		return retval;
	}

	static bool Check (string filename)
	{
		bool retval = true;
		AssemblyDefinition assembly = AssemblyFactory.GetAssembly (filename);

		string aname = assembly.Name.FullName;
		Console.WriteLine (aname);

		foreach (CustomAttribute ca in assembly.CustomAttributes) {
			if (ca.Constructor.DeclaringType.FullName != InternalsVisibleTo)
				continue;

			string fqn = ca.ConstructorParameters [0] as string;
			string name = fqn.Substring (0, fqn.IndexOf (','));
			if (Array.IndexOf<string> (PlatformCode.Assemblies, name) >= 0)
				continue;

			if (Filter (aname, name))
				continue;

			Console.WriteLine (" * unexpected: {0}{1}", fqn, Environment.NewLine);
			retval = false;
		}

		Console.WriteLine (" {0}{1}", retval ? "OK" : "FAIL!", Environment.NewLine);
		return retval;
	}

	static int Main (string [] args)
	{
		int retval = 0;
		foreach (string aname in args) {
			if (!Check (aname))
				retval = 1;
		}
		return retval;
	}
}

