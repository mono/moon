// Copyright (C) 2009-2011 Novell, Inc (http://www.novell.com)
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
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

using Mono.Cecil;
using Mono.Cecil.Cil;

using Moonlight.SecurityModel;

// analysis is global but results are kept by assemblies
public class AssemblyData {

	HashSet<string> critical;
	HashSet<string> safe_critical;

	public AssemblyData ()
	{
		critical = new HashSet<string> ();
		safe_critical = new HashSet<string> ();
	}

	public AssemblyDefinition Assembly { get; set; }

	public IEnumerable<string> Critical {
		get { return critical; } 
	}

	public IEnumerable<string> SafeCritical {
		get { return safe_critical; }
	}

	public bool IsSecurityCritical (MethodDefinition method)
	{
		string entry = method.GetFullName ();
		if (critical.Contains (entry))
			return true;

		TypeDefinition type = method.DeclaringType;
		if (!IsSecurityCritical (type))
			return false;

		switch (method.Name) {
		// e.g. everything we override from System.Object (which is transparent)
		case "Equals":
			return (method.Parameters.Count != 1 || method.Parameters [0].ParameterType.FullName != "System.Object");
		case "Finalize":
		case "GetHashCode":
		case "ToString":
			return method.HasParameters;
		// e.g. some transparent interfaces, like IDisposable (implicit or explicit)
		// downgrade some SC into SSC to respect the override/inheritance rules
		case "System.IDisposable.Dispose":
		case "Dispose":
			return method.HasParameters;
		case ".cctor":
			return false;
		default:
			return true;
		}
	}

	public bool IsSecurityCritical (TypeDefinition type)
	{
		string entry = type.GetFullName ();
		if (critical.Contains (entry))
			return true;
		return type.IsNested ? IsSecurityCritical (type.DeclaringType) : false;
	}

	public void MarkAsCritical (MemberReference member)
	{
		string entry = member.GetFullName ();
		if (!critical.Contains (entry))
			critical.Add (entry);

		if (safe_critical.Contains (entry))
			safe_critical.Remove (entry);
	}

	public void MarkAsSafeCritical (MemberReference member)
	{
		string entry = member.GetFullName ();
		if (!safe_critical.Contains (entry))
			safe_critical.Add (entry);
	}
}

class Program {

	static Dictionary<string, AssemblyData> platform = new Dictionary<string, AssemblyData> ();
	static List<AssemblyDefinition> assemblies = new List<AssemblyDefinition> ();

	static IEnumerable<AssemblyDefinition> PlatformAssemblies {
		get { return assemblies; }
	}

	static AssemblyData GetData (AssemblyDefinition assembly)
	{
		return platform [assembly.Name.Name];
	}

	static bool IsSecurityCritical (MethodDefinition method)
	{
		return GetData (method.Module.Assembly).IsSecurityCritical (method);
	}

	static bool IsSecurityCritical (TypeDefinition type)
	{
		return GetData (type.Module.Assembly).IsSecurityCritical (type);
	}

	static string PlatformDirectory { get; set; }

	static string BaseDirectory { get; set; }


	static Dictionary<string, List<string>> annotations = new Dictionary<string, List<string>> ();

	static bool Annotate (MemberReference member, string note)
	{
		if (String.IsNullOrWhiteSpace (note))
			return true;

		note = note.Trim ();

		List<string> list;
		string entry = member.GetFullName ();
		if (!annotations.TryGetValue (entry, out list)) {
			list = new List<string> ();
			annotations.Add (entry, list);
			list.Add (note);
		} else if (!list.Contains (note)) {
			list.Add (note);
		}
		return true;
	}

	static void WriteNotes (string member, StreamWriter sw)
	{
		List<string> list;
		if (annotations.TryGetValue (member, out list)) {
			foreach (string s in list) {
				if (!s.StartsWith ("#"))
					sw.Write ("# ");
				sw.WriteLine (s);
			}
		}
	}

	static void MarkAsCritical (MemberReference member)
	{
		GetData (member.Module.Assembly).MarkAsCritical (member);
	}

	static void MarkAsSafeCritical (MemberReference member)
	{
		GetData (member.Module.Assembly).MarkAsSafeCritical (member);
	}

	#region Stage Helpers

	static int stage = 0;
	static List<string> errors = new List<string> ();
	static Stopwatch watch = new Stopwatch ();

	static void StartStage (string description)
	{
		stage++;
		if (verbose > 0)
			Console.WriteLine ("Stage {0}: {1}", stage, description);
		if (verbose > 2)
			watch.Restart ();
	}

	static void EndStage ()
	{
		if (errors.Count > 0) {
			foreach (string s in errors)
				Console.WriteLine ("* {0}", s);
			errors.Clear ();
		}

		if (verbose > 2)
			Console.WriteLine ("\t\t{0}", watch.Elapsed);
		if (verbose > 0)
			Console.WriteLine ();
	}

	#endregion

	#region Read Platform Code

	static void ReadPlatformCode ()
	{
		try {
			StartStage ("Read Platform Code");

			var resolver = new DefaultAssemblyResolver ();
			resolver.AddSearchDirectory (PlatformDirectory);

			ReaderParameters rp = new ReaderParameters () { AssemblyResolver = resolver };

			foreach (string assembly in PlatformCode.Assemblies) {
				string filename = Path.Combine (PlatformDirectory, assembly + ".dll");
				if (File.Exists (filename)) {
					AssemblyDefinition ad = AssemblyDefinition.ReadAssembly (filename, rp);
					platform.Add (assembly, new AssemblyData () { Assembly = ad });
					assemblies.Add (ad);
					if (verbose > 1)
						Console.WriteLine ("\t{0}.dll: loaded", assembly);
				} else {
					errors.Add (String.Format ("{0}.dll was not found", assembly));
				}
			}
		}
		finally {
			EndStage ();
		}
	}

	#endregion

	#region Read Compatibility Data

	static void ReadCompatibilityData ()
	{
		try {
			StartStage ("Read Compatibility Data");

			foreach (string assembly in PlatformCode.Assemblies) {
				string filename = Path.Combine (BaseDirectory, "compatibility", assembly + ".compat.sc");
				if (File.Exists (filename)) {
					int entries = CheckCompatibleEntries (platform [assembly].Assembly, File.ReadLines (filename));
					if (verbose > 1)
						Console.WriteLine ("\t{0}: {1} entries", filename, entries);
				} else {
					errors.Add (String.Format ("{0} was not found", filename));
				}
			}
		}
		finally {
			EndStage ();
		}
	}

	static MemberReference GetMember (AssemblyDefinition assembly, string entry)
	{
		int pos = entry.IndexOf ("-T: ");
		if (pos != -1)
			return GetType (assembly, entry.Substring (pos + 4));

		pos = entry.IndexOf ("-M: ");
		if (pos == -1)
			return null;

		string method = entry.Substring (pos + 4);
		int start = method.IndexOf (' ') + 1;
		string type = method.Substring (start, method.IndexOf (':') - start);
		TypeDefinition t = GetType (assembly, type);
		if (t == null)
			return null;

		MethodDefinition m = t.Methods.FirstOrDefault ((md) => {
			return method == md.FullName;
		});
		return m;
	}

	static TypeDefinition GetType (AssemblyDefinition assembly, string name)
	{
		TypeDefinition t = assembly.MainModule.GetAllTypes ().FirstOrDefault ((td) => {
			return name == td.FullName;
		});
		return t;
	}

	// ensure the compatibility entry exists in our platform code (e.g. linker error)
	static int CheckCompatibleEntries (AssemblyDefinition assembly, IEnumerable<string> entries)
	{
		int n = 0;
		foreach (string entry in entries) {
			if (entry.Length == 0 || entry [0] == '#')
				continue;

			if (!entry.StartsWith ("!SC-")) {
				Console.WriteLine ("WARNING: Invalid entry action: {0}", entry);
				continue;
			}

			MemberReference m = GetMember (assembly, entry);
			if (m == null) {
				Console.WriteLine ("WARNING: Cannot find member: {0}", entry);
				continue;
			}

			MarkAsCritical (m);
			if (comments)
				Annotate (m, "required for Silverlight API compatibility");
			n++;
		}
		return n;
	}

	#endregion

	#region Read Manual Overrides

	static void ReadManualOverrides ()
	{
		try {
			StartStage ("Read Manual Overrides");

			foreach (string assembly in PlatformCode.Assemblies) {
				string filename = Path.Combine (BaseDirectory, "overrides", assembly + ".manual");
				if (File.Exists (filename)) {
					int entries = CheckManualEntries (platform [assembly].Assembly, File.ReadLines (filename));
					if (verbose > 1)
						Console.WriteLine ("\t{0}: {1} entries", filename, entries);
				} else {
					errors.Add (String.Format ("{0} was not found", filename));
				}
			}
		}
		finally {
			EndStage ();
		}
	}

	static int CheckManualEntries (AssemblyDefinition assembly, IEnumerable<string> entries)
	{
		StringBuilder notes = new StringBuilder ();
		MemberReference m = null;

		int n = 0;
		foreach (string entry in entries) {
			if (entry.Length == 0) {
				notes.Length = 0;
				continue;
			}

			switch (entry [0]) {
			case '#':
				notes.Append (entry).AppendLine ();
				break;
			case '-':
				m = GetMember (assembly, entry);
				if (m == null) {
					errors.Add (String.Format ("WARNING: could not find member in '{0}'", entry));
				} else {
//					removed.Add (m.GetFullName ()); // FIXME
					if (comments) {
						Annotate (m, notes.ToString ());
						Annotate (m, "removed by manual entry");
					}
				}
				n++;
				break;
			case '+':
				m = GetMember (assembly, entry);
				if (m == null) {
					errors.Add (String.Format ("WARNING: could not find member in '{0}'", entry));
					continue;
				} else if (comments) {
					Annotate (m, notes.ToString ());
					Annotate (m, "entry added by a manual override");
				}

				if (entry.StartsWith ("+SC", StringComparison.Ordinal))
					MarkAsCritical (m);
				else if (entry.StartsWith ("+SSC", StringComparison.Ordinal))
					MarkAsSafeCritical (m);
				else
					errors.Add (String.Format ("WARNING: invalid action specified in '{0}'", entry));
				n++;
				break;
			}
		}
		return n;
	}

	#endregion

	#region Detect Critical Code

	static void DetectCriticalCode ()
	{
		try {
			StartStage ("Detect [SecurityCritical] Code");

			foreach (AssemblyDefinition assembly in PlatformAssemblies) {
				foreach (ModuleDefinition module in assembly.Modules) {
					foreach (TypeDefinition type in module.GetAllTypes ()) {
						DetectCriticalType (type);
					}
				}
			}
		}
		finally {
			EndStage ();
		}
	}

	static void DetectCriticalType (TypeDefinition type)
	{
		// if we're not critical
		if (!IsSecurityCritical (type)) {
			// does we inherit from a SecurityCritical type ?
			TypeReference btype = type.BaseType;
			if ((btype != null) && IsSecurityCritical (btype.Resolve ())) {
				MarkAsCritical (type);
				if (comments)
					Annotate (type, "inherits from [SecurityCritical] " + btype.FullName);
			}

			if (type.HasInterfaces) {
				foreach (TypeReference intf in type.Interfaces) {
					if (IsSecurityCritical (intf.Resolve ())) {
						MarkAsCritical (type);
						if (!comments)
							break;
						Annotate (type, "implements [SecurityCritical] " + intf.FullName);
					}
				}
			}
		}

		// note: avoid marking interfaces as [SecurityCritical] as this complexify up inheritance
		// quite a bit. Simply mark the/some members when needed

		if (type.HasMethods) {
			foreach (MethodDefinition method in type.Methods) {
				DetectCriticalMethod (method);
			}
		}
	}

	// note: pointers don't *need* to be SecurityCritical because they can't be
	// used without a "unsafe" or "fixed" context that transparent code won't support
	// except in very limited circumstances
	static bool CheckType (TypeReference type)
	{
		string fullname = type.FullName;

		// pointers can only be used by fixed/unsafe code
		return !fullname.EndsWith ("*", StringComparison.Ordinal);
	}

	static string CheckVerifiability (MethodDefinition method)
	{
		if (!method.HasBody)
			return String.Empty;

		foreach (Instruction ins in method.Body.Instructions) {
			switch (ins.OpCode.Code) {
			case Code.No:		// ecma 335, part III, 2.2
			case Code.Calli:	// Lidin p260
			case Code.Cpblk:	// ecma 335, part III, 3.30
			case Code.Initblk:	// ecma 335, part III, 3.36
			case Code.Jmp:		// ecma 335, part III, 3.37 / Lidin p259
			case Code.Localloc:	// ecma 335, part III, 3.47
				return ins.OpCode.Name;
			case Code.Arglist:	// lack test case
			case Code.Cpobj:
				return ins.OpCode.Name;
			case Code.Mkrefany:	// not 100% certain
				return ins.OpCode.Name;
			}
		}
		return String.Empty;
	}

	static bool Compare (MethodDefinition m1, MethodDefinition m2, bool checkExplicitInterface = false)
	{
		if (m1.Name != m2.Name) {
			if (!checkExplicitInterface)
				return false;
			if (m1.Name != m2.DeclaringType.FullName + "." + m2.Name)
				return false;
		}

		if (m1.Parameters.Count != m2.Parameters.Count)
			return false;

		for (int i = 0; i < m1.Parameters.Count; i++) {
			ParameterDefinition p1 = m1.Parameters [i];
			ParameterDefinition p2 = m2.Parameters [i];
			if (p1.ParameterType.FullName != p2.ParameterType.FullName)
				return false;
		}
		return (m1.ReturnType.FullName == m2.ReturnType.FullName);
	}

	// check if the specified method is implementing an interface of the type (or base types)
	static string CheckInterfaces (MethodDefinition method)
	{
		TypeDefinition type = method.DeclaringType;
		while (type != null) {
			if (type.HasInterfaces) {
				foreach (TypeReference intf in type.Interfaces) {
					TypeDefinition td = intf.Resolve ();
					if (td == null || !td.HasMethods)
						continue;
					foreach (MethodDefinition im in td.GetMethods ()) {
						if (IsSecurityCritical (im)) {
							if (Compare (method, im, true)) {
								return String.Format ("implements '{0}'.", im);
							}
						}
					}
				}
			}
			type = type.BaseType == null ? null : type.BaseType.Resolve ();
		}
		return String.Empty;
	}

	static void DetectCriticalMethod (MethodDefinition method)
	{
		// p/invoke methods needs to be [SecurityCritical] to be executed, 
		// unless they are inside a type named "SafeNativeMethods"
		bool sc = (method.IsPInvokeImpl && (method.DeclaringType.Name != "SafeNativeMethods"));
		if (sc) {
			MarkAsCritical (method);
			if (!comments)
				return;
			Annotate (method, "p/invoke declaration");
		}

		string comment = CheckVerifiability (method);
		if (!String.IsNullOrEmpty (comment)) {
			MarkAsCritical (method);
			if (!comments)
				return;
			Annotate (method, comment);
		}

		// skip signature check for visible API (we get them from find-sc)
		if (!method.IsVisible ()) {
			if (method.HasParameters) {
				// compilers will add public stuff like: System.Action`1::.ctor(System.Object,System.IntPtr)
				if (!method.IsConstructor || (method.DeclaringType as TypeDefinition).BaseType.FullName != "System.MulticastDelegate") {
					foreach (ParameterDefinition p in method.Parameters) {
						if (!CheckType (p.ParameterType)) {
							MarkAsCritical (method);
							if (!comments)
								return;
							Annotate (method, String.Format ("using '{0}' as a parameter type", p.ParameterType.FullName));
						}
					}
				}
			}

			TypeReference rtype = method.ReturnType;
			if (!CheckType (rtype)) {
				MarkAsCritical (method);
				if (!comments)
					return;
				Annotate (method, String.Format ("using '{0}' as return type", rtype.FullName));
			}
		}

		// check if this method implements an interface where the corresponding member
		// is [SecurityCritical]
		comment = CheckInterfaces (method);
		if (!String.IsNullOrEmpty (comment)) {
			MarkAsCritical (method);
			if (!comments)
				return;
			if (method.IsVisible ())
				comment = "VISIBLE " + comment;
			Annotate (method, comment);
		}

		sc = IsSecurityCritical (method);
		// if we're overriding a [SecurityCritical] method then we must be one too! or
		// if we are [SecurityCritical] then the base method needs to be too!
		if (method.IsVirtual && !method.IsAbstract) {
			TypeReference tr = method.DeclaringType.BaseType;
			if (tr != null) {
				TypeDefinition td = tr.Resolve ();
				if (td != null) {
					foreach (MethodDefinition bm in td.GetMethods ()) {
						if (Compare (method, bm)) {
							if (!sc) {
								if (IsSecurityCritical (bm)) {
									MarkAsCritical (method);
									if (!comments)
										return;
									Annotate (method, String.Format ("{0}[SecurityCritical] required to override '{1}'.",
										method.IsVisible () ? "WARNING! New VISIBLE " : String.Empty, bm));
								}
							} else {
								if (!IsSecurityCritical (bm) && bm.Name != "Finalize") {
									MarkAsCritical (bm);
									if (!comments)
										return;
									Annotate (bm, String.Format ("{0}base method promoted to [SecurityCritical] because of '{1}'.",
										bm.IsVisible () ? "WARNING! New VISIBLE " : String.Empty, method));
								}
							}
						}
					}
				}
			}
		} else {
			// note: we don't want to break the override rules above (resulting in TypeLoadException)
			// an icall that is NOT part of the visible API is considered as critical (like a p/invoke)
			if (method.IsInternalCall && !method.IsVisible ()) {
				MarkAsCritical (method);
				if (!comments)
					return;
				Annotate (method, "non-visible internal call");
			}
		}

		// if this method is [SecurityCritical] (already or because we determined it should be)
		// and implements an interface then some interface members may needs to be [SecurityCritical] 
		// too or a TypeLoadException will be thrown 
		// e.g. AppDomain.add_AssemblyResolve versus _AppDomain
		TypeDefinition type = method.DeclaringType;
		if (sc && type.HasInterfaces) {
			foreach (TypeReference intf in type.Interfaces) {
				TypeDefinition td = intf.Resolve ();
				if (td == null || !td.HasMethods)
					continue;
				foreach (MethodDefinition im in td.GetMethods ()) {
					// note: in this case we don't care if the method is indirectly critical (e.g.via its type)
					if (Compare (method, im) && !IsSecurityCritical (type)) {
						// only add this if it's not something we already found before
						if (!IsSecurityCritical (im)) {
							MarkAsCritical (im);
							if (!comments)
								return;
							Annotate (im, String.Format ("Promoting {0}interface member to [SecurityCritical] because of '{1}'.",
								im.IsVisible () ? "VISIBLE " : String.Empty, method));
						}
					}
				}
			}
		}
	}

	#endregion

	#region Detect SafeCritical Code

	static void DetectSafeCriticalCode ()
	{
		try {
			StartStage ("Detect [SecuritySafeCritical] Code");

			foreach (AssemblyDefinition assembly in PlatformAssemblies) {
				foreach (ModuleDefinition module in assembly.Modules) {
					foreach (TypeDefinition type in module.GetAllTypes ()) {
						DetectSafeCriticalType (type);
					}
				}
			}
		}
		finally {
			EndStage ();
		}
	}

	static void DetectSafeCriticalType (TypeDefinition type)
	{
		if (type.HasMethods) {
			foreach (MethodDefinition method in type.Methods) {
				DetectSafeCriticalMethod (method);
			}
		}
	}

	static void DetectSafeCriticalMethod (MethodDefinition method)
	{
		// we consider all SafeNativeMethods.* as [SecuritySafeCritical]
		// if not then move them into NativeMethods or UnsafeNativeMethods
		if (method.IsPInvokeImpl && (method.DeclaringType.Name == "SafeNativeMethods")) {
			MarkAsSafeCritical (method);
			if (!comments)
				return;
			Annotate (method, "pinvoke / SafeNativeMethods");
		}

		// SC code can call SC code
		// note: here we use the 'rock' because the [SecurityCritical] attribute 
		// could be located on the type (or nested type)
		if (IsSecurityCritical (method))
			return;

		// an icall that is part of the visible API is considered as safe critical
		if (method.IsInternalCall) {
			if (method.IsVisible ()) {
				MarkAsSafeCritical (method);
				if (!comments)
					return;
				Annotate (method, "VISIBLE internal call");
			} else if (method.IsVirtual) {
				MarkAsSafeCritical (method);
				if (!comments)
					return;
				Annotate (method, "virtual internal call");
			}
		}

		if (!method.HasBody)
			return;

		foreach (Instruction ins in method.Body.Instructions) {
			MethodReference mr = (ins.Operand as MethodReference);
			if (mr == null)
				continue;

			MethodDefinition md = mr.Resolve ();
			if (md == null) {
				// this can occurs for some generated types, like Int[,] where the compiler generates a few methods
				continue;
			}

			// again we use the rock here as we want the final result (not the local attribute)
			if (IsSecurityCritical (md)) {
				MarkAsSafeCritical (method);
				if (!comments)
					return;
				Annotate (method, String.Format ("{0}method calling [SecurityCritical] {1}", 
					md.IsVisible () ? "VISIBLE " : String.Empty, md.FullName));
			}
		}
	}

	#endregion

	#region Save

	static void Save ()
	{
		try {
			StartStage ("Save Security Attributes Data Files");

			int tsc = 0;
			int tssc = 0;
			foreach (KeyValuePair<string,AssemblyData> kvp in platform) {
				if (verbose > 1)
					Console.Write ("\t{0}.dll: ", kvp.Key);

				AssemblyData data = kvp.Value;
				int sc = 0;
				int ssc = 0;
				using (StreamWriter sw = new StreamWriter (kvp.Key + ".secattr")) {
					sc = WriteList (sw, "SC", data.Critical);
					ssc = WriteList (sw, "SSC", data.SafeCritical);
				}

				if (verbose > 1)
					Console.WriteLine ("{0} [SC], {1} [SSC]", sc, ssc);
				tsc += sc;
				tssc += ssc;
			}

			if (verbose > 1)
				Console.WriteLine ();
			Console.WriteLine ("Total SC: {0}, SSC: {1}", tsc, tssc);
		}
		finally {
			EndStage ();
		}
	}

	static int WriteList (StreamWriter sw, string prefix, IEnumerable<string> enumerable)
	{
		List<string> list = enumerable.ToList ();
		list.Sort (); // makes it easier to diff between old/new files
		foreach (string entry in list) {
			if (comments) {
				sw.WriteLine ();
				WriteNotes (entry, sw);
			}
			sw.WriteLine ("{0}-{1}: {2}", prefix, entry.IndexOf (' ') == -1 ? 'T' : 'M', entry);
		}
		return list.Count;
	}


	#endregion

	#region Driver

	static int verbose;
	static bool comments;

	static void Error (string msg)
	{
		if (!String.IsNullOrEmpty (msg)) {
			Console.Write ("Error: ");
			Console.WriteLine (msg);
		}
		Console.WriteLine ("Usage: mono detect.exe platform-directory [base-directory] [--a] [--v ...] [--help]");
		Console.WriteLine ("  platform-directory   Directory where the platform assemblies are located");
		Console.WriteLine ("  base-directory       Base directory to find compatibility / overrides data");
		Console.WriteLine ("  --a                  Write annotations/comments in the output files");
		Console.WriteLine ("  --v                  Add verbosity (can be used several times)");
		Console.WriteLine ("  --help               Add verbosity (can be used several times)");
	}

	static int Main (string [] args)
	{
		foreach (string arg in args) {
			switch (arg) {
			case "-v":
			case "--v":
				verbose++;
				break;
			case "-a":
			case "--a":
				comments = true;
				break;
			case "-h":
			case "--help":
				Error (String.Empty);
				return 1;
			default:
				if (String.IsNullOrEmpty (PlatformDirectory))
					PlatformDirectory = arg;
				else if (String.IsNullOrEmpty (BaseDirectory))
					BaseDirectory = arg;
				else
					Error (String.Format ("unknown argument: '{0}'", arg));
				break;
			}
		}

		if (String.IsNullOrEmpty (PlatformDirectory)) {
			Error ("missing platform-directory argument");
			return 1;
		}
		
		if (String.IsNullOrEmpty (BaseDirectory))
			BaseDirectory = ".";

		// stage 1: read platform assemblies into memory
		ReadPlatformCode ();

		// stage 2: read compatibility data - this contains only visible [SecurityCritical] code
		ReadCompatibilityData ();

		// stage 3: read manual overrides
		ReadManualOverrides ();

		// stage 4: detect code that needs to be [SecurityCritical], e.g. pinvokes, unsafe...
		DetectCriticalCode ();

		// stage 5: caller analysis, anything calling [SecurityCritical] 
		// code needs to the marked as [SecuritySafeCritical] and audited
		DetectSafeCriticalCode ();

		// stage 6: save .secattr files that the tuner will inject back into the assemblies
		Save ();

		return 0;
	}

	#endregion
}

