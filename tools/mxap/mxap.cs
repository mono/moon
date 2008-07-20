

using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.Collections.Generic;


namespace Moonlight {

	public class MXap {

		private string application_name;
		private List<string> reference_assemblies;
		private List<string> csharp_files;
		private List<string> xaml_files;

		public string ApplicationName {
			get {
				if (application_name == null) {
					DirectoryInfo di = new DirectoryInfo (Directory.GetCurrentDirectory ());
					application_name = di.Name;
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

		public void Run ()
		{
			CreateManifest ();
			CreateCodeBehind ();
			CreateResources ();
			CreateApplicationAssembly ();
			CreateXap ();
		}

		public void CreateManifest ()
		{
			StringBuilder manifest = new StringBuilder ();

			manifest.AppendLine ("<Deployment xmlns=\"http://schemas.microsoft.com/client/2007/deployment\"");
			manifest.AppendLine ("\txmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\"");
			manifest.AppendFormat ("EntryPointAssembly=\"{0}.dll\" EntryPointType=\"{0}.App\" RuntimeVersion=\"2.0.30226.2\">\n", ApplicationName);

			manifest.AppendLine ("\t<Deployment.Parts>");

			foreach (string assembly in ReferenceAssemblies) {
				manifest.AppendFormat ("<AssemblyPart x:Name=\"{0}\" Source=\"{1}\" />\n", Path.GetFileNameWithoutExtension (assembly), assembly);
			}
			manifest.AppendFormat ("<AssemblyPart x:Name=\"{0}\" Source=\"{1}.dll\" />\n", ApplicationName, ApplicationName);
			
			manifest.AppendLine ("\t</Deployment.Parts>");
			manifest.AppendLine ("</Deployment>");

			File.WriteAllText ("AppManifest.xaml", manifest.ToString ());
		}

		public void CreateCodeBehind ()
		{
			StringBuilder xamlg_args = new StringBuilder ();

			xamlg_args.AppendFormat (" -sl2app:{0} ", ApplicationName);

			foreach (string xaml_file in XamlFiles) {
				if (Path.GetFileName (xaml_file) == "AppManifest.xaml")
					continue;
				xamlg_args.AppendFormat (" {0}", xaml_file);
			}

			RunProcess ("xamlg", xamlg_args.ToString ());
		}

		public void CreateResources ()
		{
			StringBuilder respack_args = new StringBuilder ();

			respack_args.AppendFormat ("{0}.g.resources ", ApplicationName);

			foreach (string xaml_file in XamlFiles) {
				if (Path.GetFileName (xaml_file) == "AppManifest.xaml")
					continue;
				respack_args.AppendFormat (" {0}", xaml_file);
			}

			RunProcess ("respack", respack_args.ToString ());
		}

		public void CreateApplicationAssembly ()
		{
			StringBuilder smcs_args = new StringBuilder ();

			smcs_args.AppendFormat (" -pkg:silver -debug -target:library -out:{0}.dll ", ApplicationName);

			foreach (string asm in ReferenceAssemblies) {
				smcs_args.AppendFormat (" -r:{0} ", asm);
			}

			foreach (string cs in CSharpFiles) {
				if (cs.EndsWith (".g.cs"))
					continue;
				smcs_args.AppendFormat (" {0} ", cs);
			}

			foreach (string xaml in XamlFiles) {
				if (Path.GetFileName (xaml) == "AppManifest.xaml")
					continue;
				smcs_args.AppendFormat (" {0}.g.cs ", xaml);
			}

			smcs_args.AppendFormat (" -resource:{0}.g.resources ", ApplicationName);

			RunProcess ("smcs", smcs_args.ToString ());
		}

		public void CreateXap ()
		{
			StringBuilder zip_args = new StringBuilder ();

			zip_args.AppendFormat (" {0}.xap ", ApplicationName);

			foreach (string asm in ReferenceAssemblies) {
				zip_args.AppendFormat (" {0} ", Path.GetFileName (asm));
			}

			zip_args.AppendFormat (" AppManifest.xaml ");
			zip_args.AppendFormat (" {0}.dll ", ApplicationName);

			RunProcess ("zip", zip_args.ToString ());
			
		}

		private void RunProcess (string name, string args)
		{
			Process process = new Process ();

			process.StartInfo.FileName = name;
			process.StartInfo.Arguments = args;

			process.StartInfo.CreateNoWindow = true;
			process.StartInfo.UseShellExecute = false;

			process.Start ();

			process.WaitForExit ();
		}

		public static void Main ()
		{
			MXap mxap = new MXap ();

			string cd = Directory.GetCurrentDirectory ();
			mxap.ReferenceAssemblies.AddRange (Directory.GetFiles (cd, "*.dll"));
			mxap.XamlFiles.AddRange (Directory.GetFiles (cd, "*.xaml"));
			mxap.CSharpFiles.AddRange (Directory.GetFiles (cd, "*.cs"));

			// Make sure we didn't add the Application assembly into the referenced assemblies
			DirectoryInfo info = new DirectoryInfo (cd);

			if (mxap.ReferenceAssemblies.Contains (Path.Combine (cd, info.Name + ".dll")))
				mxap.ReferenceAssemblies.Remove (Path.Combine (cd, info.Name + ".dll"));

			mxap.Run ();
		}
	}
}


