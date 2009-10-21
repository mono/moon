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
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;

using Mono.Cecil;
using Mono.Cecil.Cil;
using Moonlight.SecurityModel;

class Program {

	class RevisionComparer : IComparer<int> {

		// more recent revision on top
		public int Compare (int x, int y)
		{
			return y - x;
		}
	}

	public class AuditData {
		public string CurrentHash;
		public SortedDictionary<int,string> Comments;
		public bool Exists;

		public AuditData ()
		{
			CurrentHash = String.Empty;
			Comments = new SortedDictionary<int, string> (new RevisionComparer ());
			Exists = false;
		}
	}

	static SortedDictionary<string, AuditData> Data = new SortedDictionary<string, AuditData> (StringComparer.InvariantCulture);

	static void ReadFile (string filename)
	{
		if (!File.Exists (filename)) {
			Console.WriteLine ("File '{0}' does not exists, starting from scratch!", filename);
			return;
		}

		int line_no = 0;
		AuditData data = null;
		using (StreamReader sr = new StreamReader (filename)) {
			while (!sr.EndOfStream) {
				string line = sr.ReadLine ();
				line_no++;

				if (line.Length == 0)
					continue;

				if (Char.IsWhiteSpace (line [0])) {
					int s = line.IndexOf ("\tr") + 2;
					int e = line.IndexOf ('\t', s);
					int revision = -1;
					if (Int32.TryParse (line.Substring (s, e - s), out revision)) {
						string comments = line.Substring (e + 2).TrimStart ();
						data.Comments.Add (revision, comments);
					}
				} else {
					if (line [0] == '!') {
						data = new AuditData ();
						data.CurrentHash = line.Trim ();
					} else {
						Data.Add (line.Trim (), data);
					}
				}
			}
		}
	}

	static void Add (MethodDefinition method, string hash, int revision)
	{
		AuditData data;
		string name = method.GetFullName ();

		if (Data.TryGetValue (name, out data)) {
			if (hash != data.CurrentHash) {
				// code has changed, a new review is required
				data.CurrentHash = hash;
				string message = "unaudited - modified";
				if (method.IsVisible ())
					message += " - VISIBLE";
				if (method.IsInternalCall)
					message += " - ICALL";
				try {
					data.Comments.Add (revision, message);
				}
				catch (ArgumentException) {
					Console.WriteLine ("Comments for revision #{0} already exist.", revision);
					Environment.Exit (1);
				}
			}
			data.Exists = true;
		} else {
			// new code - review is needed
			data = new AuditData ();
			data.CurrentHash = hash;
			string message = "unaudited - new";
			if (method.IsVisible ())
				message += " - VISIBLE";
			if (method.IsInternalCall)
				message += " - ICALL";
			data.Comments.Add (revision, message);
			Data.Add (name, data);
			data.Exists = true;
		}
	}

	static MD5 hash = MD5.Create ();

	static int GetOffset (Instruction ins)
	{
		if (ins == null)
			return -1;
		return ins.Offset;
	}

	// a bit resource heavy but it's not really time critical
	static string GetHash (MethodDefinition method)
	{
		byte [] digest = null;
		using (MemoryStream ms = new MemoryStream ()) {
			using (StreamWriter sw = new StreamWriter (ms)) {
				// note: we could hash more (e.g. attributes) but that's unlikely to require a new review
				sw.WriteLine (method.GetFullName ());
				sw.WriteLine (method.Attributes);
				sw.WriteLine (method.ImplAttributes);
				// an icall (or pinvoke) can be audited but won't have any IL
				if (method.HasBody) {
					var reader = method.DeclaringType.Module.Image.GetReaderAtVirtualAddress (method.RVA);
					sw.WriteLine (reader.ReadBytes (method.Body.CodeSize));
					if (method.Body.HasExceptionHandlers) {
						List<string> handlers = new List<string> ();
						foreach (ExceptionHandler eh in method.Body.ExceptionHandlers) {
							handlers.Add (String.Format ("{0}#{1}#{2}#{3}#{4}#{5}#{6}#{7}", eh.Type, eh.CatchType,
								GetOffset (eh.TryStart), GetOffset (eh.TryEnd), 
								GetOffset (eh.HandlerStart), GetOffset (eh.HandlerEnd),
								GetOffset (eh.FilterStart), GetOffset (eh.FilterEnd)));
						}
						// we must preserve order else the hash will be different for the same handlers
						if (handlers.Count > 1)
							handlers.Sort (StringComparer.InvariantCulture);
						foreach (string s in handlers)
							sw.WriteLine (s);
					}
				}
				sw.Flush ();
				ms.Position = 0;
				hash.Initialize ();
				digest = hash.ComputeHash (ms);
			}
		}
		StringBuilder sb = new StringBuilder ("!SSC-", 37);
		foreach (byte b in digest) {
			sb.Append (b.ToString ("X2"));
		}
		return sb.ToString ();
	}

	static bool NeedReview (MethodDefinition method)
	{
		// visible icall (mostly public) are considered safe (even without the attribute) but needs to be reviewed anyway
		if (method.IsInternalCall)
			return (!method.IsSecurityCritical () && method.IsVisible ());
		// is it SSC ?
		return method.IsSecuritySafeCritical ();
	}

	static void ProcessMethod (MethodDefinition method, int revision)
	{
		if (!NeedReview (method)) {
			// we check the type too, since we want to review all methods of a SSC type
			TypeDefinition type = (method.DeclaringType as TypeDefinition);
			if (!type.IsSecuritySafeCritical ())
				return;
		}

		try {
			Add (method, GetHash (method), revision);
		}
		catch (Exception e) {
			Console.WriteLine ("Error on method '{0}': {1}", method, e);
			Environment.Exit (1);
		}
	}

	static void ProcessAssembly (string assemblyName, int revision)
	{
		AssemblyDefinition ad = AssemblyFactory.GetAssembly (assemblyName);
		foreach (ModuleDefinition module in ad.Modules) {
			foreach (TypeDefinition type in module.Types) {
				foreach (MethodDefinition ctor in type.Constructors) {
					ProcessMethod (ctor, revision);
				}
				foreach (MethodDefinition method in type.Methods) {
					ProcessMethod (method, revision);
				}
			}
		}
	}

	static void SaveFile (string filename)
	{
		using (StreamWriter sw = new StreamWriter (filename)) {
			foreach (KeyValuePair<string, AuditData> kvp in Data) {
				// write only if Exists == true (i.e. stuff gets removed)
				if (!kvp.Value.Exists)
					continue;

				sw.WriteLine (kvp.Value.CurrentHash);
				sw.WriteLine (kvp.Key);
				foreach (KeyValuePair<int, string> comments in kvp.Value.Comments) {
					sw.WriteLine ("\tr{0}\t\t{1}", comments.Key, comments.Value);
				}
				sw.WriteLine ();
			}
		}
		Data.Clear ();
	}

	static int Main (string [] args)
	{
		if (args.Length < 3) {
			Console.WriteLine ("Usage: mono audit.exe platform_directory data_directory svn_revision");
			return 1;
		}

		string platorm_dir = args [0];
		if (!Directory.Exists (platorm_dir)) {
			Console.WriteLine ("Platform directory '{0}' not found.", platorm_dir);
			return 1;
		}

		string dest_dir = Path.Combine (Environment.CurrentDirectory, args [1]);
		if (!Directory.Exists (dest_dir)) {
			Console.WriteLine ("Data directory '{0}' not found.", dest_dir);
			return 1;
		}

		string rev = args [2];
		if (rev.StartsWith ("r"))
			rev = rev.Substring (1);
		int revision;
		if (!Int32.TryParse (rev, out revision)) {
			Console.WriteLine ("Invalid revision number '{0}'.", args [2]);
			return 1;
		}

		foreach (string assembly in PlatformCode.Assemblies) {
			string filename = Path.Combine (dest_dir, assembly + ".audit");

			// step 1 - read the audit file
			ReadFile (filename);

			// step 2 - process the assembly
			string assembly_name = Path.Combine (platorm_dir, assembly + ".dll");
			ProcessAssembly (assembly_name, revision);

			// step 3 - write the updated audit file
			SaveFile (filename);
		}
		return 0;
	}
}

