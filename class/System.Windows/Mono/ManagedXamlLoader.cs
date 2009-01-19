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
using System.ComponentModel;
using Mono;

namespace Mono.Xaml
{
	internal class ManagedXamlLoader : XamlLoader {

		Assembly assembly;
		XamlLoaderCallbacks callbacks;
		GCHandle handle;

		static Dictionary<string,string> rename_type_map = new Dictionary<string,string> () {
			{ "System.Windows.Application", "System.Windows.ApplicationInternal" }
		};

		public ManagedXamlLoader ()
		{
		}

		public ManagedXamlLoader (IntPtr surface, IntPtr plugin) : this (null, surface, plugin)
		{
		}

		public ManagedXamlLoader (Assembly assembly, IntPtr surface, IntPtr plugin) : base (surface, plugin)
		{
			this.assembly = assembly;
		}
		
		public override void Setup (IntPtr native_loader, IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			base.Setup (native_loader, plugin, surface, filename, contents);

			//
			// Registers callbacks that are invoked from the
			// unmanaged code. 
			//
			callbacks.lookup_object = new LookupObjectCallback (cb_lookup_object);
			callbacks.create_gchandle = new CreateGCHandleCallback (cb_create_gchandle);
			callbacks.set_property = new SetPropertyCallback (cb_set_property);
			callbacks.import_xaml_xmlns = new ImportXamlNamespaceCallback (cb_import_xaml_xmlns);
			callbacks.get_content_property_name = new GetContentPropertyNameCallback (cb_get_content_property_name);

			NativeMethods.xaml_loader_set_callbacks (native_loader, callbacks);
			
			if (plugin != IntPtr.Zero)
				System.Windows.Interop.PluginHost.SetPluginHandle (plugin);

			if (!AllowMultipleSurfacesPerDomain) {
				PluginInDomain = plugin;
				SurfaceInDomain = surface;
			}
		}
		
		// 
		// Creates a managed dependency object from the xaml.
		// 
		public override DependencyObject CreateDependencyObjectFromString (string xaml, bool createNamescope)
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
		// 
		public override DependencyObject CreateDependencyObjectFromFile (string file, bool createNamescope)
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
		// Tries to load the assembly.
		//
		public AssemblyLoadResult LoadAssembly (string asm_name, out Assembly clientlib)
		{
			clientlib = null;

			clientlib = Application.GetAssembly (asm_name);
			return clientlib != null ? AssemblyLoadResult.Success : AssemblyLoadResult.MissingAssembly;
		}

		private bool TryGetDefaultAssemblyName (IntPtr top_level, out string assembly_name)
		{
			object obj = LookupObject (top_level);

			if (obj == null) {
				if (assembly == null) {
					assembly_name = null;
					return false;
				}

				assembly_name = assembly.GetName ().Name;
				return true;
			}

			assembly_name = obj.GetType ().Assembly.GetName ().Name;
			return true;
		}

		private bool LookupObject (IntPtr top_level, string xmlns, string name, bool create, out Value value)
		{
			if (name == null)
				throw new ArgumentNullException ("type_name");

			if (top_level == IntPtr.Zero && xmlns == null)
				return LookupComponentFromName (top_level, name, create, out value);
			
			Assembly clientlib = null;
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);

			if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
				Console.Error.WriteLine ("Unable to find an assembly to load type from.");
				value = Value.Empty;
				return false;
			}

			if (LoadAssembly (assembly_name, out clientlib) != AssemblyLoadResult.Success) {
				Console.Error.WriteLine ("Unable to load assembly");
				value = Value.Empty;
				return false;
			}

			if (clientlib == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}): Assembly loaded, but where is it?", assembly_name, xmlns, name);
				value = Value.Empty;
				return false;
			}

			string full_name = string.IsNullOrEmpty (clr_namespace) ? name : clr_namespace + "." + name;

			string mapped_name;
			if (rename_type_map.TryGetValue (full_name, out mapped_name))
				full_name = mapped_name;

			Type type = clientlib.GetType (full_name);
			if (type == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject: GetType ({0}) failed.", name);
				value = Value.Empty;
				return false;
			}

			if (create) {
				object res = null;
				try {
					res = Activator.CreateInstance (type);
				} catch (TargetInvocationException ex) {
					Console.Error.WriteLine ("ManagedXamlLoader::LoadObject: CreateInstance ({0}) failed: {1}", name, ex.InnerException);
					value = Value.Empty;
					return false;
				}

				if (res == null) {
					Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}): unable to create object instance: '{3}', the object was of type '{4}'",
							assembly_name, xmlns, name, full_name, res.GetType ().FullName);
					value = Value.Empty;
					return false;
				}
				value = Value.FromObject (res, true);
			} else {
				value = Value.Empty;
				value.k = Types.TypeToNativeKind (type);
			}

			return true;
		}

		private bool LookupComponentFromName (IntPtr top_level, string name, bool create, out Value value)
		{
			if (!create) {
				Type type = Application.GetComponentTypeFromName (name);
				if (type == null) {
					value = Value.Empty;
					return false;
				}
				value = Value.Empty;
				value.k = Types.TypeToNativeKind (type);
				return true;
			}

			object obj = Application.CreateComponentFromName (name);
			if (obj == null) {
				value = Value.Empty;
				return false;
			}

			value = Value.FromObject (obj, true);
			return true;
		}

		private static object LookupObject (IntPtr target_ptr)
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

		private static bool IsAttachedProperty (string name)
		{
			return name.IndexOf ('.') > 0;
		}

		private bool TrySetAttachedProperty (IntPtr top_level, string xmlns, IntPtr target_ptr, string type_name, string name, IntPtr value_ptr)
		{
			if (type_name == null)
				return false;

			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string ns = ClrNamespaceFromXmlns (xmlns);

			if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
				Console.Error.WriteLine ("Unable to find an assembly to load type from.");
				return false;
			}

			object target = LookupObject (target_ptr);

			if (target == null) {
				Console.Error.WriteLine ("target is null");
				return false;
			}

			Assembly clientlib;
			if (LoadAssembly (assembly_name, out clientlib) != AssemblyLoadResult.Success) {
				Console.Error.WriteLine ("couldn't load assembly:  {0}   namespace:  {1}", assembly_name, ns);
				return false;
			}

			Type attach_type = clientlib.GetType (type_name, false);
			if (attach_type == null) {
				attach_type = Application.GetComponentTypeFromName (type_name);
				if (attach_type == null) {
					Console.Error.WriteLine ("attach type is null  {0}", type_name);
					return false;
				}
			}

			MethodInfo set_method = attach_type.GetMethod (String.Concat ("Set", name), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
			if (set_method == null) {
				Console.Error.WriteLine ("set method is null: {0}", String.Concat ("Set", name));
				return false;
			}

			ParameterInfo [] set_params = set_method.GetParameters ();
			if (set_params == null || set_params.Length < 2) {
				Console.Error.WriteLine ("set method signature is inccorrect.");
				return false;
			}

			string error = null;
			IntPtr unmanaged_value = IntPtr.Zero;

			object o_value = Value.ToObject (null, value_ptr);
			if (error == null && unmanaged_value != IntPtr.Zero)
				o_value = Value.ToObject (null, unmanaged_value);

			//
			// The Setter might actually want a collection, in this case we grab the old collection with the getter
			// and then add the new object to the collection
			//
			// TODO: Check if the setter method still gets called on Silverlight
			if (typeof (IList).IsAssignableFrom (set_params [1].ParameterType) && !(o_value is IList)) {
				MethodInfo get_method = attach_type.GetMethod (String.Concat ("Get", name), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);

				if (get_method != null || get_method.GetParameters () == null || get_method.GetParameters ().Length != 1) {
					IList the_list = (IList) get_method.Invoke (null, new object [] { target });

					if (the_list == null) {
						the_list = (IList) Activator.CreateInstance (set_params [1].ParameterType);
						if (the_list == null)
							return false;
						set_method.Invoke (null, new object [] {target, the_list});
					}

					the_list.Add (o_value);
					return true;
				} else {
					// I guess we need to wrap the current value in a collection, or does this error out?
					return false;
				}
			}

			o_value = ConvertType (set_params [1].ParameterType, o_value);

			set_method.Invoke (null, new object [] {target, o_value});
			return true;
		}

		private bool TrySetPropertyReflection (IntPtr parser, IntPtr top_level, string xmlns, object target, IntPtr target_parent_ptr, string type_name, string name, IntPtr value_ptr, out string error)
		{
			PropertyInfo pi = target.GetType ().GetProperty (name);

			if (pi == null) {
				error = "Property does not exist.";
				return false;
			}

			if (!SetPropertyFromValue (parser, top_level, target, target_parent_ptr, pi, value_ptr, out error))
				return false;

			error = null;
			return true;
		}

		private bool TrySetEventReflection (IntPtr top_level, string xmlns, object publisher, string type_name, string name, IntPtr value_ptr, out string error)
		{
			object subscriber = DependencyObject.Lookup (top_level);
			EventInfo ie = publisher.GetType ().GetEvent (name);
			string handler_name = Value.ToObject (null, value_ptr) as string;
			
			// Console.WriteLine ("TrySetEventReflection ({0}, {1}, {2}, {3}, {4}, {5}) handler_name: {6}", top_level, xmlns, publisher, type_name, name, value_ptr, handler_name);
			
			if (ie == null) {
				error = "Event does not exist.";
				return false;
			}


			if (handler_name == null) {
				error = "No method name supplied for event handler.";
				return false;
			}

			MethodInfo invoker_info = ie.EventHandlerType.GetMethod ("Invoke");
			ParameterInfo [] event_params = invoker_info.GetParameters ();

			Delegate d = null;
			Type stype = subscriber.GetType ();
			MethodInfo [] methods = stype.GetMethods (BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
			MethodInfo candidate = null;
			bool name_match = false;
			
			for (int i = 0; i < methods.Length; i++) {
				MethodInfo m = methods [i];
				ParameterInfo [] parameters;
				
				if (m.Name != handler_name)
					continue;

				if (name_match) {
					error = "Multiple candidates with the same name found for event handler.";
					// Console.WriteLine (error);
					return false;
				}

				name_match = true;

				if (m.ReturnType != typeof (void))
					continue;

				parameters = m.GetParameters ();

				if (parameters.Length != event_params.Length)
					continue;

				bool match = true;
				for (int p = 0; p < parameters.Length; p++) {
					if (!event_params [p].ParameterType.IsSubclassOf (parameters [p].ParameterType) && parameters [p].ParameterType != event_params [p].ParameterType) {
						Console.WriteLine ("mismatch:  {0}  and {1}", parameters [p].ParameterType, event_params [p].ParameterType);
						match = false;
						break;
					}
				}

				if (!match)
					continue;

				if (candidate != null) {
					error = "Multiple candidates for event handler found.";
					// Console.WriteLine (error);
					return false;
				}

				candidate = m;
			}

			if (candidate == null) {
				error = "Event handler not found.";
				// Console.WriteLine (error);
				return false;
			}

			Console.WriteLine ("Found event handler: {1}::{0}", candidate.Name, candidate.DeclaringType.FullName);
			
			d = Delegate.CreateDelegate (ie.EventHandlerType, subscriber, candidate, false);
			
			if (d == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create delegate (src={3} target={4}).", top_level, name, value_ptr, ie.EventHandlerType, publisher);
				error = "Can not create even delegate.";
				return false;
			}

			// Console.Error.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): Successfully created delegate (src={3} target={4}).", top_level, name, value_ptr, ie.EventHandlerType, publisher);
				
			error = null;
			ie.AddEventHandler (publisher, d);
			return true;
		}

		private bool SetProperty (IntPtr parser, IntPtr top_level, string xmlns, IntPtr target_ptr, IntPtr target_parent_ptr, string name, IntPtr value_ptr)
		{
			string error;
			object target = LookupObject (target_ptr);

			if (target == null) {
				Console.Error.WriteLine ("target is null");
				return false;
			}

			int dot = name.IndexOf ('.');
			string type_name = null;

			if (dot >= 0) {
				type_name = name.Substring (0, dot);
				if (xmlns != null) {
					string ns = ClrNamespaceFromXmlns (xmlns);
					type_name = String.Concat (ns, ".", type_name);
				}
				name = name.Substring (++dot, name.Length - dot);
			}

			if (TrySetPropertyReflection (parser, top_level, xmlns, target, target_parent_ptr, type_name, name, value_ptr, out error))
				return true;

			if (TrySetEventReflection (top_level, xmlns, target, type_name, name, value_ptr, out error))
				return true;

			if (TrySetAttachedProperty (top_level, xmlns, target_ptr, type_name, name, value_ptr))
				return true;

			return false;
		}

		//
		// TODO: Is it legal to jam the whole metadata right in the string ie: TargetType="clr-namespace:Mono;MyType"
		//
		private Type TypeFromString (IntPtr parser, IntPtr top_level, string str)
		{
			string assembly_name = null;
			string full_name = str;
			Type res = null;

			int ps = str.IndexOf (':');
			if (ps > 0) {
				string xmlns = NativeMethods.xaml_uri_for_prefix (parser, str.Substring (0, ps));
				string name = str.Substring (ps + 1, str.Length - ps -1);
				string clr_namespace = ClrNamespaceFromXmlns (xmlns);
				assembly_name = AssemblyNameFromXmlns (xmlns);

				full_name = string.IsNullOrEmpty (clr_namespace) ? name : clr_namespace + "." + name;
			}

			do {
				if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
					Console.Error.WriteLine ("unable to find the assembly name for the target type.");
					break;
				}

				Assembly assembly = null;
				if (LoadAssembly (assembly_name, out assembly) != AssemblyLoadResult.Success) {
					Console.Error.WriteLine ("unable to load assembly for target type.");
					break;
				}

				res = assembly.GetType (full_name);
			} while (false);

			if (res == null)
				res = Application.GetComponentTypeFromName (full_name);

			return res;
		}

		private DependencyProperty DependencyPropertyFromString (IntPtr parser, IntPtr top_level, object otarget, IntPtr target_parent_ptr, string str_value)
		{
			object o = LookupObject (target_parent_ptr);
			Style parent = o as Style;

			if (parent == null) {
				Console.Error.WriteLine ("DependencyPropertyFromString Parent of target is not a Style. It's a {0}", o);
				return null;
			}

			Type target_type = parent.TargetType;
			if (target_type == null) {
				Console.Error.WriteLine ("DependencyPropertyFromString TargetType is null.");
				return null;
			}

			//
			// Check to see if we have an attached property
			//
			int dot = str_value.IndexOf ('.');
			if (dot >= 0) {
				string type_name = str_value.Substring (0, dot);
				str_value= str_value.Substring (++dot, str_value.Length - dot);

				target_type = TypeFromString (parser, top_level, type_name);
			}

			//
			// Yup, we have to call Initialize to make sure that the DPs get registered
			//
			Type walk = target_type;
			while (walk != typeof (object)) {
				System.Runtime.CompilerServices.RuntimeHelpers.RunClassConstructor (walk.TypeHandle);
				walk = walk.BaseType;
			}

			ManagedType mt = Types.Find (target_type);
			DependencyProperty dp = DependencyProperty.Lookup ((Kind) mt.native_handle, str_value);

			return dp;
		}
		
		private bool SetPropertyFromValue (IntPtr parser, IntPtr top_level, object target, IntPtr target_parent_ptr, PropertyInfo pi, IntPtr value_ptr, out string error)
		{
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			error = null;

			if (str_value != null) {
				IntPtr unmanaged_value;


				//
				// HACK: This really shouldn't be here, but I don't want to bother putting it in Helper, because that
				// code probably should be moved into this file
				//
				if (pi.PropertyType == typeof (Type)) {
					Type t = TypeFromString (parser, top_level, str_value);
					if (t != null) {
						pi.SetValue (target, t, null);
						return true;
					}
				}

				if (pi.PropertyType == typeof (DependencyProperty)) {
					DependencyProperty dp = DependencyPropertyFromString (parser, top_level, target, target_parent_ptr, str_value);
					if (dp != null) {
						pi.SetValue (target, dp, null);
						return true;
					}
				}

				Helper.SetPropertyFromString (target, pi, str_value, out error, out unmanaged_value);

				if (error == null && unmanaged_value != IntPtr.Zero)
					obj_value = Value.ToObject (null, unmanaged_value);
				else
					return error == null;
			} else {
				obj_value = Value.ToObject (pi.PropertyType, value_ptr);
			}

			if (typeof (IList).IsAssignableFrom (pi.PropertyType) && !(obj_value is IList)) {
				IList the_list = (IList) pi.GetValue (target, null);

				if (the_list == null) {
					the_list = (IList) Activator.CreateInstance (pi.PropertyType);
					if (the_list == null) {
						error = "Unable to create instance of list: " + pi.PropertyType;
						return false;
					}
					pi.SetValue (target, the_list, null);
				}
				the_list.Add (obj_value);
				return true;
			}

			obj_value = ConvertType (pi.PropertyType, obj_value);
			pi.SetValue (target, obj_value, null);
			return true;
		}

		private static object ConvertType (Type t, object value)
		{
			if (value.GetType () == t)
				return value;

			TypeConverter converter = GetConverterFor (t);
			if (converter != null && converter.CanConvertFrom (value.GetType ()))
				return converter.ConvertFrom (value);

			try {
				value = Convert.ChangeType (value, t, System.Globalization.CultureInfo.CurrentCulture);
			} catch {
			}
			
			// This will just let things fail
			return value;
		}

		private static TypeConverter GetConverterFor (Type type)
		{
			Attribute[] attrs = (Attribute []) type.GetCustomAttributes (true);
			TypeConverterAttribute at = null;
			TypeConverter converter = null;

			foreach (Attribute attr in attrs) {
				if (attr is TypeConverterAttribute) {
					at = (TypeConverterAttribute) attr;
					break;
				}
			}

			if (at == null)
				return null;

			Type t = Type.GetType (at.ConverterTypeName);
			if (t == null)
				return null;

			converter = (TypeConverter) Activator.CreateInstance (t);
			return converter;
		}

		private static string ClrNamespaceFromXmlns (string xmlns)
		{
			if (String.IsNullOrEmpty (xmlns))
				return null;

			int start = xmlns.IndexOf ("clr-namespace:") + "clr-namespace:".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
		}

		private static string AssemblyNameFromXmlns (string xmlns)
		{
			if (String.IsNullOrEmpty (xmlns))
				return null;

			int start = xmlns.IndexOf ("assembly=");
			if (start < 0)
				return null;

			start += "assembly=".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
		}

		///
		///
		/// Callbacks invoked by the xaml.cpp C++ parser
		///
		///

#region Callbacks from xaml.cpp
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// genereting an exception and unwinding the stack.
		//
		private bool cb_lookup_object (IntPtr parser, IntPtr top_level, string xmlns, string name, bool create, out Value value)
		{
			try {
				return LookupObject (top_level, xmlns, name, create, out value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}, {3}) failed: {3} ({4}).", top_level, xmlns, create, name, ex.Message, ex.GetType ().FullName);
				value = Value.Empty;
				return false;
			}
		}

		private void cb_create_gchandle ()
		{
			if (!handle.IsAllocated)
				handle = GCHandle.Alloc (this);
		}
		
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// generating an exception and unwinding the stack.
		//
		private bool cb_set_property (IntPtr parser, IntPtr top_level, string xmlns, IntPtr target, IntPtr target_parent, string name, IntPtr value_ptr)
		{
			try {
				return SetProperty (parser, top_level, xmlns, target, target_parent, name, value_ptr);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::SetProperty ({0}, {1}, {2}, {3}, {4}) threw an exception: {5}.", top_level, xmlns, target, name, value_ptr, ex.Message);
				Console.Error.WriteLine (ex);
				return false;
			}
		}

		private void cb_import_xaml_xmlns (IntPtr parser, string xmlns)
		{
			try {
				Application.ImportXamlNamespace (xmlns);
			} catch (Exception ex) {
				Console.WriteLine ("Application::ImportXamlNamespace ({0}) threw an exception:\n{1}", xmlns, ex);
			}

		}

		private string cb_get_content_property_name (IntPtr parser, IntPtr dob_ptr)
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
}
