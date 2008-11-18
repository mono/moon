//
// ManagedXamlLoader.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System.Collections;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Markup;
using System.Runtime.InteropServices;
using Mono;

namespace Mono.Xaml
{
	internal class ManagedXamlLoader : XamlLoader {
		List<GCHandle> createdHandles = new List<GCHandle>();

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
			callbacks.add_child = new AddChildCallback (cb_add_child);
			callbacks.hookup_event = new HookupEventCallback (cb_hookup_event);
			callbacks.insert_mapping = new InsertMappingCallback (cb_insert_mapping);
			callbacks.get_mapping = new GetMappingCallback (cb_get_mapping);
			callbacks.load_code = new LoadCodeCallback (cb_load_code);
			callbacks.set_name_attribute = new SetNameAttributeCallback (cb_set_name_attribute);
			callbacks.import_xaml_xmlns = new ImportXamlNamespaceCallback (cb_import_xaml_xmlns);
			callbacks.create_component_from_name = new CreateComponentFromNameCallback (cb_create_component_from_name);
			callbacks.get_content_property_name = new GetContentPropertyNameCallback (cb_get_content_property_name);

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
			
			DependencyObject.Initialize ();
			
			top = CreateFromString (xaml, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = DependencyObject.Lookup (kind, top);
			
			if (result != null) {
				// Delete our reference, result already has one.
				NativeMethods.event_object_unref (top);
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
			
			DependencyObject.Initialize ();

			top = CreateFromFile (file, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = DependencyObject.Lookup (kind, top);
			
			if (result != null) {
				// Delete our reference, result already has one.
				NativeMethods.event_object_unref (top);
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
			string value;
			if (!mappings.TryGetValue (key, out value))
				return null;
			return value;
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
		public AssemblyLoadResult LoadAssembly (string asm_name, out Assembly clientlib)
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

		private IntPtr LoadObject (string asm_name, string asm_path, string ns, string type_name, out bool is_dependency_object)
		{
			AssemblyLoadResult load_result;
			Assembly clientlib = null;
			string name;

			is_dependency_object = false;

			if (asm_name == null)
				throw new ArgumentNullException ("asm_name");

			if (type_name == null)
				throw new ArgumentNullException ("type_name");
			
			load_result = LoadAssembly (asm_name, out clientlib);
			
			if (load_result != AssemblyLoadResult.Success)
				return IntPtr.Zero;

			if (clientlib == null) {
				Console.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}): Assembly loaded, but where is it?", asm_name, ns, type_name);
				return IntPtr.Zero;
			}
			
			if (ns == null || ns == string.Empty)
				name = type_name;
			else
				name = String.Concat (ns, ".", type_name);

			object res = null;
			try {
				Console.WriteLine ("creating instance of:   {0}  {1}", name, asm_name);
				res = clientlib.CreateInstance (name);
			}
			catch (TargetInvocationException ex) {
				Console.WriteLine ("ManagedXamlLoader::LoadObject: CreateInstance ({0}) failed: {1}", name, ex.InnerException);
				return IntPtr.Zero;
			}
			if (res == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}): unable to create object instance: '{3}', the object was of type '{4}'", asm_name, ns, type_name, name, res.GetType ().FullName);
				return IntPtr.Zero;
			}

			DependencyObject dob = res as DependencyObject;
			if (dob == null) {
				// it's a non-DO object.  create the
				// GCHandle and return the GCHandle.
				GCHandle handle = GCHandle.Alloc (res);
				createdHandles.Add(handle);
				return Helper.GCHandleToIntPtr (handle);
			}
			else {
				is_dependency_object = true;
				NativeMethods.event_object_ref (dob.native);
				return dob.native;
			}
		}
		
		private object LookupObject (IntPtr target_ptr)
		{
			if (target_ptr == IntPtr.Zero)
				return  null;

			try {
				GCHandle handle = Helper.GCHandleFromIntPtr (target_ptr);
				return handle.Target;
			}
			catch {
				Kind k = NativeMethods.dependency_object_get_object_type (target_ptr); 
				return DependencyObject.Lookup (k, target_ptr);
			}
		}

		private bool SetCustomAttribute (IntPtr target_ptr, string xmlns, string name, string value)
		{
			object o = LookupObject (target_ptr);

			if (o == null)
				return false;

			DependencyObject target = o as DependencyObject;
			if (target != null) {
				PropertyInfo pi;
				string error;
				IntPtr unmanaged_value;

				if (target == null) {
					//Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}, {3}): unable to create target object.", target_ptr, xmlns, name, value);
					return false;
				}

				//
				// We might be dealing with an attached property
				//
				int dot = name.IndexOf ('.');
				if (dot > 0) {
					Console.WriteLine ("setting attached managed property {0}", name);
					string asm = AssemblyNameFromXmlns (xmlns);
					string ns = ClrNamespaceFromXmlns (xmlns);
					string type = String.Concat (ns, ".", name.Substring (0, dot));
					name = name.Substring (++dot, name.Length - dot);

					Assembly clientlib;
					if (LoadAssembly (asm, out clientlib) != AssemblyLoadResult.Success) {
						Console.WriteLine ("couldn't load assembly");
						return false;
					}
				
					Type attach_type = clientlib.GetType (type, false);
					if (attach_type == null) {
						Console.WriteLine ("attach type is null  {0}", type);
						return false;
					}

					MethodInfo set_method = attach_type.GetMethod (String.Concat ("Set", name), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
					if (set_method == null) {
						Console.WriteLine ("set method is null: {0}", String.Concat ("Set", name));
						return false;
					}

					ParameterInfo [] set_params = set_method.GetParameters ();
					if (set_params == null || set_params.Length < 2) {
						Console.WriteLine ("set method signature is inccorrect.");
						return false;
					}

					object o_value = Helper.ValueFromString (set_params [1].ParameterType, value, name, out error, out unmanaged_value);
					if (error == null && unmanaged_value != IntPtr.Zero) {
						o_value = DependencyObject.ValueToObject (null, unmanaged_value);
					}

					set_method.Invoke (null, new object [] {target, o_value});
					return true;
				}

				pi = target.GetType ().GetProperty (name);

				if (pi == null){
					Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}, {3}, {4}) no property descriptor found.", target_ptr, target.GetType (), xmlns, name, value);
					return false;
				}

				Helper.SetPropertyFromString (target, pi, value, out error, out unmanaged_value);

				if (error == null && unmanaged_value != IntPtr.Zero) {
					object obj_value = DependencyObject.ValueToObject (null, unmanaged_value);
					Helper.SetPropertyFromValue (target, pi, obj_value, out error);
				}
			
				if (error != null) {
					Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}, {3}, {4}) unable to set property: {5}.", target_ptr, target.GetType(), xmlns, name, value, error);
					// XXX ignore these for now
					// return false;
					return true;
				}

				return true;
			}
			else {
				string error;
				PropertyInfo pi;
				IntPtr unmanaged_value;

				pi = o.GetType ().GetProperty (name);

				try {
					object o_value = Helper.ValueFromString (pi.PropertyType, value, name, out error, out unmanaged_value);

					pi.SetValue (o, o_value, null);
				}
				catch (Exception e) {
					Console.Error.WriteLine (e);
					return false;
				}

				return true;
			}
		}

		private bool AddChild (IntPtr parent_ptr, string prop_name, IntPtr child_ptr)
		{
			string[] prop_path = prop_name.Split ('.');
			if (prop_path.Length < 2)
				return false;

			object parent = LookupObject (parent_ptr);
			object child = LookupObject (child_ptr);

			if (parent == null) {
				Console.WriteLine ("parent is null");
				return false;
			}

			if (child == null) {
				Console.WriteLine ("child is null");
				return false;
			}

			FieldInfo fi = parent.GetType ().GetField (prop_path[1] + "Property", BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);
			if (fi != null) {
				DependencyProperty dp = fi.GetValue (parent) as DependencyProperty;
				if (dp == null)
					return false;

				if (dp.IsAttached) {
					if (typeof (IList).IsAssignableFrom (dp.PropertyType)) {
						// it's a collection type
						MethodInfo mi = parent.GetType ().GetMethod ("Get" + prop_path[1], BindingFlags.Static | BindingFlags.Public);
						object o = mi.Invoke (parent, new object[] { child });
						if (o is IList) {
							((IList)o).Add (child);
							return true;
						}
						else {
							Console.Error.WriteLine ("unable to add child to attached property");
							return false;
						}
					}
					else {
						Console.Error.WriteLine ("unfinished - we need attached non-collection properties");
					}
				}
				else {
					DependencyObject parent_depobj = parent as DependencyObject;
					if (parent == null) {
						Console.Error.WriteLine ("we're adding a child to a non-DO parent?  nuh uh!");
						return false;
					}

					if (typeof (IList).IsAssignableFrom (dp.PropertyType)) {
						object col = parent_depobj.GetValue(dp);
						if (col == null) {
							Console.WriteLine ("Creating instant of {0} collection", dp.PropertyType);
							// automatically create the collection if it doesn't exist
							col = Activator.CreateInstance (dp.PropertyType);
							parent_depobj.SetValue (dp, col);
						}

						// XXX do we check the child
						// type vs the collection
						// type?  or do we catch the
						// exception?
						IList l = col as IList;
						if (l != null)
							l.Add (child);
					}
					else {
						parent_depobj.SetValue (dp, child);
					}

					return true;
				}
			}

			return false;
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

		private static string ClrNamespaceFromXmlns (string xmlns)
		{
			int start = xmlns.IndexOf ("clr-namespace:") + "clr-namespace:".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
		}

		private static string AssemblyNameFromXmlns (string xmlns)
		{
			int start = xmlns.IndexOf ("assembly=") + "assembly=".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
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
		private IntPtr cb_load_object (string asm_name, string asm_path, string ns, string type_name, out bool is_dependency_object)
		{
			is_dependency_object = false;
			try {
				return LoadObject (asm_name, asm_path, ns, type_name, out is_dependency_object);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}, {3}) failed: {4} ({5}).", asm_name, asm_path, ns, type_name, ex.Message, ex.GetType ().FullName);
				return IntPtr.Zero;
			}
		}
		
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// generating an exception and unwinding the stack.
		//
		private bool cb_set_custom_attribute (IntPtr target_ptr, string xmlns, string name, string value)
		{
			try {
				return SetCustomAttribute (target_ptr, xmlns, name, value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::SetCustomAttribute ({0}, {1}, {2}) threw an exception: {3}.", target_ptr, name, value, ex.Message);
				return false;
			}
		}

		private bool cb_add_child (IntPtr parent_ptr, string prop_name, IntPtr child_ptr)
		{
			try {
				return AddChild (parent_ptr, prop_name, child_ptr);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::AddChild ({0}, {1}, {2}) threw an exception: {3}.",
							 parent_ptr, prop_name, child_ptr, ex.Message);
				return false;
			}
		}

		private bool cb_hookup_event (IntPtr target_ptr, IntPtr dest_ptr, string name, string value)
		{
			DependencyObject target = DependencyObject.Lookup (target_ptr);
			DependencyObject dest = DependencyObject.Lookup (dest_ptr);

			if (target == null) {
				Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create target object.", target_ptr, name, value);
				return false;
			}


			EventInfo src = target.GetType ().GetEvent (name);
			if (src == null) {
				Console.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to find event name.", target, name, value);
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
				Delegate d = Delegate.CreateDelegate (src.EventHandlerType, dest, value);
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
					Console.Error.WriteLine ("Application::CreateComponentFromName ({0}) returned null.", name);
					return IntPtr.Zero;
				}

				NativeMethods.event_object_ref (res.native);
				return res.native;
			} catch (Exception ex) {
				Console.WriteLine ("Application::CreateComponentFromName ({0}) threw an exception:\n{1}", name, ex);
				return IntPtr.Zero;
			}
		}

		private string cb_get_content_property_name (IntPtr dob_ptr)
		{
			DependencyObject dob = DependencyObject.Lookup (dob_ptr);
			if (dob == null)
				return null;

			Type t = dob.GetType ();
			object [] o = t.GetCustomAttributes (typeof (ContentPropertyAttribute), true);
			if (o.Length == 0)
				return null;
			ContentPropertyAttribute cpa = (ContentPropertyAttribute ) o [0];

			return cpa.Name;
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
