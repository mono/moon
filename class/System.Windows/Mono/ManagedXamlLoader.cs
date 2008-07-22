//
// ManagedXamlLoader.cs
//
// Authors:
//   Rolf Bjarne Kvinge (RKvinge@novell.com)
//   Miguel de Icaza (miguel@ximian.com)
//
// Copyright 2007 Novell, Inc.
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
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using Mono;

namespace Mono.Xaml
{
	internal class ManagedXamlLoader : XamlLoader {
		//
		// Maps the assembly path to the location where the browser stored
		// the downloaded file on its cache (ClientBin/Dingus.dll -> /home/xxx/.mozilla/cache/...)
		// 
		static Dictionary<string, string> mappings = new Dictionary <string, string> ();

		//
		// Maps a loaded assembly to the originating path
		//
		internal static Dictionary<Assembly, string> assembly_to_source = new Dictionary<Assembly, string>();
		
		XamlLoaderCallbacks callbacks;

#if WITH_DLR
		DLRHost dlr_host;
#endif

		
		public ManagedXamlLoader ()
		{
		}
		
		public ManagedXamlLoader (IntPtr surface, IntPtr plugin) : base (surface, plugin)
		{
		}
		
		public override void Setup (IntPtr native_loader, IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			base.Setup (native_loader, plugin, surface, filename, contents);
			
			//
			// Registers callbacks that are invoked from the
			// unmanaged code. 
			//
			callbacks.load_managed_object = new LoadObjectCallback (cb_load_object);
			callbacks.set_custom_attribute = new SetAttributeCallback (cb_set_custom_attribute);
			callbacks.hookup_event = new HookupEventCallback (cb_hookup_event);
			callbacks.insert_mapping = new InsertMappingCallback (cb_insert_mapping);
			callbacks.get_mapping = new GetMappingCallback (cb_get_mapping);
			callbacks.load_code = new LoadCodeCallback (cb_load_code);
			callbacks.set_name_attribute = new SetNameAttributeCallback (cb_set_name_attribute);
			callbacks.import_xaml_xmlns = new ImportXamlNamespaceCallback (cb_import_xaml_xmlns);
			callbacks.create_component_from_name = new CreateComponentFromNameCallback (cb_create_component_from_name);

			NativeMethods.xaml_loader_set_callbacks (native_loader, callbacks);
			
			if (plugin != IntPtr.Zero)
				System.Windows.Interop.PluginHost.SetPluginHandle (plugin);

			if (!AllowMultipleSurfacesPerDomain) {
				PluginInDomain = plugin;
				SurfaceInDomain = surface;
			}
			
			//
			// Sets default handler for loading assemblies, this allows
			// code to call Assemly.Load ("relative") and trigger a
			// download (Phalanger.NET first exposed this). 
			//

//			AppDomain.CurrentDomain.AssemblyResolve += AssemblyResolver;
		}
		
		// 
		// Creates a managed dependency object from the xaml.
		// Must always return a DependencyObject (the abstract declaration in agmono
		// cannot be declared with a return value of type DependencyObject since agmono
		// can't reference agclr, it would cause a circular dependency).
		// 
		public override object CreateDependencyObjectFromString (string xaml, bool createNamescope)
		{
			if (xaml == null)
				throw new ArgumentNullException ("xaml");

			IntPtr top;
			DependencyObject result;
			Kind kind;
			
			DependencyObject.Ping ();
			
			top = CreateFromString (xaml, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = DependencyObject.Lookup (kind, top);
			
			if (result != null) {
				// Delete our reference, result already has one.
				NativeMethods.base_unref (top);
			}
			
			return result;
		}

		// 
		// Creates a managed dependency object from the xaml in the file
		// Must always return a DependencyObject (the abstract declaration in agmono
		// cannot be declared with a return value of type DependencyObject since agmono
		// can't reference agclr, it would cause a circular dependency).
		// 
		public override object CreateDependencyObjectFromFile (string file, bool createNamescope)
		{
			if (file == null)
				throw new ArgumentNullException ("file");

			IntPtr top;
			DependencyObject result;
			Kind kind;
			
			DependencyObject.Ping ();

			top = CreateFromFile (file, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = DependencyObject.Lookup (kind, top);
			
			if (result != null) {
				// Delete our reference, result already has one.
				NativeMethods.base_unref (top);
			}
			
			return result;
		}
		
		//
		// Our assembly resolver, invoked by Assembly.Load if it fails
		// to find an assembly locally.    This happens if the managed code
		// tries to call Assembly.Load ("file.dll") as it expects the
		// runtime to download the file.dll that is relative to the assembly
		// that invoked Assembly.Load
		//
		internal Assembly AssemblyResolver (object sender, ResolveEventArgs args)
		{
			AssemblyName an = new AssemblyName (args.Name);
			Assembly result = null;

			//
			// Find the caller that called Load, to use that as the base for 
			// lookig up assemblies
			//

			// Skip agclr (this frame)
			StackFrame [] frames = new StackTrace (1).GetFrames ();
			Assembly mscorlib = typeof (object).Assembly;
			Assembly caller = null;
			foreach (StackFrame f in frames){
				MethodBase m = f.GetMethod ();
				Assembly mass = m.DeclaringType.Assembly;
				if (mass != mscorlib){
					caller = m.DeclaringType.Assembly;
					break;
				}
			}

			if (caller == null){
				Console.WriteLine ("ManagedXamlLoader.AssemblyResolver: failed to find caller assembly to mscorlib");
				return null;
			}

			// We always do sync loads from here
			string path = null;
			foreach (KeyValuePair<Assembly,string> pair in assembly_to_source){
				Console.WriteLine ("ManagedXamlLoader::AssemblyResolver: {0} {1}", pair.Key, pair.Value);
				if (pair.Key != caller)
					continue;
				
				// Found it, lookup the base dir
				foreach (KeyValuePair<string,string> ab in mappings){
					if (ab.Value == pair.Value){
						string apath = ab.Key;
						if (apath.IndexOf ('/') == -1)
							path = an.Name;
						else {
							path = apath.Substring (0, apath.LastIndexOf ('/')) + "/" + an.Name + ".dll";
						}
					}
				}
			}
			if (path == null){
				Console.WriteLine ("ManagedXamlLoader::AssemblyResolver: Never found the calling assembly and its path");
				return null;
			}

			//AssemblyLoadResult r =
			LoadAssemblyPlugin (path, path, true, ref result);
			return result;
		}
			
		public static void AssemblyToSource (Assembly k, string path)
		{
			if (assembly_to_source.ContainsKey (k))
				return;
			
			assembly_to_source.Add (k, path);
		}
		
		public string GetMapping (string key)
		{
			if (!mappings.ContainsKey (key))
				return null;
			return mappings [key];
		}

		public void InsertMapping (string key, string name)
		{
			//Console.WriteLine ("ManagedXamlLoader::InsertMapping ({0}, {1}).", key, name);
			if (mappings.ContainsKey (key)){
				Console.Error.WriteLine ("ManagedXamlLoader::InsertMapping ({0}, {1}): Inserting a duplicate key? (current value: {2}).", key, name, mappings [key]);
				return;
			}
			
			mappings [key] = name;
		}
		
		public void RequestFile (string asm_path)
		{
			Console.WriteLine ("ManagedXamlLoader::RequestFile ({0}).", asm_path);
			NativeMethods.xaml_loader_add_missing (native_loader, asm_path);
		}

		//
		// Tries to load the assembly.
		// Requests any referenced assemblies if necessary.
		//
		public AssemblyLoadResult LoadAssembly (string asm_path, string asm_name, out Assembly clientlib)
		{
			clientlib = null;

			clientlib = Application.GetAssembly (asm_name);
			return clientlib != null ? AssemblyLoadResult.Success : AssemblyLoadResult.MissingAssembly;
		}

		//
		// Tries to load an assembly, this is called only from the plugin
		// 
		AssemblyLoadResult LoadAssemblyPlugin (string asm_path, string asm_name, bool synchronous, ref Assembly clientlib)
		{
			if (!synchronous) {
				DependencyLoader dl = new DependencyLoader (this, asm_path);
				
				return dl.Load (ref clientlib);
			}
			
			//
			// FIXME: Move this support into the DependencyLoader
			//
			Console.WriteLine ("WARNING: ManagedXamlLoader Sync Assembly Loader has not been ported to use DependencyLoader");
			byte[] arr = LoadDependency (asm_path);
			if (arr != null)
				clientlib = Assembly.Load (arr);
			if (clientlib == null) {
				//Console.WriteLine ("ManagedXamlLoader::LoadAssembly (asm_path={0} asm_name={1}): could not load client library: {2}", asm_path, asm_name, asm_path);
				return AssemblyLoadResult.LoadFailure;
			}
			AssemblyToSource (clientlib, asm_path);
			return AssemblyLoadResult.Success;
		}
		
		private IntPtr LoadObject (string asm_name, string asm_path, string ns, string type_name)
		{
			AssemblyLoadResult load_result;
			Assembly clientlib = null;
			string name;
			
			if (asm_name == null)
				throw new ArgumentNullException ("asm_name");
			
			if (asm_path == null)
				throw new ArgumentNullException ("asm_path");
						
			if (type_name == null)
				throw new ArgumentNullException ("type_name");
			
			load_result = LoadAssembly (asm_path, asm_name, out clientlib);
			
			if (load_result != AssemblyLoadResult.Success)
				return IntPtr.Zero;

			if (clientlib == null) {
				Console.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}, {3}): Assembly loaded, but where is it?", asm_name, asm_path, ns, type_name);
				return IntPtr.Zero;
			}
			
			if (ns == null || ns == string.Empty)
				name = type_name;
			else
				name = String.Concat (ns, ".", type_name);

			object res = null;
			try {
				res = clientlib.CreateInstance (name);
			}
			catch (TargetInvocationException ex) {
				Console.WriteLine ("ManagedXamlLoader::LoadObject: CreateInstance ({0}) failed: {1}", name, ex.InnerException);
				return IntPtr.Zero;
			}
			DependencyObject dob = res as DependencyObject;

			if (dob == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}, {3}): unable to create object instance: '{4}', the object was of type '{5}'", asm_name, asm_path, ns, type_name, name, res.GetType ().FullName);
				return IntPtr.Zero;
			}

			NativeMethods.base_ref (dob.native);

			return dob.native;
		}
		
		private bool SetCustomAttribute (IntPtr target_ptr, string name, string value)
		{
			Kind k = NativeMethods.dependency_object_get_object_type (target_ptr); 
			DependencyObject target = DependencyObject.Lookup (k, target_ptr);

			if (target == null) {
				//Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}): unable to create target object.", target_ptr, name, value);
				return false;
			}

			string error;
			IntPtr unmanaged_value;
			Helper.SetPropertyFromString (target, name, value, out error, out unmanaged_value);

			if (unmanaged_value != IntPtr.Zero) {
				object obj_value = DependencyObject.ValueToObject (unmanaged_value);

				error = null;
				Helper.SetPropertyFromValue (target, name, obj_value, out error);
			}

			if (error != null){
				//Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}) unable to set property: {3}.", target_ptr, name, value, error);
				return false;
			}

			return true;
		}

		private bool LoadCode (string source, string type)
		{
			Console.WriteLine ("ManagedXamlLoader::LoadCode: '" + source + "' '" + type + "'");

			if (source == null) {
				Console.WriteLine ("ManagedXamlLoader::LoadCode: ERROR: Source can't be null.");
				return false;
			}

			if (type == null) {
				Console.WriteLine ("ManagedXamlLoader::LoadCode: ERROR: Type can't be null.");
				return false;
			}

			//
			// First try the desktop case
			//
			Stream s = Moonlight.LoadResource (source);
			if (s != null) {
#if WITH_DLR
				if (dlr_host == null)
					dlr_host = new DLRHost ();
				dlr_host.LoadSource (s, type, mappings);
				SetGlobalsAndEvents ();
#else
				Console.WriteLine ("ManagedXamlLoader::LoadCode: Ignoring request to load code.");
#endif
				return true;
			}

			//
			// Then the browser case
			//
			string local = GetMapping (source);
			if (local != null) {
				s = new FileStream (local, FileMode.Open, FileAccess.Read);
#if WITH_DLR
				if (dlr_host == null)
				    dlr_host = new DLRHost ();
				try {
					dlr_host.LoadSource (s, type, mappings);
				} catch (MissingReferenceException ex) {
					RequestFile (ex.Reference);
					return false;
				} catch (Exception ex) {
					if (Events.IsPlugin ()) {
						Events.ReportException (ex);
						// This is not a fatal error
						return true;
					}
					else
						throw;
				}
				SetGlobalsAndEvents ();
#else
				Console.WriteLine ("ManagedXamlLoader::LoadCode: Ignoring request to load code.");
#endif
				return true;
			} else {
				RequestFile (source);

				return false;
			}
		}

		private void SetNameAttribute (IntPtr target_ptr, string name)
		{
#if WITH_DLR
			Kind k = NativeMethods.dependency_object_get_object_type (target_ptr); 
			DependencyObject target = DependencyObject.Lookup (k, target_ptr);

			if (dlr_host != null) {
				dlr_host.SetVariable (name, target);
			} else {
				RememberName (target, name);
			}
#endif
		}

#if WITH_DLR
		private class RememberedEvent {
			public DependencyObject target;
			public string name;
			public string value;
		}

		private Dictionary<string, DependencyObject> named_objects;

		private Dictionary<RememberedEvent, RememberedEvent> events;

		//
		// When SetNameAttribute and hookup_event are called, scripts are not yet loaded.
		// So we store the info in hash tables and process them after a script has been
		// loaded
		private void RememberName (DependencyObject target, string name) {
			if (named_objects == null)
				named_objects = new Dictionary<string, DependencyObject> ();
			named_objects [name] = target;
		}

		private void RememberEvent (DependencyObject target, string name, string value) {
			if (events == null)
				events = new Dictionary<RememberedEvent, RememberedEvent> ();
			RememberedEvent ev = new RememberedEvent ();
			ev.target = target;
			ev.name = name;
			ev.value = value;
			events [ev] = ev;
		}

		private void SetGlobalsAndEvents () {
			if (named_objects != null) {
				foreach (KeyValuePair<string, DependencyObject> kvp in named_objects)
					dlr_host.SetVariable (kvp.Key, kvp.Value);
				named_objects = null;
			}
			if (events != null) {
				foreach (RememberedEvent ev in events.Keys) {
					dlr_host.HookupEvent (ev.target, ev.name, ev.value);
				}
			}
		}
#endif

		//
		// Load a dependency file synchronously using the plugin
		//
		private byte[] LoadDependency (string path)
		{
			IntPtr plugin_handle = System.Windows.Interop.PluginHost.Handle;
			if (plugin_handle == IntPtr.Zero)
				return null;

			// FIXME: Cache result
			int size = 0;
			IntPtr n = NativeMethods.plugin_instance_load_url (plugin_handle, path, ref size);
			byte[] arr = new byte [size];
			unsafe {
				using (Stream u = new SimpleUnmanagedMemoryStream ((byte *) n, (int) size)){
					u.Read (arr, 0, size);
				}
			}
			Helper.FreeHGlobal (n);

			return arr;;
		}

		///
		///
		/// Callbacks invoked by the xaml.cpp C++ parser
		///
		///

#region Callbacks from xaml.cpp

		private string cb_get_mapping (string key)
		{
			try {
				return GetMapping (key);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::GetMapping ({0}) failed: {1}.", key, ex.Message);
				return null;
			}
		}
		
		private void cb_insert_mapping (string key, string name)
		{
			try {
				InsertMapping (key, name);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::InsertMapping ({0}, {1}) failed: {2}.", key, name, ex.Message);
			}
		}
		
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// genereting an exception and unwinding the stack.
		//
		private IntPtr cb_load_object (string asm_name, string asm_path, string ns, string type_name)
		{
			try {
				return LoadObject (asm_name, asm_path, ns, type_name);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}, {3}) failed: {4} ({5}).", asm_name, asm_path, ns, type_name, ex.Message, ex.GetType ().FullName);
				return IntPtr.Zero;
			}
		}
		
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// genreating an exception and unwinding the stack.
		//
		private bool cb_set_custom_attribute (IntPtr target_ptr, string name, string value)
		{
			try {
				return SetCustomAttribute (target_ptr, name, value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}) threw an exception: {3}.", target_ptr, name, value, ex.Message);
				return false;
			}
		}
		
		private bool cb_hookup_event (IntPtr target_ptr, string name, string value)
		{
			Kind k = NativeMethods.dependency_object_get_object_type (target_ptr);
			DependencyObject target = DependencyObject.Lookup (k, target_ptr);

			if (target == null) {
				//Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create target object.", target_ptr, name, value);
				return false;
			}

			EventInfo src = target.GetType ().GetEvent (name);
			if (src == null) {
				//Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to find event name.", target_ptr, name, value);
				return false;
			}

#if WITH_DLR
			if (dlr_host != null) {
				try {
					dlr_host.HookupEvent (target, name, value);
				}
				catch (Exception ex) {
					Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to hookup event: {3}", target_ptr, name, value, ex);
					return false;
				}
			} else {
				RememberEvent (target, name, value);
			}
#endif

			try {
				Delegate d = Delegate.CreateDelegate (src.EventHandlerType, target, value);
				if (d == null) {
					//Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create delegate (src={3} target={4}).", target_ptr, name, value, src.EventHandlerType, target);
					return false;
				}

				src.AddEventHandler (target, d);
				return true;
			}
			catch {
				return false;
			}
		}

		private bool cb_load_code (string source, string type)
		{
			try {
				return LoadCode (source, type);
			}
			catch (Exception ex) {
				Console.WriteLine ("ManagedXamlLoader::LoadCode ({0}, {1}): {2}", source, type, ex);
				return false;
			}
		}

		private void cb_set_name_attribute (IntPtr target_ptr, string name)
		{
			try {
				SetNameAttribute (target_ptr, name);
			}
			catch (Exception ex) {
				Console.WriteLine ("ManagedXamlLoader::SetNameAttribute () threw an exception: " + ex);
			}
		}

		private void cb_import_xaml_xmlns (string xmlns)
		{
			try {
				Application.ImportXamlNamespace (xmlns);
			} catch (Exception ex) {
				Console.WriteLine ("Application::ImportXamlNamespace ({0}) threw an exception:\n{1}", xmlns, ex);
			}

		}

		private IntPtr cb_create_component_from_name (string name)
		{
			try {
				DependencyObject res = Application.CreateComponentFromName (name);
				if (res == null) {
					Console.Error.WriteLine ("Application::CreateComponentFromName ({0}) return null.", name);
					return IntPtr.Zero;
				}

				NativeMethods.base_ref (res.native);
				return res.native;
			} catch (Exception ex) {
				Console.WriteLine ("Application::CreateComponentFromName ({0}) threw an exception:\n{1}", name, ex);
				return IntPtr.Zero;
			}
		}

#endregion
	}

	//
	// This class is a helper routine used to load an assembly and
	// its dependencies;  If a dependnecy is missing, it requests that
	// it be downloaded, if the dependency can not be met, it errors out.
	//
	// Handles recursive and multiple requests
	//
	internal class DependencyLoader {
		Assembly main;
		ManagedXamlLoader loader;
		int missing;
		string dirname;
		string asm_path;
		
		//
		// The hashtable is indexed by assembly name, and can contain:
		//   * An Assembly (for resolved references).
		//   * An AssemblyName (for unresolved references).
		//
		Dictionary<string, object> deps;

		public DependencyLoader (ManagedXamlLoader parent, string asm_path)
		{
			loader = parent;
			this.asm_path = asm_path;
			
			dirname = ""; 
			int p = asm_path.LastIndexOf ('/');
			if (p != -1)
				dirname = asm_path.Substring (0, p + 1);
		}
		
		//
		// Loads the entry point
		//
		AssemblyLoadResult LoadMain ()
		{
			string mapped = loader.GetMapping (asm_path);
			if (mapped == null){
				loader.RequestFile (asm_path);
				return AssemblyLoadResult.MissingAssembly;
			}
			
			main = Helper.LoadFile (mapped);
			if (main == null){
				Console.WriteLine ("DependencyLoader: LoadMain (\"{0}\") failed to load client library", asm_path);
				return AssemblyLoadResult.LoadFailure;
			}
			ManagedXamlLoader.AssemblyToSource (main, mapped);
			return AssemblyLoadResult.Success;
		}
		
		void UpdateDeps (Assembly a)
		{
			AssemblyName [] list = Helper.GetReferencedAssemblies (a);
			
			foreach (AssemblyName an in list){
				if (an.Name == "System.Windows" || an.Name == "mscorlib" ||
					an.Name == "System.Xml" || an.Name == "System" ||
					an.Name == "Microsoft.Scripting" ||
					an.Name == "System.Windows.Browser" ||
					an.Name == "System.Net" ||
					an.Name == "System.Core")
					continue;
				
				if (deps == null) {
					deps = new Dictionary<string, object> ();
					deps [a.GetName ().Name] = a;
				}
				if (deps.ContainsKey (an.Name))
					continue;
				deps [an.Name] = an;
				missing++;
			}
		}

		Assembly LoadDependency (string key, string file)
		{
			try {
				Assembly a = Helper.LoadFile (file);

				// enter it into the loaded deps
				deps [key] = a;
				ManagedXamlLoader.AssemblyToSource (a, file);
				missing--;

				return a;
			} catch (Exception ex){
				Console.WriteLine ("DependencyLoader::LoadDependency, error loading file {0}: {1}", file, ex.Message);
				throw;
			}
		}

		//
		// Triggers the assembly loading
		//
		public AssemblyLoadResult Load (ref Assembly clientlib)
		{
			AssemblyLoadResult result;
			
			// Load the initial assembly
			if (main == null){
				result = LoadMain ();
				if (result != AssemblyLoadResult.Success)
					return result;
				
				UpdateDeps (main);
				
				if (missing == 0){
					clientlib = main;
					return AssemblyLoadResult.Success;
				}
			}
			
			while (missing > 0){
				foreach (string name in deps.Keys){
					object v = deps [name];

					// If its already loaded, ignore
					if (v is Assembly)
						continue;
					
					AssemblyName an = (AssemblyName) v;
					string request = dirname + an.Name + ".dll";
					string mapped = loader.GetMapping (request);
					
					// Try first to detect a downloaded dependency
					if (mapped == null){
						loader.RequestFile (request);
						return AssemblyLoadResult.MissingAssembly;
					}

					try {
						Assembly a = LoadDependency (name, mapped);
						UpdateDeps (a);
						break;
					} catch {
						return AssemblyLoadResult.LoadFailure;
					}
				}
			}

			clientlib = main;
			return AssemblyLoadResult.Success;
		}
	}
}
