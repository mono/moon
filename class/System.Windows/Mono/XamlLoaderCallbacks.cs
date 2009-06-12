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
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

using Mono;

namespace Mono.Xaml
{
	unsafe internal delegate bool LookupObjectCallback (IntPtr loader, IntPtr parser, Value* top_level, Value* parent, string xmlns, string name, [MarshalAs (UnmanagedType.U1)] bool create, [MarshalAs (UnmanagedType.U1)] bool is_property, out Value value);
	unsafe internal delegate void CreateGCHandleCallback ();

	unsafe internal delegate bool SetPropertyCallback (IntPtr loader, IntPtr parser, Value* top_level, string xmlns, Value* target, IntPtr target_data, Value* target_parent, string prop_xmlns, string name, Value* value, IntPtr value_data);
	unsafe internal delegate bool AddToContainerCallback (IntPtr loader, IntPtr parser, Value* top_level, string xmlns, string prop_name, string key_name, Value* parent, IntPtr parent_data, Value* child, IntPtr child_data);
	unsafe internal delegate bool ImportXamlNamespaceCallback (IntPtr loader, IntPtr parser, string xmlns);
	unsafe internal delegate string GetContentPropertyNameCallback (IntPtr loader, IntPtr parser, Value* object_ptr);
	
	internal struct XamlLoaderCallbacks {
		public LookupObjectCallback lookup_object;
		public CreateGCHandleCallback create_gchandle;
		public SetPropertyCallback set_property;
		public AddToContainerCallback add_to_container;
		public ImportXamlNamespaceCallback import_xaml_xmlns;
		public GetContentPropertyNameCallback get_content_property_name;
	}

	internal enum AssemblyLoadResult
	{
		Success = -1,
		MissingAssembly = 1,
		LoadFailure = 2
	}

	internal abstract class XamlLoader : MarshalByRefObject
	{
		// Contains any surface/plugins already loaded in the current domain.
		// This is required form System.Windows.XamlReader.Load to work.
		private static Surface surface_in_domain;
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
			return CreateManagedXamlLoader (typeof (DependencyObject).Assembly, surface, plugin);
		}

		public static XamlLoader CreateManagedXamlLoader (Assembly assembly, IntPtr surface, IntPtr plugin)
		{
			return new ManagedXamlLoader (assembly, surface, plugin);
		}
		
		public static int gen = 0;

		public XamlLoader ()
		{
			gen++;
		}
		
		public XamlLoader (IntPtr surface, IntPtr plugin)
		{
			gen++;
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
				
				return surface_in_domain == null ? IntPtr.Zero : surface_in_domain.Native;
			}
			set {
				if (allow_multiple_surfaces_per_domain)
					throw new ArgumentException ("Multiple surfaces per domain is enabled, so calling SurfaceInDomain/PluginInDomain is wrong.");
				
				if (value == IntPtr.Zero)
					return;
								
				if (surface_in_domain != null && surface_in_domain.Native == value)
					return;
				
				if (surface_in_domain != null) {
					Console.Error.WriteLine ("There already is a surface in this AppDomain.");
				} else {
					surface_in_domain = new Surface (value);
				}
			}
		}
		
		public static Surface SurfaceObjectInDomain {
			get { return surface_in_domain; }
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
			if (native_loader == IntPtr.Zero)
				return;
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

			try {
				CreateNativeLoader (null, xaml);
				return NativeMethods.xaml_loader_create_from_string (NativeLoader, xaml, createNamescope, out kind);
			}
			finally {
				FreeNativeLoader ();
			}
		}

		//
		// Hydrates the object dob from the given xaml
		//
		public void Hydrate (IntPtr dependency_object, string xaml)
		{
			try {
				Kind k;
				CreateNativeLoader (null, xaml);
				IntPtr ret = NativeMethods.xaml_loader_hydrate_from_string (NativeLoader, xaml, dependency_object, true, out k);
				if (ret == IntPtr.Zero)
					throw new Exception ("Invalid XAML file");
			}
			finally {
				FreeNativeLoader ();
			}
		}
		
		//
		// Creates a native object from the given filename
		// 
		public IntPtr CreateFromFile (string path, bool createNamescope, out Kind kind)
		{
			if (path == null)
				throw new ArgumentNullException ("path");

			try {
				CreateNativeLoader (null, path);
				return NativeMethods.xaml_loader_create_from_file (NativeLoader, path, createNamescope, out kind);
			}
			finally {
				FreeNativeLoader ();
			}
		}

		// 
		// Creates a managed dependency object from the xaml.
		// 
		public abstract object CreateObjectFromString (string xaml, bool createNamescope);
		public abstract object CreateObjectFromFile (string path, bool createNamescope);
	}
}
