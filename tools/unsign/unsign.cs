using Mono.Cecil;
using System;
using System.Diagnostics;
using System.IO;

class unsign
{
	static void Main (string [] args)
	{
		string source = args [0];
		string destination = Path.GetFileName (source) + ".Unsigned";
		string resign = null;
		
		/*
		 * unsign assembly.dll [--resign keyfile.snk]
		 */
		
		AssemblyDefinition a;

		if (args.Length > 1 && args [1] == "--resign")
			resign = args [2]; // key file
		
		a = AssemblyFactory.GetAssembly (source);
		Console.WriteLine ("Assembly {0} successfully loaded from: {1}", a, source);		

		if (resign == null)
			a.Name.HasPublicKey = false;
		
		if (resign == null) {
			a.Name.PublicKey = null;
			a.Name.PublicKeyToken = null;		
		} else {
			string pubkey;
			string token;
			string output;
			
			using (Process sn = new Process ()) {
				sn.StartInfo.FileName = "sn";
				sn.StartInfo.Arguments = "-q -tp " + resign;
				sn.StartInfo.UseShellExecute = false;
				sn.StartInfo.RedirectStandardOutput = true;
				sn.Start ();
				output = sn.StandardOutput.ReadToEnd ();
				
				output = output.Replace ("Public Key:", "");
				output = output.Replace ("Public Key Token:", "|");
				output = output.Replace ("\n", "");
				output = output.Replace ("\r", "");
				output = output.Replace (" ", "");
				
				string [] split = output.Split ('|');
				pubkey = split [0];
				token = split [1];
				
				a.Name.PublicKey = new byte [pubkey.Length / 2];
				for (int i = 0; i < a.Name.PublicKey.Length; i++)
					a.Name.PublicKey [i] = Byte.Parse (pubkey.Substring (i * 2, 2),  System.Globalization.NumberStyles.HexNumber, null);
				a.Name.PublicKeyToken = new byte [token.Length / 2];
				for (int i = 0; i < a.Name.PublicKeyToken.Length; i++)
					a.Name.PublicKeyToken [i] = Byte.Parse (token.Substring (i * 2, 2),  System.Globalization.NumberStyles.HexNumber, null);

				sn.StartInfo.Arguments = "-R " + resign;
				sn.Start ();
				sn.StandardOutput.ReadToEnd ();
			}
		}

		AssemblyFactory.SaveAssembly (a, destination);
		
		if (resign != null) {
			using (Process sn = new Process ())  {
				sn.StartInfo.FileName = "sn";
				sn.StartInfo.Arguments = "-q -R " + destination + " " + resign;
				sn.Start ();
				sn.WaitForExit ();
			}
		}
		
		Console.WriteLine ("Assembly {0} successfully saved to: {1}", a, destination);
		
		
	}
}
