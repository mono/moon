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
			NativeMethods.deployment_set_current_application (Current.native, application.NativeHandle);
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

		internal bool ExtractXap (string xapPath) {
			xap_dir = NativeMethods.xap_unpack (xapPath);
			if (xap_dir == null){
				Report.Error ("Failure to unpack {0}", xapPath);
				return false;
			}
			return true;
		}

		internal bool ReadManifest () {
			XamlLoader loader = XamlLoader.CreateManagedXamlLoader (Surface.Native, PluginHost.Handle);
			string app_manifest = Path.Combine (XapDir, "AppManifest.xaml");

			if (!File.Exists (app_manifest)){
				Report.Error ("No AppManifest.xaml found on the XAP package");
				return false;
			}

			string app_manifest_contents;

			using (StreamReader r = new StreamReader (app_manifest))
				app_manifest_contents = r.ReadToEnd();

			try {
				loader.Hydrate (native, app_manifest_contents);
			}
			catch (Exception e) {
				Report.Error (e.ToString());
				return false;
			}

			if (EntryPointAssembly == null) {
				Report.Error ("AppManifest.xaml: No EntryPointAssembly found");
				return false;
			}

			if (EntryPointType == null) {
				Report.Error ("No entrypoint defined in the AppManifest.xaml");
				return false;
			}
			return true;
		}

		internal bool InitializeDeployment () {
			EntryPointType = "System.Windows.Application";
			EntryPointAssembly = typeof (Application).Assembly.GetName ().Name;
			EntryAssembly = typeof (Application).Assembly;
			return LoadAssemblies ();
		}
			
		internal bool InitializeDeployment (IntPtr plugin, string xapPath) {
			InitializePluginHost (plugin);
			if (!ExtractXap (xapPath))
				return false;
			if (!ReadManifest ())
				return false;

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
					client.OpenReadCompleted += new OpenReadCompletedEventHandler (OnOpenReadCompleted);
					client.OpenReadAsync (new Uri (PluginHost.RootUri, new Uri (part.Source))); 
					delay_load = true;
				} else {
					try {
						var asm = Assembly.LoadFrom (Path.Combine (XapDir, part.Source));

						assemblies.Add (asm);

						if (EntryAssembly == null && asm.GetName ().Name == EntryPointAssembly)
							EntryAssembly = asm;

					} catch (Exception e) {
						Report.Error ("Error while loading the {0} assembly  {1}", part.Source, e);
						return false;
					}
				}
			}

			if (delay_load)
				return true;

			return CreateApplication ();
		}

		// stop using an anonymous method since its name keeps changing and this code is [SecuritySafeCritical]
		internal void OnOpenReadCompleted (object sender, OpenReadCompletedEventArgs e) 
		{
			AssemblyPart a = new AssemblyPart ();
			Assembly asm = a.Load (e.Result);
						
			if (EntryAssembly == null && asm.GetName ().Name == EntryPointAssembly)
				EntryAssembly = asm;

			Deployment d = Deployment.Current;
			if (d.Assemblies.Count == Parts.Count + 1)
				d.CreateApplication ();
		}

		internal bool CreateApplication () {
			if (EntryAssembly == null) {
				Report.Error ("Could not find the entry point assembly");
				return false;
			}

			Type entry_type = EntryAssembly.GetType (EntryPointType);
			if (entry_type == null){
				Report.Error ("Could not find the startup type {0} on the {1}",
					      EntryPointType, EntryPointAssembly);
				return false;
			}

			if (!entry_type.IsSubclassOf (typeof (Application)) && entry_type != typeof (Application)){
				Report.Error ("Startup type does not derive from System.Windows.Application");
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
				return false;
			}
			
			foreach (Assembly a in Assemblies)
				Application.LoadXmlnsDefinitionMappings (a);
			
			Application instance = null;

			try {
				instance = (Application) Activator.CreateInstance (entry_type);
			} catch (Exception e){
				Report.Error ("Error while creating the instance: {0}", e);
				return false;
			}

			instance.OnStartup ();

			Events.InitSurface (Surface.Native);
			SetCurrentApplication (instance);
			NativeMethods.event_object_unref (instance.NativeHandle);

			return true;
		}
	}
}
