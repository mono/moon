
using NDesk.Options;
using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.Collections.Generic;

namespace Moonlight {

	public class MXap {

		private string application_name;
		private List<string> external_assemblies;
		private List<string> reference_assemblies;
		private List<string> mdb_files;
		private List<string> csharp_files;
		private List<string> xaml_files;
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
		const string RuntimeVersion = "2.0.31005.0";

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

		public int Run ()
		{
			return (CreateManifest ()
				&& CreateCodeBehind ()
				&& CreateResources ()
				&& CreateApplicationAssembly ()
				&& CreateXap ()
				&& CreateHtmlWrapper ()) ? 0 : 1;
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
			manifest.AppendLine ("</Deployment>");

			File.WriteAllText (AppManifest_Filename, manifest.ToString ());

			return true;
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
				xamlg_args.AppendFormat (" -root:{0} ", Cd);

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

			compiler_args.AppendFormat (" -debug+ -target:library -out:{0}.dll ", ApplicationName);

			foreach (string asm in ReferenceAssemblies) {
				compiler_args.AppendFormat (" -r:\"{0}\" ", asm);
			}

			foreach (string asm in ExternalAssemblies) {
				compiler_args.AppendFormat (" -r:{0} ", asm);
			}

			if (desktop && top_builddir == null) {
				compiler_args.Append (" -pkg:silverdesktop ");
				compiler_args.Append (" -pkg:gtksilver ");
			}

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
				zip_args.AppendFormat (" -j " + pair.Value);

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
				if (filename == "smcs")
					need_smcs_hack = true;
				filename = "mono";
				args = String.Format ("{3}{0}/{1} {2}", top_builddir, builddir_exe, args,
						      need_smcs_hack ? "--runtime=moonlight --security=temporary-smcs-hack " : "");
			}

			return RunProcess (filename, args);
		}

		private bool RunProcess (string filename, string args)
		{
			if (Verbose)
				Console.WriteLine ("Running {0} {1}", filename, args);

			Process process = new Process ();

			process.StartInfo.FileName = filename;
			process.StartInfo.Arguments = args;

			process.StartInfo.CreateNoWindow = true;
			process.StartInfo.UseShellExecute = false;

			process.Start ();

			process.WaitForExit ();

			return (process.ExitCode == 0);
		}

		static void ShowHelp (OptionSet os)
		{
			Console.WriteLine ("mxap usage is: mxap [options] [directory]");
			Console.WriteLine ();
			os.WriteOptionDescriptions (Console.Out);
		}

		void AddResource (string v)
		{
			int comma;
			
			if (string.IsNullOrEmpty (v))
				return;

			comma = v.IndexOf (',');
			if (comma == -1) {
				Resources.Add (Path.GetFileName (v), v);
			} else {
				Resources.Add (v.Substring (comma + 1), v.Substring (0, comma)); 
			}
		}

		void ParseResource (string v, out string name, out string filename)
		{
			bool local = Path.GetFullPath (v).StartsWith (Path.GetFullPath ("."));
			int comma = v.IndexOf (',');
			if (comma == -1) {
				if (local)
					name = ApplicationName + "." + v.Replace ('/', '.');
				else
					name = ApplicationName + "." + Path.GetFileName (v);
				filename = v;
			} else {
				name = v.Substring (comma + 1);
				filename = v.Substring (0, comma);
			}
		}

		void AddAssemblyResource (string v)
		{
			if (string.IsNullOrEmpty (v))
				return;

			string name, filename;
			ParseResource (v, out name, out filename);
			AssemblyResources.Add (name, filename);
		}

		void AddContentResource (string v)
		{
			if (string.IsNullOrEmpty (v))
				return;

			string name, filename;
			ParseResource (v, out name, out filename);

			if (!Directory.Exists ("obj"))
				Directory.CreateDirectory ("obj");

			File.Copy (filename, Path.Combine ("obj", name), true);
			ContentResources.Add (name, Path.Combine ("obj", name));
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

		public static int Main (string [] args)
		{
			MXap mxap = new MXap ();
			bool help = false;
			bool clean = false;
			mxap.Cd = Directory.GetCurrentDirectory ();

			var p = new OptionSet () {
				{ "h|?|help", v => help = v != null },
				{ "generate-html", v => mxap.GenerateHtml = v != null },
				{ "include-mdb", v => mxap.IncludeMdb = v != null},
				{ "application-name=", v => mxap.ApplicationName = v },
				{ "generate-manifest", v => mxap.GenerateManifest = v != null },
				{ "entry-point-type=", v => mxap.EntryPointType = v },
				{ "cs-sources=", v => mxap.CSSources = v },
				{ "desktop", v => mxap.Desktop = v != null },
				{ "builddirhack=", v => mxap.TopBuildDir = v },
				{ "r:|reference:", v => mxap.ExternalAssemblies.Add (v) },
				{ "l|list-generated", v => mxap.ListGenerated = v != null },
				{ "v|verbose", v => mxap.Verbose =  v != null },
				{ "res|resource:", v => mxap.AddResource (v) },
				{ "ares|assembly-resource:", v => mxap.AddAssemblyResource (v) },
				{ "cres|content-resource:", v => mxap.AddContentResource (v) },
				{ "clean", "Removes generated files. Use with caution!", v => clean = v != null }
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
				mxap.Cd = extra [0];

			if (mxap.TopBuildDir == null && mxap.ExternalAssemblies.Count > 0) {
				Console.Error.WriteLine ("--reference requires --builddirhack");
				return 1;
			}

			mxap.ReferenceAssemblies.AddRange (Directory.GetFiles (mxap.Cd, "*.dll"));
			mxap.XamlFiles.AddRange (Directory.GetFiles (mxap.Cd, "*.xaml"));
			if (mxap.CSSources == null) {
				mxap.CSharpFiles.AddRange (Directory.GetFiles (mxap.Cd, "*.cs"));
			} else {
				mxap.CSharpFiles.AddRange (File.ReadAllLines (mxap.CSSources));
			}
			
			if (mxap.IncludeMdb)
				mxap.MdbFiles.AddRange (Directory.GetFiles (mxap.Cd, "*.mdb"));

			if (mxap.XamlFiles.Count == 0 || mxap.CSharpFiles.Count == 0) {
				Console.Error.WriteLine ("No XAML files or C# files found");
				ShowHelp (p);
				return 1;
			}

			// Make sure we didn't add the Application assembly into the referenced assemblies
			DirectoryInfo info = new DirectoryInfo (mxap.Cd);

			if (mxap.ReferenceAssemblies.Contains (Path.Combine (mxap.Cd, info.Name + ".dll")))
				mxap.ReferenceAssemblies.Remove (Path.Combine (mxap.Cd, info.Name + ".dll"));


			return mxap.Run ();
		}
	}
}


