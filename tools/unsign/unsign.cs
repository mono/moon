using Mono.Cecil;
using System;
using System.IO;

class unsign
{
	static void Main (string [] args)
	{
		string source = args [0];
		string destination = Path.GetFileName (source) + ".Unsigned";

		AssemblyDefinition a;

		a = AssemblyFactory.GetAssembly (source);
		Console.WriteLine ("Assembly {0} successfully loaded from: {1}", a, source);		

		a.Name.HasPublicKey = false;
		a.Name.PublicKey = null;
		a.Name.PublicKeyToken = null;		

		AssemblyFactory.SaveAssembly (a, destination);
		Console.WriteLine ("Assembly {0} successfully saved to: {1}", a, destination);
	}
}
