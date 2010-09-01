
using NDesk.Options;
using System;
using System.IO;
using System.Text;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;

using System.Xml;
using System.Xml.XPath;


namespace Moonlight {

	public class MXap {

		private string application_name;
		private List<string> external_assemblies;
		private List<string> reference_assemblies;
		private List<string> packages;
		private List<string> package_assemblies;
		private List<string> mdb_files;
		private List<string> csharp_files;
		private List<string> xaml_files;
		private List<string> external_parts;
		private List<string> defines;
		private Dictionary<string, string> resources;
		private Dictionary<string, string> assembly_resources;
		private Dictionary<string, string> content_resources;
		private string top_builddir = null; // Defaults to null
		private bool desktop = false; // Defaults to false
		private bool generate_html = true; // Defaults to true
		private bool include_mdb = true; // Defaults to true
		private bool verbose = false; //default to false
		private bool generate_manifest = true; 
		private bool list_generated = false;
		private string entry_point_type = null;
		private string cs_sources;
		private string cd;
		private bool in_place = true;
		
		const string RuntimeVersion2 = "2.0.31005.0";
		const string RuntimeVersion3 = "3.0.40818.0";
		const string RuntimeVersion4 = "4.0.50826.0";

		private string RuntimeVersion = RuntimeVersion2;

		public string CSSources {
			get { return cs_sources; }
			set { cs_sources = value; }
		}
		
		public string EntryPointType {
			get { return entry_point_type; }
			set { entry_point_type = value;  }
		}
		
		public bool GenerateManifest {
			get { return generate_manifest; }
			set { generate_manifest = value; }
		}
		
		public bool GenerateHtml {
			get { return generate_html; }
			set { generate_html = value; }
		}

		public bool Desktop {
			get { return desktop; }
			set { desktop = value; }
		}

		public string TopBuildDir {
			get { return top_builddir; }
			set { top_builddir = value; }
		}

		public bool IncludeMdb {
			get { return include_mdb; }
			set { include_mdb = value; }
		}

		public bool Verbose {
			get { return verbose; }
			set { verbose = value; }
		}

		public bool ListGenerated {
			get { return list_generated; }
			set { list_generated = value; }
		}
		
		public string Cd {
			get { return cd; }
			set { cd = value; }
		}

		public string ApplicationName {
			get {
				if (application_name == null) {
					DirectoryInfo di = new DirectoryInfo (Directory.GetCurrentDirectory ());
					application_name = di.Name;
					application_name.Replace ("-", "_");
				}

				return application_name;
			}
			set {
				application_name = value;
			}
		}

		public List<string> ReferenceAssemblies {
			get {
				if (reference_assemblies == null)
					reference_assemblies = new List<string> ();
				return reference_assemblies;
			}
		}

		public List<string> ExternalAssemblies {
			get {
				if (external_assemblies == null)
					external_assemblies = new List<string> ();
				return external_assemblies;
			}
		}

		public List<string> Packages {
			get {
				if (packages == null)
					packages = new List<string> ();
				return packages;
			}
		}

		public List<string> PackageAssemblies {
			get {
				if (package_assemblies == null)
					package_assemblies = new List<string> ();
				return package_assemblies;
			}
		}

		public List<string> MdbFiles {
			get {
				if (mdb_files == null)
					mdb_files = new List<string> ();
				return mdb_files;
			}
		}

		public List<string> CSharpFiles {
			get {
				if (csharp_files == null)
					csharp_files = new List<string> ();
				return csharp_files;
			}
		}

		public List<string> XamlFiles {
			get {
				if (xaml_files == null)
					xaml_files = new List<string> ();
				return xaml_files;
			}
		}

		public List<string> ExternalPartManifests {
			get {
				if (external_parts == null)
					external_parts = new List<string> ();
				return external_parts;
			}
		}

		public List<string> Defines {
			get {
				if (defines == null)
					defines = new List<string> ();
				return defines;
			}
		}

		public Dictionary<string, string> Resources {
			get {
				if (resources == null)
					resources = new Dictionary<string, string> ();
				return resources;
			}
		}

		public Dictionary<string, string> AssemblyResources {
			get {
				if (assembly_resources == null)
					assembly_resources = new Dictionary<string, string> ();
				return assembly_resources;
			}
		}

		public Dictionary<string, string> ContentResources {
			get {
				if (content_resources == null)
					content_resources = new Dictionary<string, string> ();
				return content_resources;
			}
		}

		public string TmpDir { get; set; }
		public string WorkingDir { get; set; }
		public string OutputDir { get; set; }
		public bool InPlace {
			get { return in_place; }
			set { in_place = value; }
		}

		public bool Run ()
		{
			return (CreateManifest ()
				&& CreateCodeBehind ()
				&& CreateResources ()
				&& CreateApplicationAssembly ()
				&& CreateXap ()
				&& CreateHtmlWrapper ());
		}

		public bool CreateManifest ()
		{
			string AppManifest_Filename = "AppManifest.xaml";

			if (!GenerateManifest)
				return true;
			
			if (ListGenerated) {
				Console.WriteLine (AppManifest_Filename);
				return true;
			}

			StringBuilder manifest = new StringBuilder ();

			manifest.Append ("<Deployment xmlns=\"http://schemas.microsoft.com/client/2007/deployment\"");
			manifest.Append (" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" ");
			manifest.AppendFormat ("EntryPointAssembly=\"{0}\" EntryPointType=\"", ApplicationName);
			if (entry_point_type != null)
				manifest.Append (entry_point_type);
			else 
				manifest.AppendFormat ("{0}.App", ApplicationName);
			manifest.AppendFormat ("\" RuntimeVersion=\"{0}\">\n", RuntimeVersion);

			manifest.AppendLine ("  <Deployment.Parts>");

			foreach (string assembly in ReferenceAssemblies) {
				manifest.AppendFormat ("    <AssemblyPart x:Name=\"{0}\" Source=\"{1}\" />\n", Path.GetFileNameWithoutExtension (assembly), Path.GetFileName (assembly));
			}

			manifest.AppendFormat ("    <AssemblyPart x:Name=\"{0}\" Source=\"{1}.dll\" />\n", ApplicationName, ApplicationName);			
			manifest.AppendLine ("  </Deployment.Parts>");

			if (external_parts != null && external_parts.Count > 0) {
				manifest.AppendLine ("  <Deployment.ExternalParts>");
				foreach (string ext_part in ExternalPartManifests) {
					if (!CreateExternalPartsFromManifest (ext_part, manifest))
						return false;
				}
				manifest.AppendLine ("  </Deployment.ExternalParts>");
			}

			manifest.AppendLine ("</Deployment>");

			File.WriteAllText (AppManifest_Filename, manifest.ToString ());

			return true;
		}

		// Creates an extension part package, and adds the extension part
		// definitions to the assembly manifest.
		public bool CreateExternalPartsFromManifest (string path, StringBuilder manifest)
		{
			XPathDocument doc = new XPathDocument (path);
			XPathNavigator nav = doc.CreateNavigator ();
			XPathNodeIterator assemblies = nav.Select ("/manifest/assembly");

			if (assemblies == null) {
				ExternalPartError (path, "No assembly elements found.");
				return false;
			}

			while (assemblies.MoveNext ()) {
				CreateExternalPart (path, assemblies.Current, manifest);
			}
			
			return true;
		}

		public bool CreateExternalPart (string path, XPathNavigator assembly, StringBuilder manifest)
		{
			string name;
			string version;
			string publickeytoken;
			string relpath;
			string source;

			XPathNavigator nav = assembly.SelectSingleNode ("name");
			if (nav == null) {
				ExternalPartError (path, "Invalid assembly element, no name specified.");
				return false;
			}
			name = nav.Value;

			nav = assembly.SelectSingleNode ("version");
			if (nav == null) {
				ExternalPartError (path, "Invalid assembly element, no version specified.");
				return false;
			}
			version = nav.Value;
				
			nav = assembly.SelectSingleNode ("publickeytoken");
			if (nav == null) {
				ExternalPartError (path, "Invalid assembly element, no publickeytoken specified.");
				return false;
			}
			publickeytoken = nav.Value;

			nav = assembly.SelectSingleNode ("relpath");
			if (nav == null) {
				ExternalPartError (path, "Invalid assembly element, no relpath specified.");
				return false;
			}
			relpath = nav.Value;

			nav = assembly.SelectSingleNode ("extension/@downloadUri");
			if (nav == null) {
				ExternalPartError (path, "Invalid assembly element, no extension source specified.");
				return false;
			}
			source = nav.Value;

			manifest.AppendFormat ("<ExtensionPart Source=\"{0}\" />\n", source);

			// If its an absolute URI we don't add it to the package
			Uri dummy = null;
			if (Uri.TryCreate (source, UriKind.Absolute, out dummy))
				return true;

			if (!File.Exists (relpath)) {
				ExternalPartError (path, String.Format ("Invalid assembly element, unable to find assembly {0}.", source));
				return false;
			}

			if (!VerifyExternalPartAssembly (path, name, version, publickeytoken, relpath))
				return false;

			return RunProcess ("zip", String.Format (" {0} {1}", source, relpath));
		}

		private bool VerifyExternalPartAssembly (string path, string name, string version, string publickeytoken, string relpath)
		{
			Assembly asm = Assembly.ReflectionOnlyLoadFrom (relpath);
			AssemblyName aname = asm.GetName ();

			if (aname.Name != name) {
				ExternalPartVerificationError (path, String.Format ("Names did not match ({0}, {1}).", aname.Name, name));
				return false;
			}

			if (aname.Version.ToString () != version) {
				ExternalPartVerificationError (path, String.Format ("Versions did not match ({0}, {1}).", aname.Version, version));
				return false;
			}

			string otoken = BitConverter.ToString (aname.GetPublicKeyToken ()).Replace("-","");
			if (otoken != publickeytoken.ToUpper ()) {
				ExternalPartVerificationError (path, String.Format ("Public key tokens do not match ({0}, {1}).", otoken, publickeytoken.ToUpper ()));
				return false;
			}
			
			return true;
		}

		private void ExternalPartError (string path, string error)
		{
			Console.Error.WriteLine ("Invalid external part manifest '{0}'. {1}", path, error);
		}

		private void ExternalPartVerificationError (string path, string error)
		{
			ExternalPartError (path, String.Concat ("Assembly verification failed. ", error));
		}	

		public bool CreateCodeBehind ()
		{
			if (ListGenerated) {
				foreach (string xaml_file in XamlFiles) {
					if (Path.GetFileName (xaml_file) == "AppManifest.xaml")
						continue;
					Console.WriteLine ("{0}.g.cs", Path.GetFileName (xaml_file));
				}
				return true;
			}
			else {
				StringBuilder xamlg_args = new StringBuilder ();

				xamlg_args.AppendFormat (" -sl2app:{0} ", ApplicationName);
				xamlg_args.AppendFormat (" -root:{0} ", WorkingDir);

				foreach (string xaml_file in XamlFiles) {
					if (Path.GetFileName (xaml_file) == "AppManifest.xaml")
						continue;
					//xamlg_args.AppendFormat (" {0}", Path.GetFileName (xaml_file));
					xamlg_args.AppendFormat (" \"{0}\"", xaml_file);
				}

				return RunTool ("xamlg",
						"tools/xamlg/xamlg.exe",
						xamlg_args.ToString ());
			}
		}

		public bool CreateResources ()
		{
			if (ListGenerated) {
				Console.WriteLine ("{0}.g.resources", ApplicationName);
				return true;
			}

			StringBuilder respack_args = new StringBuilder ();

			respack_args.AppendFormat ("{0}.g.resources ", ApplicationName);

			foreach (string xaml_file in XamlFiles) {
				if (Path.GetFileName (xaml_file) == "AppManifest.xaml")
					continue;
				//respack_args.AppendFormat (" {0}", Path.GetFileName (xaml_file));
				respack_args.AppendFormat (" \"{0}\"", xaml_file);
			}

			foreach (KeyValuePair<string, string> pair in Resources)
				respack_args.AppendFormat (" \"{0},{1}\"", pair.Value, pair.Key);
			
			return RunTool ("respack",
					"tools/respack/respack.exe",
					respack_args.ToString ());
		}

		public bool CreateApplicationAssembly ()
		{
			if (ListGenerated) {
				Console.WriteLine ("{0}.dll", ApplicationName);
				return true;
			}

			StringBuilder compiler_args = new StringBuilder ();

			if (desktop)
				compiler_args.Append (" -d:DESKTOP ");
			foreach (string define in Defines) {
				compiler_args.AppendFormat (" -d:{0} ", define);
			}

			compiler_args.AppendFormat (" -debug+ -target:library -out:{0}.dll ", ApplicationName);

			foreach (string asm in ReferenceAssemblies) {
				compiler_args.AppendFormat (" -r:\"{0}\" ", asm);
			}

			foreach (string asm in ExternalAssemblies) {
				compiler_args.AppendFormat (" -r:{0} ", asm);
			}

			foreach (string asm in PackageAssemblies) {
				compiler_args.AppendFormat (" -r:{0} ", asm);
			}

			if (desktop && top_builddir == null)
					compiler_args.Append (" -pkg:moonlight-gtk-2.0");

			foreach (string cs in CSharpFiles) {
				if (cs.EndsWith (".g.cs"))
					continue;
				compiler_args.AppendFormat (" \"{0}\" ", cs);
			}

			foreach (string xaml in XamlFiles) {
				if (Path.GetFileName (xaml) == "AppManifest.xaml")
					continue;
				if (!File.Exists (Path.GetFileName (xaml) + ".g.cs"))
					continue;
				compiler_args.AppendFormat (" {0}.g.cs ", Path.GetFileName (xaml));
			}

			compiler_args.AppendFormat (" -resource:{0}.g.resources ", ApplicationName);

			foreach (KeyValuePair<string, string> pair in AssemblyResources)
				compiler_args.AppendFormat (" -resource:\"{0},{1}\"", pair.Value, pair.Key);

			if (desktop)
				return RunProcess ("gmcs", compiler_args.ToString());
			else
				return Run21Tool ("smcs",
						   "class/lib/2.1/smcs.exe",
						   compiler_args.ToString());
		}

		public bool CreateXap ()
		{
			if (ListGenerated) {
				Console.WriteLine ("{0}.xap", ApplicationName);
				return true;
			}

			StringBuilder zip_args = new StringBuilder ();
			zip_args.AppendFormat (" {0}.xap ", ApplicationName);

			foreach (string asm in ReferenceAssemblies) {
				zip_args.AppendFormat (" {0} ", Path.GetFileName (asm));
			}
			foreach (string mdb in MdbFiles) {
				zip_args.AppendFormat (" {0} ", Path.GetFileName (mdb));
			}

			zip_args.AppendFormat (" AppManifest.xaml ");
			zip_args.AppendFormat (" {0}.dll ", ApplicationName);
			zip_args.AppendFormat (" {0}.dll.mdb ", ApplicationName);

			foreach (KeyValuePair<string, string> pair in ContentResources)
				zip_args.AppendFormat (" " + pair.Value);

			return RunProcess ("zip", zip_args.ToString ());
		}

		public bool CreateHtmlWrapper ()
		{
			if (!GenerateHtml)
				return true;

			if (ListGenerated) {
				Console.WriteLine ("{0}.html", ApplicationName);
				return true;
			}

			StringBuilder xaml2html_args = new StringBuilder ();

			xaml2html_args.AppendFormat (" {0}.xap ", ApplicationName);

			return RunTool ("xaml2html",
					"tools/xaml2html/xaml2html.exe",
					xaml2html_args.ToString ());
		}

		private bool Run21Tool (string filename, string builddir_exe, string args)
		{
			string old_mono_path = Environment.GetEnvironmentVariable ("MONO_PATH");
			if (TopBuildDir != null) {
				string new_mono_path = Path.Combine (
							     Path.Combine (
								   Path.Combine (TopBuildDir, "class"),
								   "lib"),
							     "2.1");
				if (!string.IsNullOrEmpty (old_mono_path))
					new_mono_path = string.Format ("{0}:{1}", new_mono_path, old_mono_path);
				Environment.SetEnvironmentVariable ("MONO_PATH", new_mono_path);
			}

			bool rv = RunTool (filename, builddir_exe, args);

			if (TopBuildDir != null)
				Environment.SetEnvironmentVariable ("MONO_PATH", old_mono_path);

			return rv;
		}
	

		private bool RunTool (string filename, string builddir_exe, string args)
		{
			if (top_builddir != null) {
				bool need_smcs_hack = false;
				string moonlight_mono = null;
				if (filename == "smcs") {
					need_smcs_hack = true;
					moonlight_mono = Environment.GetEnvironmentVariable ("MOONLIGHT_MONO");
				}
				if (string.IsNullOrEmpty (moonlight_mono))
					filename = "mono";
				else
					filename = moonlight_mono;
				args = String.Format ("{3}{0}/{1} {2}", top_builddir, builddir_exe, args,
						      need_smcs_hack ? "--runtime=moonlight --security=temporary-smcs-hack " : "");
			}

			return RunProcess (filename, args);
		}

		private bool RunProcess (string filename, string args)
		{
			string ret;
			return RunProcess (filename, args, out ret);
		}

		private bool RunProcess (string filename, string args, out string result)
		{
			if (Verbose)
				Console.WriteLine ("Running {0} {1}", filename, args);

			Process process = new Process ();

			process.StartInfo.FileName = filename;
			process.StartInfo.Arguments = args;
			process.StartInfo.CreateNoWindow = true;
			process.StartInfo.UseShellExecute = false;
			process.StartInfo.RedirectStandardOutput = true;

			process.Start ();

			result = null;
			if (process.StandardOutput != null)
				result = process.StandardOutput.ReadToEnd ();

			process.WaitForExit ();
			bool ret = (process.ExitCode == 0);

			process.Close ();
			return ret;
		}

		static void ShowHelp (OptionSet os)
		{
			Console.WriteLine ("mxap usage is: mxap [options] [directory]");
			Console.WriteLine ();
			os.WriteOptionDescriptions (Console.Out);
		}

		void AddResource (string v)
		{
			if (string.IsNullOrEmpty (v))
				return;

			string name, filename;
			bool local = Path.GetFullPath (v).StartsWith (Path.GetFullPath ("."));
			int comma = v.IndexOf (',');
			if (comma == -1) {
				if (local) {
					name = v;
					filename = v;
				} else {
					name = filename = Path.GetFileName (v);
					if (!InPlace)
						File.Copy (v, Path.Combine (TmpDir, Path.GetFileName (v)));
					else
						File.Copy (v, Path.Combine (WorkingDir, Path.GetFileName (v)));
				}
			} else {
				name = v.Substring (comma + 1);
				filename = v.Substring (0, comma);
				if (!InPlace) {
					File.Copy (filename, Path.Combine (TmpDir, Path.GetFileName (filename)));
					filename = Path.Combine (TmpDir, Path.GetFileName (filename));
				}
			}

			Resources.Add (name, filename);
		}

		void AddAssemblyResource (string v)
		{
			if (string.IsNullOrEmpty (v))
				return;

			string name, filename;

			bool local = Path.GetFullPath (v).StartsWith (Path.GetFullPath ("."));
			int comma = v.IndexOf (',');
			if (comma == -1) {
				if (local) {
					name = ApplicationName + "." + v.Replace ('/', '.');
					filename = v;
				} else {
					name = ApplicationName + "." + Path.GetFileName (v);
					filename = Path.GetFileName (v);
					if (!InPlace)
						File.Copy (v, Path.Combine (TmpDir, filename));
					else
						File.Copy (v, Path.Combine (WorkingDir, filename));
				}
			} else {
				name = v.Substring (comma + 1);
				filename = v.Substring (0, comma);
				if (!InPlace) {
					File.Copy (filename, Path.Combine (TmpDir, Path.GetFileName (filename)));
					filename = Path.Combine (TmpDir, Path.GetFileName (filename));
				}
			}

			AssemblyResources.Add (name, filename);
		}

		void AddContentResource (string v)
		{
			if (string.IsNullOrEmpty (v))
				return;

			string name, filename;
			bool local = Path.GetFullPath (v).StartsWith (Path.GetFullPath ("."));
			int comma = v.IndexOf (',');
			if (comma == -1) {
				if (local) {
					name = v;
					filename = v;
				} else {
					name = filename = Path.GetFileName (v);
					if (!InPlace)
						File.Copy (v, Path.Combine (TmpDir, Path.GetFileName (v)));
					else
						File.Copy (v, Path.Combine (WorkingDir, Path.GetFileName (v)));
				}
			} else {
				name = v.Substring (comma + 1);
				filename = v.Substring (0, comma);
				if (!InPlace) {
					File.Copy (filename, Path.Combine (TmpDir, Path.GetFileName (filename)));
					filename = Path.Combine (TmpDir, Path.GetFileName (filename));
				}
			}

			ContentResources.Add (name, filename);
		}

		void SetRuntimeVersion (string v)
		{
			switch (v) {
			case "2":
				RuntimeVersion = RuntimeVersion2;
				break;
			case "3":
				RuntimeVersion = RuntimeVersion3;
				break;
			case "4":
				RuntimeVersion = RuntimeVersion4;
				break;
			default:
				RuntimeVersion = v;
				break;
			}
		}

		static void DoClean (string app)
		{
			File.Delete (app + ".dll");
			File.Delete (app + ".dll.mdb");
			File.Delete (app + ".g.resources");
			File.Delete (app + ".html");
			File.Delete (app + ".xap");
			File.Delete ("AppManifest.xaml");
			foreach (string path in Directory.GetFiles (Directory.GetCurrentDirectory(), "*.g.cs"))
				File.Delete(path);
		}

		static void RecursiveCopy (string source, string dir, string dest)
		{
			foreach (string file in Directory.GetFiles (source + dir))
				File.Copy (file, Path.Combine (dest + dir, Path.GetFileName (file)));
			foreach (string d in Directory.GetDirectories (source + dir)) {
				string d1 = d.Replace (source, "");
				Directory.CreateDirectory (dest + d1);
				RecursiveCopy (source, d1, dest);
			}
		}

		
		static bool ParseBool (string v, bool defaul) {
			if (v == null) return true;
			bool ret;
			if (bool.TryParse (v, out ret))
				return ret;
			if (v.ToLower() == "no") return false;
			if (v.ToLower() == "yes") return true;
			return defaul;
		}	
		
		public static int Main (string [] args)
		{
			MXap mxap = new MXap ();
			bool help = false;
			bool clean = false;
			List<string> resources = new List<string>();
			List<string> aresources = new List<string>();
			List<string> cresources = new List<string>();
			string resourceFile = null;
			string aresourceFile = null;
			string cresourceFile = null;

			mxap.Cd = Directory.GetCurrentDirectory ();

			var p = new OptionSet () {
				{ "h|?|help", v => help = v != null },
				{ "generate-html:", v => mxap.GenerateHtml = ParseBool (v, mxap.GenerateHtml) },
				{ "include-mdb:", v => mxap.IncludeMdb = ParseBool (v, mxap.IncludeMdb) },
				{ "application-name=", v => mxap.ApplicationName = v },
				{ "generate-manifest:", v => mxap.GenerateManifest = ParseBool (v, mxap.GenerateManifest) },
				{ "use-existing-manifest", v => mxap.GenerateManifest = v == null },
				{ "entry-point-type=", v => mxap.EntryPointType = v },
				{ "cs-sources=", v => mxap.CSSources = v },
				{ "res-sources=", v => resourceFile = v },
				{ "ares-sources=", v => aresourceFile = v },
				{ "cres-sources=", v => cresourceFile = v },
				{ "desktop:", v => mxap.Desktop = ParseBool (v, mxap.Desktop) },
				{ "builddirhack=", v => mxap.TopBuildDir = v },
				{ "r=|reference=", v => mxap.ExternalAssemblies.Add (v) },
				{ "pkg=", "Use assemblies listed in .pc files to build. They won't be included in the xap, so this is only useful for extra system libraries.", v => mxap.Packages.Add (v) },
				{ "l:|list-generated:", v => mxap.ListGenerated = ParseBool (v, mxap.ListGenerated) },
				{ "v:|verbose:", v => mxap.Verbose =  ParseBool (v, mxap.Verbose) },
				{ "res=|resource=", "-res=filename[,resource name]", v => resources.Add (v) },
				{ "ares=|assembly-resource=", "-ares=filename[,resource name]", v => aresources.Add (v) },
				{ "cres=|content-resource=", "-cres=filename[,resource name]", v => cresources.Add (v) },
				{ "d:|define:", "-d:name", v => mxap.Defines.Add (v) },
				{ "clean", "Removes generated files. Use with caution!", v => clean = v != null },
				{ "out=|output-dir=", v => mxap.OutputDir = v },
				{ "inplace:", "Don't use a temporary directory", v => mxap.InPlace = ParseBool (v, mxap.InPlace) },
				{ "runtime-version=|rv=", String.Format ("Select the Silverlight Runtime Version (2 = {0}, 3 = {1} 4 = {2}, or use the full version string)", RuntimeVersion2, RuntimeVersion3, RuntimeVersion4), v => mxap.SetRuntimeVersion (v) }
			};

			List<string> extra = null;
			try {
				extra = p.Parse(args);
			} catch (OptionException){
				Console.WriteLine ("Try `mxap --help' for more information.");
				return 1;
			}

			if (help){
				ShowHelp (p);
				return 0;
			}

			if (clean) {
				DoClean (mxap.ApplicationName);
				return 0;
			}

			if (extra.Count > 0)
				mxap.Cd = Path.GetFullPath (extra [0]);

			if (mxap.TopBuildDir == null && mxap.ExternalAssemblies.Count > 0) {
				Console.Error.WriteLine ("--reference requires --builddirhack");
				return 1;
			}

			if (mxap.OutputDir == null)
				mxap.OutputDir = mxap.Cd;
			else
				mxap.OutputDir = Path.GetFullPath (mxap.OutputDir);

			Directory.SetCurrentDirectory (mxap.Cd);
			mxap.WorkingDir = mxap.Cd;
			if (mxap.Verbose)
				Console.WriteLine ("Using source directory " + mxap.WorkingDir);

			if (!mxap.InPlace) {
				mxap.TmpDir = Path.Combine (Path.GetTempPath(), Path.GetRandomFileName ());

				if (mxap.Verbose)
					Console.WriteLine ("Using temporary directory " + mxap.TmpDir);

				Directory.CreateDirectory (mxap.TmpDir);
				RecursiveCopy (mxap.WorkingDir, "", mxap.TmpDir);
			}

			if (resourceFile != null)
				foreach (string res in File.ReadAllLines (resourceFile))
					mxap.AddResource (res);

			if (aresourceFile != null)
				foreach (string res in File.ReadAllLines (aresourceFile))
					mxap.AddAssemblyResource (res);

			if (cresourceFile != null)
				foreach (string res in File.ReadAllLines (cresourceFile))
					mxap.AddContentResource (res);

			foreach (string res in resources) {
				mxap.AddResource (res);
			}

			foreach (string res in aresources) {
				mxap.AddAssemblyResource (res);
			}

			foreach (string res in cresources) {
				mxap.AddContentResource (res);
			}

			if (!mxap.InPlace) {
				Directory.SetCurrentDirectory (mxap.TmpDir);
				mxap.WorkingDir = mxap.TmpDir;
			}

			foreach (string pkg in mxap.Packages) {
				string ret;
				mxap.RunProcess ("pkg-config", "--libs " + pkg, out ret);
				if (ret != null) {
					string [] libs = ret.Trim (new Char [] {' ', '\n', '\r', '\t'}).Replace("-r:", "").Split (new Char [] { ' ', '\t'});
					mxap.PackageAssemblies.AddRange (libs);
				}
			}

			mxap.ReferenceAssemblies.AddRange (Directory.GetFiles (mxap.WorkingDir, "*.dll"));
			mxap.XamlFiles.AddRange (Directory.GetFiles (mxap.WorkingDir, "*.xaml"));
			mxap.ExternalPartManifests.AddRange (Directory.GetFiles (mxap.WorkingDir, "*.extmap.xml"));

			if (mxap.CSSources == null) {
				mxap.CSharpFiles.AddRange (Directory.GetFiles (mxap.WorkingDir, "*.cs"));
			} else {
				mxap.CSharpFiles.AddRange (File.ReadAllLines (mxap.CSSources));
			}

			if (mxap.IncludeMdb)
				mxap.MdbFiles.AddRange (Directory.GetFiles (mxap.WorkingDir, "*.mdb"));

			if (mxap.XamlFiles.Count == 0 || mxap.CSharpFiles.Count == 0) {
				Console.Error.WriteLine ("No XAML files or C# files found");
				ShowHelp (p);
				return 1;
			}

			// Make sure we didn't add the Application assembly into the referenced assemblies
			DirectoryInfo info = new DirectoryInfo (mxap.WorkingDir);

			if (mxap.ReferenceAssemblies.Contains (Path.Combine (mxap.WorkingDir, info.Name + ".dll")))
				mxap.ReferenceAssemblies.Remove (Path.Combine (mxap.WorkingDir, info.Name + ".dll"));

			if (!mxap.Run ())
				return 1;

			if (mxap.Verbose)
				Console.WriteLine ("Using Output directory " + mxap.OutputDir);

			if (!Directory.Exists (mxap.OutputDir)) {
				Console.WriteLine ("warning: output directory doesn't exist, defaulting to source directory " + mxap.Cd + " ...");
				mxap.OutputDir = mxap.Cd;
			}

			if (!mxap.InPlace || mxap.Cd != mxap.OutputDir) {
				string app = mxap.ApplicationName;
				File.Copy (Path.Combine (mxap.WorkingDir, app + ".dll"), Path.Combine (mxap.OutputDir, app + ".dll"), true);
				File.Copy (Path.Combine (mxap.WorkingDir, app + ".dll.mdb"), Path.Combine (mxap.OutputDir, app + ".dll.mdb"), true);
				File.Copy (Path.Combine (mxap.WorkingDir, app + ".g.resources"), Path.Combine (mxap.OutputDir, app + ".g.resources"), true);
				File.Copy (Path.Combine (mxap.WorkingDir, app + ".html"), Path.Combine (mxap.OutputDir, app + ".html"), true);
				File.Copy (Path.Combine (mxap.WorkingDir, app + ".xap"), Path.Combine (mxap.OutputDir, app + ".xap"), true);
				File.Copy (Path.Combine (mxap.WorkingDir, "AppManifest.xaml"), Path.Combine (mxap.OutputDir, "AppManifest.xaml"), true);
				foreach (string file in Directory.GetFiles (mxap.WorkingDir, "*.g.cs"))
					File.Copy (file, Path.Combine (mxap.OutputDir, Path.GetFileName (file)), true);

				if (!mxap.InPlace)
					Directory.Delete (mxap.TmpDir, true);
			}
			return 0;
		}
	}
}
