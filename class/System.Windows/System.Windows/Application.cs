//
// Application.cs
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
using Mono;
using Mono.Xaml;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security;
using System.Windows.Controls;
using System.Windows.Resources;
using System.Windows.Interop;
using System.Collections;
using System.Collections.Generic;
using System.Resources;
using System.Windows.Markup;

namespace System.Windows {

	public partial class Application {
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
		internal static IntPtr s_surface;
		
		//
		// Application instance fields
		//
		string xap_dir;
		IntPtr surface;
		UIElement root_visual;
		SilverlightHost host;

		public Application ()
		{
			xap_dir = s_xap_dir;
			surface = s_surface;

			current = this;
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

			return CreateFromXap (plugin, surface, xapPath) != null;
		}
		
		static Application CreateFromXap (IntPtr plugin, IntPtr surface, string xapPath)
		{			

			if (plugin != IntPtr.Zero)
				PluginHost.SetPluginHandle (plugin);
			
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

			AssemblyPart entry_point_assembly = (AssemblyPart) deployment.DepObjectFindName (deployment.EntryPointAssembly);
			if (entry_point_assembly == null){
				Report.Error ("AppManifest.xaml: Could not find the referenced entry point assembly");
				return null;
			}

			if (deployment.EntryPointType == null){
				Report.Error ("No entrypoint defined in the AppManifest.xaml");
				return null;
			}

			//
			// Load the XAML to CLR object mappings from this assembly.
			//
			LoadXmlnsDefinitionMappings (typeof (Application).Assembly);
			
			//
			// Load the assemblies from the XAP file, and find the startup assembly
			//
			Assembly a;
			Assembly startup = null;
			assemblies = new Assembly [deployment.Parts.Count];
			int i = 0;

			foreach (var part in deployment.Parts){
				try {
					a = Assembly.LoadFrom (Path.Combine (xap_dir, part.Source));
					if (part.Name == entry_point_assembly.Name)
						startup = a;
					assemblies [i++] = a;
				} catch (Exception e) {
					Report.Error ("Error while loading the {0} assembly  {1}", part.Source, e);
					return null;
				}
				LoadXmlnsDefinitionMappings (a);
			}

			try {
				a = Assembly.LoadFrom (Path.Combine (xap_dir, entry_point_assembly.Source));
				startup = a;
			} catch (Exception e) {
				Report.Error ("Errror while loading startup assembly {0}  {1}", entry_point_assembly.Source, e);
				return null;
			}

			if (startup == null){
				Report.Error ("Could not find the startup assembly");
				return null;
			}

			Type entry_type = startup.GetType (deployment.EntryPointType);
			if (entry_type == null){
				Report.Error ("Could not find the startup type {0} on the {1}",
					      deployment.EntryPointType, deployment.EntryPointAssembly);
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
				instance.Startup (instance, new StartupEventArgs ());
			}
			
			if (instance.root_visual != null) {
				NativeMethods.surface_attach (instance.surface, instance.root_visual.native);
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
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public static void LoadComponent (object component, Uri xamlUri)
		{
			Application app = component as Application;
			DependencyObject cdo = component as DependencyObject;
			
			if (cdo == null && app == null)
				throw new ArgumentException ("Not a DependencyObject or Application", "component");

			StreamResourceInfo sr = GetResourceStream (xamlUri);

			// Does not seem to throw.
			if (sr == null)
				return;

			string xaml = new StreamReader (sr.Stream).ReadToEnd ();
			ManagedXamlLoader loader = new ManagedXamlLoader ();
			Assembly loading_asm = component.GetType ().Assembly;

			if (cdo != null) {
				// This can throw a System.Exception if the XAML file is invalid.
				
				loader.Hydrate (cdo.native, loading_asm.GetName ().Name, loading_asm.CodeBase, xaml);
			} else {
				ApplicationInternal temp = new ApplicationInternal ();
				loader.Hydrate (temp.native, loading_asm.GetName ().Name, loading_asm.CodeBase, xaml);

				// TODO: Copy the important stuff such as Resourcesfrom the temp DO to the app
			}
		}

		//
		// From casual documentation inspection:
		//
		//   "pathname"                     resource file embedded in application package
		//   "AssemblyName;component/pathname"   embedded in AssemblyName, the file pathname
		//
		// 
#if NET_2_1
		[SecuritySafeCritical]
#endif
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

			ResourceManager rm = new ResourceManager (aname + ".g", assembly);
			rm.IgnoreCase = true;
			Stream s = rm.GetStream (rest);
			
			if (s == null){
				Report.Info ("Could not find resource: {0}", rest);
				return null;
			}

			return new StreamResourceInfo (s, "");
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public static StreamResourceInfo GetResourceStream (StreamResourceInfo zipPackageStreamResourceInfo, Uri resourceUri)
		{
			throw new NotImplementedException ("GetResourceStream-2");
		}

		public static Application Current {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get {
				return current;
			}
		}

		public ResourceDictionary Resources {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get {
				throw new NotImplementedException ("Resources");
			}
		}

		public UIElement RootVisual {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get {
				return root_visual;
			}

#if NET_2_1
			[SecuritySafeCritical]
#endif
			set {
				// Can only be set once according to the docs.
				if (root_visual != null)
					return;
				
				root_visual = value;
			}
		}

		public SilverlightHost Host {
			get { return host ?? (host = new SilverlightHost ()); }
		}

		public event EventHandler Exit;
		public event StartupEventHandler Startup;
		public event EventHandler<ApplicationUnhandledExceptionEventArgs> UnhandledException;


		internal static Dictionary<XmlnsDefinitionAttribute,Assembly> xmlns_definitions = new Dictionary<XmlnsDefinitionAttribute, Assembly> ();
		internal static List<string> imported_namespaces = new List<string> ();
		
		internal static void LoadXmlnsDefinitionMappings (Assembly a)
		{
			object [] xmlns_defs = a.GetCustomAttributes (typeof (XmlnsDefinitionAttribute), false);

			foreach (XmlnsDefinitionAttribute ns_mapping in xmlns_defs){
				xmlns_definitions [ns_mapping] = a;
			}
		}

		
		internal static void ImportXamlNamespace (string xmlns)
		{
			imported_namespaces.Add (xmlns);
		}

		internal static Type GetComponentTypeFromName (string name)
		{
			return (from def in xmlns_definitions
				where imported_namespaces.Contains (def.Key.XmlNamespace)
				let clr_namespace = def.Key.ClrNamespace
				let assembly = def.Value
				from type in assembly.GetTypes ()
				where type.Namespace == clr_namespace && type.Name == name && type.IsSubclassOf (typeof (DependencyObject))
				select type).FirstOrDefault ();
		}

		//
		// Creates the proper component by looking the namespace and name
		// in the various assemblies loaded
		//
		internal static DependencyObject CreateComponentFromName (string name)
		{
			Type t = GetComponentTypeFromName (name);

			if (t == null)
				return null;

			return (DependencyObject) Activator.CreateInstance (t);
		}

		internal static Assembly GetAssembly (string assembly_name)
		{
			Assembly a = (from def in assemblies where def.GetName ().Name == assembly_name select def).FirstOrDefault ();
			return a;
		}
	}
}
