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
			callbacks.create_object = new CreateObjectCallback (cb_create_object);
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

		private bool CreateObject (IntPtr top_level, string xmlns, string name, out Value value)
		{
			

			if (name == null)
				throw new ArgumentNullException ("type_name");

			if (top_level == IntPtr.Zero && xmlns == null)
				return CreateComponentFromName (top_level, name, out value);
			
			AssemblyLoadResult load_result;
			Assembly clientlib = null;
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string full_name;

			if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
				Console.Error.WriteLine ("Unable to find an assembly to load type from.");
				value = Value.Empty;
				return false;
			}

			load_result = LoadAssembly (assembly_name, out clientlib);
			if (load_result != AssemblyLoadResult.Success) {
				Console.Error.WriteLine ("Unable to load assembly");
				value = Value.Empty;
				return false;
			}

			if (clientlib == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}): Assembly loaded, but where is it?", assembly_name, xmlns, name);
				value = Value.Empty;
				return false;
			}
			
			if (clr_namespace == null || clr_namespace == string.Empty)
				full_name = name;
			else
				full_name = String.Concat (clr_namespace, ".", name);

			object res = null;
			try {
				res = Activator.CreateInstance (clientlib.GetType (full_name, true));
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

			value = DependencyObject.GetAsValue (res, true);
			return true;
		}

		private bool CreateComponentFromName (IntPtr top_level, string name, out Value value)
		{
			object obj = Application.CreateComponentFromName (name);
			if (obj == null) {
				value = Value.Empty;
				return false;
			}

			value = DependencyObject.GetAsValue (obj, true);
			return true;
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

		private bool IsAttachedProperty (string name)
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
				Console.Error.WriteLine ("attach type is null  {0}", type_name);
				return false;
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

			object o_value = DependencyObject.ValueToObject (null, value_ptr);
			if (error == null && unmanaged_value != IntPtr.Zero)
				o_value = DependencyObject.ValueToObject (null, unmanaged_value);

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

		private bool TrySetPropertyReflection (IntPtr top_level, string xmlns, object target, string type_name, string name, IntPtr value_ptr, out string error)
		{
			PropertyInfo pi = target.GetType ().GetProperty (name);

			if (pi == null) {
				error = "Property does not exist.";
				return false;
			}

			if (!SetPropertyFromValue (target, pi, value_ptr, out error))
				return false;

			error = null;
			return true;
		}

		private bool TrySetEventReflection (IntPtr top_level, string xmlns, object publisher, string type_name, string name, IntPtr value_ptr, out string error)
		{
			object subscriber = DependencyObject.Lookup (top_level);
			EventInfo ie = publisher.GetType ().GetEvent (name);
			
			if (ie == null) {
				error = "Event does not exist.";
				return false;
			}

			string handler_name = DependencyObject.ValueToObject (null, value_ptr) as string;

			if (handler_name == null) {
				error = "No method name supplied for event handler.";
				return false;
			}

			Delegate d = Delegate.CreateDelegate (ie.EventHandlerType, subscriber, handler_name);
			if (d == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create delegate (src={3} target={4}).", top_level, name, value_ptr, ie.EventHandlerType, publisher);
				error = "Can not create even delegate.";
				return false;
			}

			error = null;
			ie.AddEventHandler (publisher, d);
			return true;
		}

		private bool SetProperty (IntPtr top_level, string xmlns, IntPtr target_ptr, string name, IntPtr value_ptr)
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

			if (TrySetPropertyReflection (top_level, xmlns, target, type_name, name, value_ptr, out error))
				return true;

			if (TrySetEventReflection (top_level, xmlns, target, type_name, name, value_ptr, out error))
				return true;

			if (TrySetAttachedProperty (top_level, xmlns, target_ptr, type_name, name, value_ptr))
				return true;

			return false;
		}

		private bool SetPropertyFromValue (object target, PropertyInfo pi, IntPtr value_ptr, out string error)
		{
			object obj_value = DependencyObject.ValueToObject (null, value_ptr);
			string str_value = obj_value as string;

			error = null;

			if (str_value != null) {
				IntPtr unmanaged_value;

				Helper.SetPropertyFromString (target, pi, str_value, out error, out unmanaged_value);

				if (error == null && unmanaged_value != IntPtr.Zero)
					obj_value = DependencyObject.ValueToObject (null, unmanaged_value);
				else
					return error == null;
			} else
				obj_value = DependencyObject.ValueToObject (pi.PropertyType, value_ptr);

			if (typeof (IList).IsAssignableFrom (pi.PropertyType) && !(obj_value is IList)) {
				IList the_list = (IList) pi.GetValue (target, null);

				if (the_list == null) {
					the_list = (IList) Activator.CreateInstance (pi.PropertyType);
					if (the_list == null) {
						error = "Unable to create instance of list: " + pi.PropertyType;
						return false;
					}
					pi.SetValue (target, the_list, null);
					the_list.Add (obj_value);
					return true;
				} else {
					// I guess we need to wrap the current value in a collection, or does this error out?
					return false;
				}
			}

			obj_value = ConvertType (pi.PropertyType, obj_value);
			pi.SetValue (target, obj_value, null);
			return true;
		}

		private object ConvertType (Type t, object value)
		{
			if (value.GetType () == t)
				return value;

			TypeConverter converter = GetConverterFor (t);
			if (converter != null && converter.CanConvertFrom (value.GetType ()))
				return converter.ConvertFrom (value);

			// TODO: Handle IConvertible
			
			// This will just let thigns fail
			return value;
		}

		public static TypeConverter GetConverterFor (Type type)
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
			if (xmlns == null)
				return null;

			int start = xmlns.IndexOf ("clr-namespace:") + "clr-namespace:".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
		}

		private static string AssemblyNameFromXmlns (string xmlns)
		{
			if (xmlns == null)
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
		private bool cb_create_object (IntPtr top_level, string xmlns, string name, out Value value)
		{
			try {
				return CreateObject (top_level, xmlns, name, out value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LoadObject ({0}, {1}, {2}) failed: {3} ({4}).", top_level, xmlns, name, ex.Message, ex.GetType ().FullName);
				value = Value.Empty;
				return false;
			}
		}
		
		//
		// Proxy so that we return IntPtr.Zero in case of any failures, instead of
		// generating an exception and unwinding the stack.
		//
		private bool cb_set_property (IntPtr top_level, string xmlns, IntPtr target, string name, IntPtr value_ptr)
		{
			try {
				return SetProperty (top_level, xmlns, target, name, value_ptr);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::SetProperty ({0}, {1}, {2}, {3}, {4}) threw an exception: {5}.", top_level, xmlns, target, name, value_ptr, ex.Message);
				Console.Error.WriteLine (ex);
				return false;
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
}

