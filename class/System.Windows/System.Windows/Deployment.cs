//
// Deployment.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.IO;
using System.Net;
using System.Reflection;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Interop;
using Mono;
using Mono.Xaml;

namespace System.Windows {

	public sealed partial class Deployment : DependencyObject {
		Types types;
		List<Assembly> assemblies;
		List<Assembly> delay_assemblies;
		Assembly entry_point_assembly;
		string xap_dir;

		public static Deployment Current {
			get {
				IntPtr dep = NativeMethods.deployment_get_current ();
				return NativeDependencyObjectHelper.Lookup (Kind.DEPLOYMENT, dep) as Deployment;
			}

			internal set {
				NativeMethods.deployment_set_current (value == null ? IntPtr.Zero : value.native);
			}
		}
	
		public static void RegisterAssembly (Assembly assembly)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void SetCurrentApplication (Application application)
		{
			NativeMethods.deployment_set_current_application (Current.native, application == null ? IntPtr.Zero : application.NativeHandle);
		}

		internal Types Types {
			get {
				if (types == null)
					types = new Types (NativeMethods.deployment_get_types (native));
				return types;
			}
		}

		internal List<Assembly> Assemblies {
			get {
				return assemblies;
			}
		}

		internal string XapDir {
			get {
				return xap_dir;
			}
			set {
				xap_dir = value;
			}
		}
		
		internal Assembly EntryAssembly {
			get {
				return entry_point_assembly;
			}
			set {
				entry_point_assembly = value;
			}
		}
		
		internal void InitializePluginHost (IntPtr plugin) {
			if (plugin != IntPtr.Zero)
				PluginHost.SetPluginHandle (plugin);
		}

		internal void ExtractXap (string xapPath) {
			xap_dir = NativeMethods.xap_unpack (xapPath);
			if (xap_dir == null)
				throw new MoonException (2103, "Invalid or malformed application: Check manifest");
		}

		void CompareRuntimeVersions ()
		{
			int[] versions = new int[4];
			string[] version_strings = RuntimeVersion.Split ('.');

			for (int i = 0; i < version_strings.Length; i ++) {
				if (!Int32.TryParse (version_strings[i], out versions[i]))
					throw new MoonException (2105, "invalid RuntimeVersion");
			}

			if (versions[0] == 2) {
				/* SL2 accepts anything newer than 2.0.30524.0 */
				if (version_strings.Length > 1)
					if (versions[1] != 0)
						throw new MoonException (2106, "Failed to load the application. It was built with an obsolete version of Silverlight");

				if (version_strings.Length > 2)
					if (versions[2] < 30524)
						throw new MoonException (2106, "Failed to load the application. It was built with an obsolete version of Silverlight");
			}
			else if (versions[0] == 3) {
				// we don't actually validate any 3.0
				// runtime versions yet since there's
				// no telling what it'll be at RTM.
			}
			else {
				throw new MoonException (2105, "invalid RuntimeVersion");
			}
		}

		internal void ReadManifest () {
			XamlLoader loader = XamlLoader.CreateManagedXamlLoader (Surface.Native, PluginHost.Handle);
			string app_manifest = Path.Combine (XapDir, "AppManifest.xaml");

			if (!File.Exists (app_manifest))
				throw new MoonException(2103, "Invalid or malformed application: Check manifest");

			string app_manifest_contents;

			using (StreamReader r = new StreamReader (app_manifest))
				app_manifest_contents = r.ReadToEnd();

			try {
				loader.Hydrate (native, app_manifest_contents);
			}
			catch (Exception e) {
				throw new MoonException (7016, e.Message);
			}

			if (RuntimeVersion == null)
				throw new MoonException (2105, "No RuntimeVersion specified in AppManifest");

			CompareRuntimeVersions ();

			if (EntryPointAssembly == null)
				throw new MoonException (2103, "Invalid or malformed application: Check manifest");

			if (EntryPointType == null)
				throw new Exception ("No entrypoint defined in the AppManifest.xaml");
		}

		internal bool InitializeDeployment () {
			TerminateCurrentApplication ();
			EntryPointType = "System.Windows.Application";
			EntryPointAssembly = typeof (Application).Assembly.GetName ().Name;
			EntryAssembly = typeof (Application).Assembly;
			return LoadAssemblies ();
		}
			
		internal bool InitializeDeployment (IntPtr plugin, string xapPath) {
			TerminateCurrentApplication ();
			InitializePluginHost (plugin);
			ExtractXap (xapPath);
			ReadManifest ();

			NativeMethods.deployment_set_is_loaded_from_xap (native, true);
			return LoadAssemblies ();
		}

		internal bool LoadAssemblies () {
			assemblies = new List <Assembly> ();
			assemblies.Add (typeof (Application).Assembly);
			
			bool delay_load = false;

			for (int i = 0; Parts != null && i < Parts.Count; i++) {
				var part = Parts [i];

				if (part.Source[0] == '/') {
					WebClient client = new WebClient ();
					client.OpenReadCompleted += delegate (object sender, OpenReadCompletedEventArgs e) {
						if (e.Cancelled || (e.Error != null))
							return;

						AssemblyPart a = new AssemblyPart ();
						Assembly asm = a.Load (e.Result);
						
						SetEntryAssembly (asm);

						Deployment d = Deployment.Current;
						if (d.Assemblies.Count == Parts.Count + 1)
							d.CreateApplication ();
					};
					client.OpenReadAsync (new Uri (part.Source, UriKind.Relative));
					delay_load = true;
				} else {
					Assembly asm = LoadXapAssembly (part.Source);
					assemblies.Add (asm);
					SetEntryAssembly (asm);
				}
			}

			if (delay_load)
				return true;

			return CreateApplication ();
		}

		Assembly LoadXapAssembly (string name)
		{
			string filename = Path.GetFullPath (Path.Combine (XapDir, name));
			// note: the content of the AssemblyManifest.xaml file is untrusted
			if (!filename.StartsWith (XapDir))
				throw new MoonException (2105, string.Format ("Trying to load the assembly '{0}' outside the XAP directory.", name));

			try {
				return Assembly.LoadFrom (filename);
			} catch (Exception e) {
				throw new MoonException (2105, string.Format ("Error while loading the '{0}' assembly : {1}", name, e.Message));
			}
		}

		// extracted since Assembly.GetName is security critical
		void SetEntryAssembly (Assembly asm)
		{
			if (asm.GetName ().Name == EntryPointAssembly)
				EntryAssembly = asm;
		}

		void TerminateCurrentApplication ()
		{
			if (Application.Current != null) {
				Application.Current.Terminate ();
	                        NativeMethods.surface_attach (Deployment.Current.Surface.Native, IntPtr.Zero);
			}
		}

		internal bool CreateApplication () {
			SetCurrentApplication (null);

			if (EntryAssembly == null)
				throw new Exception ("Could not find the entry point assembly");

			Type entry_type = EntryAssembly.GetType (EntryPointType);
			if (entry_type == null)
				throw new MoonException (2103, string.Format ("Could not find the startup type {0} on the {1}",
									      EntryPointType, EntryPointAssembly));

			if (!entry_type.IsSubclassOf (typeof (Application)) && entry_type != typeof (Application)) {
#if SANITY
				Type t = entry_type;
				int spacing = 0;
				while (t != null) {
					if (spacing > 0) {
						for (int i = 0; i < spacing; i ++)
							Console.Write (" ");
						Console.Write ("+ ");
					}
					Console.WriteLine ("{0}", t);
					spacing += 2;
					t = t.BaseType;
				}
#endif

				throw new MoonException (2103, "Startup type does not derive from System.Windows.Application");
			}
			
			foreach (Assembly a in Assemblies)
				Application.LoadXmlnsDefinitionMappings (a);

			Application instance = null;

			try {
				instance = (Application) Activator.CreateInstance (entry_type);
			} catch (Exception e) {
				throw new MoonException (2103, string.Format ("Error while creating the instance of type {0}", entry_type));
			}

			SetCurrentApplication (instance);

			Events.InitSurface (Surface.Native);

			instance.OnStartup ();

			return true;
		}
	}
}
