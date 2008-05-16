//
// Application.cs
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
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
using Mono;
using Mono.Xaml;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows.Resources;
using System.Windows.Interop;
using System.Collections;
using System.Collections.Generic;
using System.Resources;

namespace System.Windows {

	public class Application {
		static Application current;
		static Assembly [] assemblies;
		
		//
		// Controls access to the s_ static fields, which are used
		// by the Application constructor to initialize these fields
		// and any other fields that require initialization by passing
		// data before the derived class executes.
		//
		static object creation = new object ();
		static string s_xap_dir;
		static IntPtr s_surface;
		
		//
		// Application instance fields
		//
		string xap_dir;
		IntPtr surface;
		DependencyObject root_visual;
		
		public Application ()
		{
			xap_dir = s_xap_dir;
			surface = s_surface;
		}

		internal void Terminate ()
		{
			if (xap_dir == null)
				return;

			if (Exit != null)
				Exit (this, EventArgs.Empty);
			
			try {
				Helper.DeleteDirectory (xap_dir);
				xap_dir = null;
			} catch {
			}
		}				

		/// <summary>
		///    Initializes the Application singleton by creating the Application from the XAP file
		/// </summary>
		/// <remarks>
		///   This is consumed by the plugin, should not be used by user code.
		/// </remarks>
		internal static bool LaunchFromXap (IntPtr plugin, IntPtr surface, string xapPath)
		{
			if (current != null)
				throw new Exception ("Should only be called once per AppDomain");

			current = CreateFromXap (plugin, surface, xapPath);

			return current != null;
		}
		
		static Application CreateFromXap (IntPtr plugin, IntPtr surface, string xapPath)
		{			
			if (plugin != IntPtr.Zero){
				PluginHost.SetPluginHandle (plugin);

				// Why this?   I have no idea, am copying from the XamlLoader code
				// I think this should be gone, and we should only use
				// PluginHost

				AppDomain.CurrentDomain.SetData ("PluginInstance", plugin);
			}
			
			string xap_dir = NativeMethods.xap_unpack (xapPath);
			if (xap_dir == null){
				Report.Error ("Failure to unpack {0}", xapPath);
				return null;
			}

			//
			// Load AppManifest.xaml, validate a bunch of properties
			//
			ManagedXamlLoader loader = new ManagedXamlLoader (surface, PluginHost.Handle);
			string app_manifest = Path.Combine (xap_dir, "AppManifest.xaml");
			if (!File.Exists (app_manifest)){
				Report.Error ("No AppManifest.xaml found on the XAP package");
				return null;
			}
			
			object result = loader.CreateDependencyObjectFromFile (app_manifest, true);
			if (result == null){
				Report.Error ("Invalid AppManifest.xaml file");
				return null;
			}

			Deployment deployment = (Deployment) result;

			if (deployment.EntryPointAssembly == null){
				Report.Error ("AppManifest.xaml: No EntryPointAssebly found");
				return null;
			}

			DependencyObject entry_point_assembly = deployment.FindName (deployment.EntryPointAssembly);
			if (entry_point_assembly == null){
				Report.Error ("AppManifest.xaml: Could not find the referenced entry point assembly");
				return null;
			}

			if (deployment.EntryPointType == null){
				Report.Error ("No entrypoint defined in the AppManifest.xaml");
				return null;
			}

			//
			// Load the assemblies from the XAP file, and find the startup assembly
			//
			Assembly startup = null;
			assemblies = new Assembly [deployment.Parts.Count];
			int i = 0;
				
			foreach (var part in deployment.Parts){
				try {
					Assembly a = Assembly.LoadFrom (Path.Combine (xap_dir, part.Source));
					if (part == entry_point_assembly)
						startup = a;
					assemblies [i++] = a;
				} catch {
					Report.Error ("Error while loading the {0} assembly", part.Source);
					return null;
				}
			}

			if (startup == null){
				Report.Error ("Could not find the startup assembly");
				return null;
			}

			Type entry_type = startup.GetType (deployment.EntryPointType);
			if (entry_type == null){
				Report.Error ("Could not find the startup type {0} on the {1} assembly", entry_type, deployment.EntryPointAssembly);
				return null;
			}

			if (!entry_type.IsSubclassOf (typeof (Application))){
				Report.Error ("Startup type does not derive from System.Windows.Application");
				return null;
			}

			Application instance;

			lock (creation){
				s_xap_dir = xap_dir;
				s_surface = surface;
				
				try {
					instance = (Application) Activator.CreateInstance (entry_type);
				} catch (Exception e){
					Report.Error ("Error while creating the instance: {0}", e);
					return null;
				}
			}

			// TODO:
			// Get the event args to pass to startup
			
			if (instance.Startup != null){
				StartupEventArgs sargs = new StartupEventArgs ();

				Report.Warning ("TODO: Need to pass correct StartupEventArgs");
				instance.Startup (instance, sargs);
			}
			
			return instance;
		}
		
		static Application CreateFromXap (string xapPath)
		{
			return CreateFromXap (IntPtr.Zero, IntPtr.Zero, xapPath);
		}
		
		//
		// component is used as the target type of the object
		// we are loading, makes no sense to me, sounds like a
		// hack.
		//
		public static void LoadComponent (object component, Uri xamlUri)
		{
			//Console.WriteLine ("LoadComponent: {0} of type {1} for {2}", component, component.GetType (), xamlUri);

			// For now, do nothing, we cant cope with Applications 
			if (component is Application)
				return;

			DependencyObject cdo = component as DependencyObject;
			
			if (cdo == null)
				throw new ArgumentException ("Not a DependencyObject or Application", "component");

			StreamResourceInfo sr = GetResourceStream (xamlUri);

			// Does not seem to throw.
			if (sr == null)
				return;

			string xaml = new StreamReader (sr.Stream).ReadToEnd ();
			ManagedXamlLoader loader = new ManagedXamlLoader ();
			loader.Hydrate (cdo.native, xaml);
		}

		//
		// From casual documentation inspection:
		//
		//   "pathname"                     resource file embedded in application package
		//   "AssemblyName;component/pathname"   embedded in AssemblyName, the file pathname
		//
		// 
		public static StreamResourceInfo GetResourceStream (Uri resourceUri)
		{
			string loc = resourceUri.ToString ();
			int p = loc.IndexOf (';');

			if (p == -1)
				return StreamResourceInfo.FromFile (Path.Combine (Current.xap_dir, loc));

			string aname = loc.Substring (0, p);

			// For now, strip this, I have no idea what this means.
			if (aname [0] == '/')
				aname = aname.Substring (1);

			Assembly assembly = assemblies.FirstOrDefault (a => a.GetName ().Name == aname);

			if (assembly == null){
				Report.Info ("Could not find the named assembly in the resourceUri");
				return null;
			}

			string rest = loc.Substring (p+1);
			if (!rest.StartsWith ("component/")){
				Report.Info ("Second component does not inclue component/");
				return null;
			}
			rest = rest.Substring (10);

			//Console.WriteLine ("Before RM, rest={0}", rest);
			//Console.WriteLine ("Requesting assembly: " + aname + ".g");
			ResourceManager rm = new ResourceManager (aname + ".g", assembly);
			rm.IgnoreCase = true;
			Stream s = rm.GetStream (rest);
			
			if (s == null){
				Report.Info ("Could not find resource: {0}", rest);
				return null;
			}
			
			return new StreamResourceInfo (s, "");
		}

		public static StreamResourceInfo GetResourceStream (StreamResourceInfo zipPakResourceStreamInfo, Uri resourceUri)
		{
			throw new NotImplementedException ("GetResourceStream-2");
		}

		public static Application Current {
			get {
				return current;
			}
		}

		public ResourceDictionary Resources {
			get {
				throw new NotImplementedException ("Resources");
			}
		}

		public DependencyObject RootVisual {
			get {
				return root_visual;
			}

			set {
				//
				// Must be a UIElement, and can only be set once
				//
				if (!(value is UIElement))
					throw new InvalidOperationException ();

				// Can only be set once according to the docs.
				if (root_visual != null)
					return;
				
				root_visual = value;

				NativeMethods.surface_attach (surface, root_visual.native);
			}
		}

		public event EventHandler Exit;
		public event StartupEventHandler Startup;
		public event EventHandler<ApplicationUnhandledExceptionEventArgs> UnhandledException;

	}
}
