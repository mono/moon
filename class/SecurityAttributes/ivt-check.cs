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
		case "System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			// note: MoonAtkBridge is NOT shipped with Moonlight and IS considered platform code
			// but only if it's installed at a specific (plugin) place
			retval = (ivtname == "MoonAtkBridge, PublicKey=00240000048000009400000006020000002400005253413100040000110000004bb98b1af6c1df0df8c02c380e116b7a7f0c8c827aecfccddc6e29b7c754cd608b49dfcef4df9699ad182e50f66afa4e68dabc7b6aeeec0aa4719a5f8e0aae8c193080a706adc3443a8356b1f254142034995532ac176398e12a30f6a74a119a89ac47672c9ae24d7e90de686557166e3b873cd707884431a0451d9d6f7fe795");
			break;
		case "System.ServiceModel.Web, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			retval = (ivtname == "System.Json, PublicKey=0024000004800000940000000602000000240000525341310004000001000100b5fc90e7027f67871e773a8fde8938c81dd402ba65b9201d60593e96c492651e889cc13f1415ebb53fac1131ae0bd333c5ee6021672d9718ea31a8aebd0da0072f25d87dba6fc90ffd598ed4da35e44c398c454307e8e33b8426143daec9f596836f97c8f74750e5975c64e2189f45def46b2a2b1247adc3652bf5c308055da9");
			retval |= (ivtname == "System.ServiceModel.Web.Extensions, PublicKey=0024000004800000940000000602000000240000525341310004000001000100b5fc90e7027f67871e773a8fde8938c81dd402ba65b9201d60593e96c492651e889cc13f1415ebb53fac1131ae0bd333c5ee6021672d9718ea31a8aebd0da0072f25d87dba6fc90ffd598ed4da35e44c398c454307e8e33b8426143daec9f596836f97c8f74750e5975c64e2189f45def46b2a2b1247adc3652bf5c308055da9");
			break;
		case "System.Xml, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e":
			// note: right now our System.Xml is totally transparent (no SC nor SSC) and is not,
			// for moonlight, considered as platform code anymore (r146370)
			retval = (ivtname == "System.Xml.Serialization, PublicKey=0024000004800000940000000602000000240000525341310004000001000100b5fc90e7027f67871e773a8fde8938c81dd402ba65b9201d60593e96c492651e889cc13f1415ebb53fac1131ae0bd333c5ee6021672d9718ea31a8aebd0da0072f25d87dba6fc90ffd598ed4da35e44c398c454307e8e33b8426143daec9f596836f97c8f74750e5975c64e2189f45def46b2a2b1247adc3652bf5c308055da9");
			break;
		}

		if (retval)
			Console.WriteLine (" * special exception made for '{0}'", ivtname);
		return retval;
	}

	static bool Check (string filename)
	{
		bool retval = true;
		AssemblyDefinition assembly = AssemblyDefinition.ReadAssembly (filename);

		string aname = assembly.Name.FullName;
		Console.WriteLine (aname);

		foreach (CustomAttribute ca in assembly.CustomAttributes) {
			if (ca.AttributeType.FullName != InternalsVisibleTo)
				continue;

			string fqn = ca.ConstructorArguments [0].Value as string;
			string name = fqn.Substring (0, fqn.IndexOf (','));
			if (Array.IndexOf<string> (PlatformCode.Assemblies, name) >= 0)
				continue;

			if (Filter (aname, fqn))
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

