//
// XamlLoaderCallbacks.cs
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
	unsafe internal delegate bool LookupObjectCallback (XamlCallbackData *data, Value* parent, string xmlns, string name, [MarshalAs (UnmanagedType.U1)] bool create, [MarshalAs (UnmanagedType.U1)] bool is_property, out Value value, ref MoonError error);

	unsafe internal delegate bool SetPropertyCallback (XamlCallbackData *data, string xmlns, Value* target, IntPtr target_data, Value* target_parent, string prop_xmlns, string name, Value* value, IntPtr value_data, ref MoonError error);
	unsafe internal delegate bool ImportXamlNamespaceCallback (XamlCallbackData *data, string xmlns, ref MoonError error);
	unsafe internal delegate bool AddChildCallback (XamlCallbackData *data, Value* parent_parent, [MarshalAs (UnmanagedType.U1)] bool parent_is_property, string parent_xmlns, Value *parent, IntPtr parent_data, Value* child, IntPtr child_data, ref MoonError error);

	unsafe internal delegate IntPtr ParseTemplateFunc (Value *context, IntPtr resource_base, IntPtr surface, IntPtr binding_source, string xaml, ref MoonError error);


	internal struct XamlLoaderCallbacks {
		public IntPtr gchandle;
		public LookupObjectCallback lookup_object;
		public SetPropertyCallback set_property;
		public ImportXamlNamespaceCallback import_xaml_xmlns;
		public AddChildCallback add_child;
	}

	internal unsafe struct XamlCallbackData {
		public IntPtr loader;
		public IntPtr parser;
		public Value *top_level;
		public XamlCallbackFlags flags;
	}

	[Flags]
	internal enum XamlCallbackFlags {
		None,
		SettingDelayedProperty = 2
	}

	[Flags]
	internal enum XamlLoaderFlags {
		None,
		ValidateTemplates = 2,
		ImportDefaultXmlns = 4,
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
		
		// We keep an instance copy of the surface and plugin here,
		// since we have to support multiple surfaces for the non-browser case.
		protected IntPtr surface;
		protected IntPtr plugin;
		protected Uri resourceBase;

		public static XamlLoader CreateManagedXamlLoader (Uri resourceBase, IntPtr surface, IntPtr plugin)
		{
			return CreateManagedXamlLoader (typeof (DependencyObject).Assembly, resourceBase, surface, plugin);
		}

		public static XamlLoader CreateManagedXamlLoader (Assembly assembly, Uri resourceBase, IntPtr surface, IntPtr plugin)
		{
			return new ManagedXamlLoader (assembly, resourceBase, surface, plugin);
		}
		
		public XamlLoader ()
		{
		}
		
		public XamlLoader (Uri resourceBase, IntPtr surface, IntPtr plugin)
		{
			this.resourceBase = resourceBase;
			this.surface = surface;
			this.plugin = plugin;
		}
				
		public virtual void Setup (IntPtr native_loader, IntPtr plugin, IntPtr surface)
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
					surface_in_domain = NativeDependencyObjectHelper.FromIntPtr (value) as Surface;
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

		//
		// Creates a native object for the given xaml.
		//
		// Caller is responsible for calling value_free_value on the returned value
		public IntPtr CreateFromString (string xaml, bool createNamescope, out Kind kind)
		{
			
			return CreateFromString (xaml, createNamescope, false, false, out kind);
		}

		// Caller is responsible for calling value_free_value on the returned value
		public IntPtr CreateFromString (string xaml, bool createNamescope, bool validateTemplates, out Kind kind)
		{
			return CreateFromString (xaml, createNamescope, false, false, out kind);
		}

		// Caller is responsible for calling value_free_value on the returned value
		public IntPtr CreateFromString (string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns, out Kind kind)
		{
			return CreateFromStringInternal (xaml, createNamescope, validateTemplates, import_default_xmlns, out kind);
		}

		//
		// Hydrates the object dob from the given xaml
		//
		public void Hydrate (object value, string xaml)
		{
			Hydrate (value, xaml, true, false, false);
		}

		public void Hydrate (object value, string xaml, bool createNamescope)
		{
			Hydrate (value, xaml, createNamescope, false, false);
		}

		public void Hydrate (object value, string xaml, bool createNamescope, bool validateTemplates)
		{
			Hydrate (value, xaml, createNamescope, validateTemplates, false);
		}

		public void Hydrate (object value, string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns)
		{
			HydrateInternal (value, xaml, createNamescope, validateTemplates, import_default_xmlns);
		}

		public void Hydrate (object value, Stream stream)
		{
			HydrateInternal (value, stream, true, false, false);
		}

		public void Hydrate (object value, Stream stream, bool createNamescope, bool validateTemplates, bool import_default_xmlns)
		{
			HydrateInternal (value, stream, createNamescope, validateTemplates, import_default_xmlns);
		}

		//
		// Creates a native object from the given filename
		// 
		// Caller is responsible for calling value_free_value on the returned value
		public IntPtr CreateFromFile (string path, bool createNamescope, out Kind kind)
		{
			return CreateFromFileInternal (path, createNamescope, out kind);
		}


		protected abstract IntPtr CreateFromFileInternal (string path, bool createNamescope, out Kind kind);
		protected abstract void HydrateInternal (object value, string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns);
		protected abstract void HydrateInternal (object value, Stream xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns);
		protected abstract IntPtr CreateFromStringInternal (string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns, out Kind kind);

		// 
		// Creates a managed dependency object from the xaml.
		// 
		public abstract object CreateObjectFromString (string xaml, bool createNamescope);
		public abstract object CreateObjectFromString (string xaml, bool createNamescope, bool validateTemplates);
		public abstract object CreateObjectFromReader (StreamReader reader, bool createNamescope);
		public abstract object CreateObjectFromFile (string path, bool createNamescope);
	}

	internal sealed class SL4XamlLoader : XamlLoader {

		public SL4XamlLoader (Uri resource_base)
		{
			resourceBase = resource_base;
		}

		protected override IntPtr CreateFromFileInternal (string path, bool createNamescope, out Kind kind)
		{
			Value v = XamlParser.CreateFromFile (path, createNamescope, false);

			kind = v.k;
			return ValueToIntPtr (v);
		}

		protected override void HydrateInternal (object value, string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ValidateTemplates = validateTemplates,
				HydrateObject = value,
				ResourceBase = resourceBase,
			};

			object v = p.ParseString (xaml);
		}

		protected override void HydrateInternal (object value, Stream xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ValidateTemplates = validateTemplates,
				HydrateObject = value,
				ResourceBase = resourceBase,
			};

			using (StreamReader reader = new StreamReader (xaml)) {
				object v = p.ParseReader (reader);
			}
		}

		protected override IntPtr CreateFromStringInternal (string xaml, bool createNamescope, bool validateTemplates, bool import_default_xmlns, out Kind kind)
		{
			Value v = XamlParser.CreateFromFile (xaml, createNamescope, false);

			kind = v.k;
			return ValueToIntPtr (v);
		}

		// 
		// Creates a managed dependency object from the xaml.
		// 
		public override object CreateObjectFromString (string xaml, bool createNamescope)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ResourceBase = resourceBase,
			};

			return p.ParseString (xaml);
		}

		public override object CreateObjectFromString (string xaml, bool createNamescope, bool validateTemplates)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ValidateTemplates = validateTemplates,
				ResourceBase = resourceBase,
			};

			return p.ParseString (xaml);
		}

		public override object CreateObjectFromFile (string path, bool createNamescope)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ResourceBase = resourceBase,
			};

			return p.ParseFile (path);
		}

		public override object CreateObjectFromReader (StreamReader reader, bool createNamescope)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = createNamescope,
				ResourceBase = resourceBase,
			};

			return p.ParseReader (reader);
		}

		private IntPtr ValueToIntPtr (Value v)
		{
			IntPtr ptr = Marshal.AllocHGlobal (Marshal.SizeOf (v));

			Marshal.StructureToPtr (v, ptr, false);
			return ptr;
		}
	}
}
