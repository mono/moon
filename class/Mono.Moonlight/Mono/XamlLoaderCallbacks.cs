//
// ManagedXamlLoader.cs
//
// Authors:
//   Rolf Bjarne Kvinge (RKvinge@novell.com)
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
using Mono;

namespace Mono.Xaml
{
	public delegate IntPtr LoadObjectCallback (string asm_name, string asm_path, string ns, string type_name);
	public delegate bool SetAttributeCallback (IntPtr target, string name, string value);
	public delegate bool HookupEventCallback (IntPtr target, string name, string value);
	public delegate void InsertMappingCallback (string key, string value);
	public delegate string GetMappingCallback (string key);
	public delegate bool LoadCodeCallback (string source, string type);
	public delegate void SetNameAttributeCallback (IntPtr target, string name);
	
	public struct XamlLoaderCallbacks {
		public LoadObjectCallback load_managed_object;
		public SetAttributeCallback set_custom_attribute;
		public HookupEventCallback hookup_event;
		public GetMappingCallback get_mapping;
		public InsertMappingCallback insert_mapping;
		public LoadCodeCallback load_code;
		public SetNameAttributeCallback set_name_attribute;
	}

	public enum AssemblyLoadResult
	{
		Success = -1,
		MissingAssembly = 1,
		LoadFailure = 2
	}

#if !NET_2_1
	public
#else
	internal
#endif
	abstract class XamlLoader : MarshalByRefObject
	{
		// Contains any surface/plugins already loaded in the current domain.
		// This is required form System.Windows.XamlReader.Load to work.
		private static IntPtr surface_in_domain;
		private static IntPtr plugin_in_domain;
		private static bool allow_multiple_surfaces_per_domain;
		
		protected IntPtr native_loader;
		private bool load_deps_synch = false;
		
		// We keep an instance copy of the surface and plugin here,
		// since we have to support multiple surfaces for the non-browser case.
		protected IntPtr surface;
		protected IntPtr plugin;
		
		public static XamlLoader CreateManagedXamlLoader (IntPtr surface, IntPtr plugin)
		{
			System.Reflection.Assembly assembly = Helper.GetAgclr ();
			XamlLoader loader = (XamlLoader) Helper.CreateInstance (assembly.GetType ("Mono.Xaml.ManagedXamlLoader"), true);
			loader.surface = surface;
			loader.plugin = plugin;
			return loader;
		}
		
		public XamlLoader ()
		{
		}
		
		public XamlLoader (IntPtr surface, IntPtr plugin)
		{
			this.surface = surface;
			this.plugin = plugin;
		}
				
		public virtual void Setup (IntPtr native_loader, IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			this.native_loader = native_loader;
			this.plugin = plugin;
			this.surface = surface;
		}
				
		public IntPtr PluginHandle {
			get {
				return plugin;
			}
		}
		
		public IntPtr NativeLoader {
			get {
				return native_loader;
			}
		}
		
		public IntPtr Surface {
			get {
				return surface;
			}
		}
		
		//
		// Set whenever the loader will load dependencies synchronously using the browser
		// This is used in cases where the user of the loader can't operate in async mode
		// such as Control:InitializeFromXaml ()
		//
		public bool LoadDepsSynch {
			get {
				return load_deps_synch;
			}
			set {
				load_deps_synch = value;
			}
		}
		
		public static bool AllowMultipleSurfacesPerDomain {
			get {
				
				return allow_multiple_surfaces_per_domain;
			}
			set {
				allow_multiple_surfaces_per_domain = value;
			}
		}
		
		public static IntPtr SurfaceInDomain {
			get {
				if (allow_multiple_surfaces_per_domain)
					throw new ArgumentException ("Multiple surfaces per domain is enabled, so calling SurfaceInDomain/PluginInDomain is wrong.");
				
				return surface_in_domain;
			}
			set {
				if (allow_multiple_surfaces_per_domain)
					throw new ArgumentException ("Multiple surfaces per domain is enabled, so calling SurfaceInDomain/PluginInDomain is wrong.");
				
				if (value == IntPtr.Zero || (value == surface_in_domain && value != IntPtr.Zero))
					return;
				
				if (surface_in_domain != IntPtr.Zero) {
					Console.Error.WriteLine ("There already is a surface in this AppDomain.");
				} else {
					surface_in_domain = value;
				}
			}
		}
		
		public static IntPtr PluginInDomain {
			get {
				if (allow_multiple_surfaces_per_domain)
					throw new ArgumentException ("Multiple surfaces per domain is enabled, so calling SurfaceInDomain/PluginInDomain is wrong.");
				
				return plugin_in_domain;
			}
			set {
				if (allow_multiple_surfaces_per_domain)
					throw new ArgumentException ("Multiple surfaces per domain is enabled, so calling SurfaceInDomain/PluginInDomain is wrong.");
				
				if (value == IntPtr.Zero || (value == plugin_in_domain && value != IntPtr.Zero))
					return;
				
				if (plugin_in_domain != IntPtr.Zero) {
					Console.Error.WriteLine ("There already is a plugin in this AppDomain.");
				} else {
					plugin_in_domain = value;
				}
			}			 
		}
		
		public void CreateNativeLoader (string filename, string contents)
		{
			if (!AllowMultipleSurfacesPerDomain) {
				if (surface == IntPtr.Zero)
					surface = SurfaceInDomain;
				if (plugin == IntPtr.Zero)
					plugin = PluginInDomain;
			}
			
			if (surface == IntPtr.Zero)
				throw new Exception ("The surface where the xaml should be loaded is not set.");
			
			//Console.WriteLine ("ManagedXamlLoader::CreateNativeLoader (): surface: {0}", surface);
			native_loader = NativeMethods.xaml_loader_new (filename, contents, surface);
			
			if (native_loader == IntPtr.Zero)
				throw new Exception ("Unable to create native loader.");
			
			Setup (native_loader, plugin, surface, filename, contents);
		}
		
		public void FreeNativeLoader ()
		{
			NativeMethods.xaml_loader_free (native_loader);
			native_loader = IntPtr.Zero;
		}
		
		//
		// Creates a native object for the given xaml.
		// 
		public IntPtr CreateFromString (string xaml, bool createNamescope, out Kind kind)
		{
			if (xaml == null)
				throw new ArgumentNullException ("xaml");

			IntPtr top;
			
			CreateNativeLoader (null, xaml);
			top = NativeMethods.xaml_create_from_str (NativeLoader, xaml, createNamescope, out kind);
			FreeNativeLoader ();
			
			return top;
		}

		// 
		// Creates a managed dependency object from the xaml.
		// Must always return a DependencyObject (since we don't reference agclr, we can't 
		// declare the return type as DependencyObject)
		// 
		public abstract object CreateDependencyObjectFromString (string xaml, bool createNamescope);
		
		public object InitializeFromXaml (string xaml, IntPtr native)
		{
			if (xaml == null)
				throw new ArgumentNullException ("xaml");
			
			Kind kind;
			IntPtr native_child;
			
			LoadDepsSynch = true;
			CreateNativeLoader (null, xaml);
			native_child = NativeMethods.control_initialize_from_xaml_callbacks (native, xaml,
											  out kind, NativeLoader);
			FreeNativeLoader ();
		
			if (native_child == IntPtr.Zero)
				// FIXME: Add detail
				throw new Exception ();
		
			return Helper.LookupDependencyObject (kind, native_child);
		}
		
		public object InitializeFromXaml (string xaml, object dependency_object)
		{
			return InitializeFromXaml (xaml, Helper.GetNativeObject (dependency_object));
		}
	}
}
