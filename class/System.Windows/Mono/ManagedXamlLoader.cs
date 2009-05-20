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
using System.Windows.Controls;
using System.Windows.Markup;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Windows.Data;
using Mono;

namespace Mono.Xaml
{
	internal class ManagedXamlLoader : XamlLoader {

		Assembly assembly;
		XamlLoaderCallbacks callbacks;
		GCHandle handle;

		public ManagedXamlLoader ()
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
		public override object CreateObjectFromString (string xaml, bool createNamescope)
		{
			if (xaml == null)
				throw new ArgumentNullException ("xaml");

			IntPtr top;
			object result;
			Kind kind;
			
			DependencyObject.Initialize ();
			
			top = CreateFromString (xaml, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = Value.ToObject (null, top);
			DependencyObject dob = result as DependencyObject;
			if (dob != null) {
				NativeMethods.event_object_unref (dob.native);
			}

			return result;
		}

		// 
		// Creates a managed dependency object from the xaml in the file
		// 
		public override object CreateObjectFromFile (string file, bool createNamescope)
		{
			if (file == null)
				throw new ArgumentNullException ("file");

			IntPtr top;
			object result;
			Kind kind;
			
			DependencyObject.Initialize ();

			top = CreateFromFile (file, createNamescope, out kind);
			
			if (top == IntPtr.Zero)
				return null;

			result = Value.ToObject (null, top);
			DependencyObject dob = result as DependencyObject;
			if (dob != null) {
				NativeMethods.event_object_unref (dob.native);
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
			if (assembly != null) {
				assembly_name = assembly.GetName ().Name;
				return true;
			}

			object obj = Value.ToObject (null, top_level);

			if (obj == null) {
				assembly_name = null;
				return false;
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
			string full_name = string.IsNullOrEmpty (clr_namespace) ? name : clr_namespace + "." + name;

			Type type = LookupType (top_level, assembly_name, full_name);
			if (type == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LookupObject: GetType ({0}) failed using assembly: {1} ({2}).", name, assembly_name, xmlns);
				value = Value.Empty;
				return false;
			}

			if (create) {
				object res = null;
				try {
					res = Activator.CreateInstance (type);
				} catch (TargetInvocationException ex) {
					Console.WriteLine (ex);
					Console.Error.WriteLine ("ManagedXamlLoader::LookupObject: CreateInstance ({0}) failed: {1}", name, ex.InnerException);
					value = Value.Empty;
					return false;
				}

				if (res == null) {
					Console.Error.WriteLine ("ManagedXamlLoader::LookupObject ({0}, {1}, {2}): unable to create object instance: '{3}', the object was of type '{4}'",
								 assembly_name, xmlns, name, full_name, type.FullName);
					value = Value.Empty;
					return false;
				}
				value = Value.FromObject (res, false);
			} else {
				value = Value.Empty;
				value.k = Deployment.Current.Types.Find (type).native_handle;
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
				value.k = Deployment.Current.Types.Find (type).native_handle;
				return true;
			}

			object obj = Application.CreateComponentFromName (name);
			if (obj == null) {
				value = Value.Empty;
				return false;
			}

			value = Value.FromObject (obj, false);
			return true;
		}

		private static bool IsAttachedProperty (string name)
		{
			return name.IndexOf ('.') > 0;
		}

		private DependencyProperty LookupDependencyPropertyForBinding (IntPtr top_level, IntPtr parser, FrameworkElement fwe, string type_name, string propertyName)
		{
			// map the property name + type_name to an actual DependencyProperty
			Kind kind;
			Type type = string.IsNullOrEmpty (type_name) ? null : TypeFromString (parser, top_level, type_name);
			if (type != null) {
				Types.Ensure (type);
				kind = Deployment.Current.Types.TypeToNativeKind (type);
			} else {
				kind = fwe.GetKind ();
			}

			if (kind == Kind.INVALID)
				return null;
			
			try {
				return DependencyProperty.Lookup (kind, propertyName);
			}
			catch {
				return null;
			}
		}


		private bool TrySetExpression (IntPtr top_level, IntPtr loader, IntPtr parser, object target, IntPtr target_data, string type_name, string name, IntPtr value_ptr)
		{
			FrameworkElement dob = target as FrameworkElement;
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			if (str_value == null || dob == null)
				return false;
			
			if (!str_value.StartsWith ("{"))
				return false;

			MarkupExpressionParser p = new MarkupExpressionParser (dob, name, parser, target_data);
			string expression = str_value;
			object o = p.ParseExpression (ref expression);

			if (o == null)
				return false;

			if (o is Binding ||
			    o is TemplateBindingExpression) {
				DependencyProperty prop = LookupDependencyPropertyForBinding (top_level, parser, dob, type_name, name);
				if (prop == null)
					return false;

				if (o is Binding) {
					Binding binding = o as Binding;

					dob.SetBinding (prop, binding);
					return true;
				}
				else if (o is TemplateBindingExpression) {
					TemplateBindingExpression tb = o as TemplateBindingExpression;

					IntPtr context = NativeMethods.xaml_loader_get_context (loader);
					IntPtr source_ptr = NativeMethods.xaml_context_get_template_binding_source (context);

					DependencyObject templateSourceObject = NativeDependencyObjectHelper.Lookup (source_ptr) as DependencyObject;

					if (templateSourceObject == null)
						return false;

					DependencyProperty sourceProperty = DependencyProperty.Lookup (templateSourceObject.GetKind(),
												       tb.SourcePropertyName);
					if (sourceProperty == null)
						return false;

					tb.TargetProperty = prop;
					tb.Source = templateSourceObject as Control;
					tb.SourceProperty = sourceProperty;

					dob.SetTemplateBinding (prop, tb);

					return true;
				}
			}

			return true;
		}

		private bool TrySetAttachedProperty (IntPtr top_level, IntPtr loader, IntPtr parser, string xmlns, object target, IntPtr target_data, string type_name, string name, IntPtr value_ptr)
		{
			if (type_name == null)
				return false;

			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string ns = ClrNamespaceFromXmlns (xmlns);

			if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
				Console.Error.WriteLine ("Unable to find an assembly to load type from.");
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
			object o_value = GetObjectValue (target, target_data, name, parser, value_ptr, out error);

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

					try {
						the_list.Add (o_value);

						if (o_value is DependencyObject && target is DependencyObject && !(the_list is DependencyObject)) {
							NativeMethods.dependency_object_set_parent (((DependencyObject)o_value).native, ((DependencyObject)target).native);
						}

						return true;
					}
					catch {
						// don't return here, fall through to the ConvertType case below.
					}
				} else {
					// I guess we need to wrap the current value in a collection, or does this error out?
					Console.WriteLine ("ow god my eye!");
					return false;
				}
			}

			o_value = ConvertType (null, set_params [1].ParameterType, o_value);
			set_method.Invoke (null, new object [] {target, o_value});
			return true;
		}

		private bool TrySetPropertyReflection (IntPtr top_level, IntPtr loader, IntPtr parser, string xmlns, object target, IntPtr target_data, IntPtr target_parent_ptr, string type_name, string name, IntPtr value_ptr, IntPtr value_data, out string error)
		{
			PropertyInfo pi = target.GetType ().GetProperty (name, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

			if (pi == null) {
				error = "Property does not exist.";
				return false;
			}

			if (!SetPropertyFromValue (parser, top_level, target, target_data, target_parent_ptr, pi, value_ptr, value_data, out error))
				return false;

			error = null;
			return true;
		}

		private bool TrySetEventReflection (IntPtr top_level, IntPtr loader, string xmlns, object publisher, string type_name, string name, IntPtr value_ptr, out string error)
		{
			object subscriber = null;
			EventInfo ie = publisher.GetType ().GetEvent (name);
			string handler_name = Value.ToObject (null, value_ptr) as string;

			try {
				subscriber = Value.ToObject (null, top_level);
			} catch {

			}
				
			//Console.WriteLine ("TrySetEventReflection ({0}, {1}, {2}, {3}, {4}, {5}) handler_name: {6}", top_level, xmlns, publisher, type_name, name, value_ptr, handler_name);
			
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
			MethodInfo [] methods = stype.GetMethods (BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.DeclaredOnly | BindingFlags.Instance);
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

		private bool TrySetEnumContentProperty (IntPtr top_level, IntPtr loader, IntPtr parser, string xmlns, object target, IntPtr target_ptr, IntPtr target_data, IntPtr value_ptr, IntPtr value_data)
		{
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			if (str_value == null)
				return false;
			
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string type_name = NativeMethods.xaml_get_element_name (parser, target_data);
			string full_name = String.IsNullOrEmpty (clr_namespace) ? type_name : clr_namespace + "." + type_name;

			Type type = LookupType (top_level, assembly_name, full_name);

			if (type == null || !type.IsEnum)
				return false;

			object e = Enum.Parse (type, str_value);

			NativeMethods.value_free_value2 (target_ptr);

			unsafe {
				Value *val = (Value *) target_ptr;

				GCHandle handle = GCHandle.Alloc (e);
				val->k = Kind.MANAGED;
				val->u.p = GCHandle.ToIntPtr (handle);
			}

			return true;
		}

		private bool TrySetCollectionContentProperty (IntPtr top_level, IntPtr loader, IntPtr parser, string xmlns, object target, IntPtr target_ptr, IntPtr target_data, IntPtr value_ptr, IntPtr value_data)
		{
			IList list = target as IList;

			if (list == null)
				return false;

			object value = Value.ToObject (null, value_ptr);

			list.Add (value);
			return true;
		}

		private bool SetProperty (IntPtr loader, IntPtr parser, IntPtr top_level, string xmlns, IntPtr target_ptr, IntPtr target_data, IntPtr target_parent_ptr, string name, IntPtr value_ptr, IntPtr value_data)
		{
			string error;
			object target = Value.ToObject (null, target_ptr);

			if (target == null) {
				Console.Error.WriteLine ("target is null: {0} {1} {2}", target_ptr, name, xmlns);
				return false;
			}

			if (name == null) {
				if (TrySetEnumContentProperty (top_level, loader, parser, xmlns, target, target_ptr, target_data, value_ptr, value_data))
					return true;
				if (TrySetCollectionContentProperty (top_level, loader, parser, xmlns, target, target_ptr, target_data, value_ptr, value_data))
					return true;
				Console.Error.WriteLine ("no property name supplied");
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

			if (TrySetExpression (top_level, loader, parser, target, target_data, type_name, name, value_ptr))
				return true;

			if (TrySetPropertyReflection (top_level, loader, parser, xmlns, target, target_data, target_parent_ptr, type_name, name, value_ptr, value_data, out error))
				return true;

			if (TrySetEventReflection (top_level, loader, xmlns, target, type_name, name, value_ptr, out error))
				return true;

			if (TrySetAttachedProperty (top_level, loader, parser, xmlns, target, target_data, type_name, name, value_ptr))
				return true;

			return false;
		}

		private Type LookupType (IntPtr top_level, string assembly_name, string full_name)
		{
			Type res = null;
			bool explicit_assembly = assembly_name != null;

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

				if (res == null && explicit_assembly && TryGetDefaultAssemblyName (top_level, out assembly_name)) {
					if (LoadAssembly (assembly_name, out assembly) != AssemblyLoadResult.Success) {
						Console.Error.WriteLine ("unable to load default assembly for target type.");
						break;
					}
					res = assembly.GetType (full_name);
					Console.WriteLine ("type:  {0}  base:  {1}", res, res.BaseType);
					if (res != null && !res.IsPublic)
						res = null;
				}

				if (res == null && !explicit_assembly) {
					assembly = typeof (DependencyObject).Assembly;
					res = assembly.GetType (full_name);
					if (res != null && !res.IsPublic)
						res = null;
				}
			} while (false);

			if (res == null)
				res = Application.GetComponentTypeFromName (full_name);

			return res;
		}

		private void SetCLRPropertyFromString (object target, PropertyInfo pi, string value, out string error, out IntPtr unmanaged_value)
		{
			unmanaged_value = IntPtr.Zero;
			error = null;

			object new_value = null;
			try {
				new_value = MoonlightTypeConverter.ConvertObject (pi, value, target.GetType ());
				pi.SetValue (target, new_value, null);
				return;
			} catch (Exception ex) {
				
			}

			//
			// lastly, attempt to create an unmanaged Value* object, if one is created, the managed
			// parser will create a managed wrapper for the object and call SetPropertyFromValue with
			// the managed object
			//
			bool result = NativeMethods.value_from_str_with_typename (TypeToMoonType (pi.PropertyType), pi.Name, value, out unmanaged_value);
			if (!result) {
				error = string.Format ("unable to convert to type {0} from a string", pi.PropertyType);
			}
		}

		private string TypeToMoonType (Type t)
		{
			if (t == typeof (double))
				return "double";
			if (t == typeof (bool))
				return "bool";
			return t.Name;
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

			return LookupType (top_level, assembly_name, full_name);
		}

		private DependencyProperty DependencyPropertyFromString (IntPtr parser, IntPtr top_level, object otarget, IntPtr target_parent_ptr, string str_value)
		{
			object o = Value.ToObject (null, target_parent_ptr);
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
			
			Types.Ensure (target_type);

			ManagedType mt = Deployment.Current.Types.Find (target_type);
			DependencyProperty dp = DependencyProperty.Lookup ((Kind) mt.native_handle, str_value);

			return dp;
		}
		
		private bool SetPropertyFromValue (IntPtr parser, IntPtr top_level, object target, IntPtr target_data, IntPtr target_parent_ptr, PropertyInfo pi, IntPtr value_ptr, IntPtr value_data, out string error)
		{
			error = null;
			object obj_value = Value.ToObject (null, value_ptr);

			if (obj_value is Binding && target is FrameworkElement) {
				FrameworkElement fe = (FrameworkElement) target;
				fe.SetBinding (DependencyProperty.Lookup (fe.GetKind (), pi.Name), (Binding) obj_value);
				return true;
			};

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

				try {
					the_list.Add (obj_value);

					if (obj_value is DependencyObject && target is DependencyObject && !(the_list is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject)obj_value).native, ((DependencyObject)target).native);
					}

					return true;
				}
				catch {
					// don't return here, fall
					// through to the string case
					// below.
				}
			}

			if (typeof (ResourceDictionary).IsAssignableFrom (pi.PropertyType) && !(obj_value is ResourceDictionary)) {
				ResourceDictionary the_dict = (ResourceDictionary) pi.GetValue (target, null);

				if (the_dict == null) {
					the_dict = (ResourceDictionary) Activator.CreateInstance (pi.PropertyType);
					if (the_dict == null) {
						error = "Unable to create instance of dictionary: " + pi.PropertyType;
						return false;
					}
					pi.SetValue (target, the_dict, null);
				}

				if (value_data == IntPtr.Zero) {
					error = "Can not add attributes to a resource dictionary.";
					return false;
				}

				try {
					string key = NativeMethods.xaml_get_element_key (parser, value_data);

					if (key == null) {
						error = "No key for element: " + obj_value;
						return false;
					}

					the_dict.Add (key, obj_value);
					if (obj_value is DependencyObject && target is DependencyObject && !(the_dict is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject)obj_value).native, ((DependencyObject) target).native);
					}

					return true;
				} catch (Exception e) {
					// Fall through to string
					Console.Error.WriteLine (e);
				}
			}

			string str_value = obj_value as string;
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

				if (typeof (System.Windows.Data.Binding).IsAssignableFrom (pi.PropertyType) && MarkupExpressionParser.IsBinding (str_value)) {
					MarkupExpressionParser p = new MarkupExpressionParser (null, pi.Name,  parser, target_data);

					string expression = str_value;
					obj_value = p.ParseExpression (ref expression);

					if (!(obj_value is Binding))
						return false;

					pi.SetValue (target, obj_value, null);
					return true;
				}
				
				if (MarkupExpressionParser.IsStaticResource (str_value)) {
					// FIXME: The NUnit tests show we need to use the parent of the target to resolve
					// the StaticResource, but are there any cases where we should use the actual target?
					MarkupExpressionParser p = new MarkupExpressionParser ((DependencyObject) NativeDependencyObjectHelper.Lookup (target_parent_ptr), "", parser, target_data);
					obj_value = p.ParseExpression (ref str_value);
					obj_value = ConvertType (pi, pi.PropertyType, obj_value);
					pi.SetValue (target, obj_value, null);
					return true;
				}

				SetCLRPropertyFromString (target, pi, str_value, out error, out unmanaged_value);

				if (error == null && unmanaged_value != IntPtr.Zero)
					obj_value = Value.ToObject (null, unmanaged_value);
				else
					return error == null;
			} else {
				obj_value = Value.ToObject (pi.PropertyType, value_ptr);
			}

			obj_value = ConvertType (pi, pi.PropertyType, obj_value);
			pi.SetValue (target, obj_value, null);
			return true;
		}

		private static object ConvertType (PropertyInfo pi, Type t, object value)
		{
			if (value == null)
				return null;

			Type valueType = value.GetType ();
			if (valueType == t)
				return value;

			TypeConverter converter = Helper.GetConverterFor (pi, t);

			if (converter != null && converter.CanConvertFrom (value.GetType ()))
				return converter.ConvertFrom (value);

			try {
				if (t.IsEnum && value is string)
					return Enum.Parse (t, (string)value);
			} catch {
			}

			try {
				if (!valueType.IsSubclassOf (t))
					value = Convert.ChangeType (value, t, System.Globalization.CultureInfo.CurrentCulture);
			} catch {
			}
			
			// This will just let things fail
			return value;
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

		private static object GetObjectValue (object target, IntPtr target_data, string prop_name, IntPtr parser, IntPtr value_ptr, out string error)
		{
			error = null;

			IntPtr unmanaged_value = IntPtr.Zero;
			object o_value = Value.ToObject (null, value_ptr);
			if (error == null && unmanaged_value != IntPtr.Zero)
				o_value = Value.ToObject (null, unmanaged_value);

			if (o_value is String && MarkupExpressionParser.IsStaticResource ((string) o_value)) {
				MarkupExpressionParser mp = new MarkupExpressionParser ((DependencyObject) target, prop_name, parser, target_data);
				string str_value = o_value as String;
				o_value = mp.ParseExpression (ref str_value);
			}

			return o_value;
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
		private bool cb_lookup_object (IntPtr loader, IntPtr parser, IntPtr top_level, string xmlns, string name, bool create, out Value value)
		{
			try {
				return LookupObject (top_level, xmlns, name, create, out value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LookupObject ({0}, {1}, {2}, {3}) failed: {3} ({4}).", top_level, xmlns, create, name, ex.Message, ex.GetType ().FullName);
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
		private bool cb_set_property (IntPtr loader, IntPtr parser, IntPtr top_level, string xmlns, IntPtr target, IntPtr target_data, IntPtr target_parent, string name, IntPtr value_ptr, IntPtr value_data)
		{
			try {
				return SetProperty (loader, parser, top_level, xmlns, target, target_data, target_parent, name, value_ptr, value_data);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::SetProperty ({0}, {1}, {2}, {3}, {4}) threw an exception: {5}.", top_level, xmlns, target, name, value_ptr, ex.Message);
				Console.Error.WriteLine (ex);
				return false;
			}
		}

		private void cb_import_xaml_xmlns (IntPtr loader, IntPtr parser, string xmlns)
		{
			try {
				Application.ImportXamlNamespace (xmlns);
			} catch (Exception ex) {
				Console.WriteLine ("Application::ImportXamlNamespace ({0}) threw an exception:\n{1}", xmlns, ex);
			}

		}

		private string cb_get_content_property_name (IntPtr loader, IntPtr parser, IntPtr object_ptr)
		{
			object obj = Value.ToObject (null, object_ptr);

			if (obj == null)
				return null;

			Type t = obj.GetType ();
			object [] o = t.GetCustomAttributes (typeof (ContentPropertyAttribute), true);
			if (o.Length == 0)
				return null;
			ContentPropertyAttribute cpa = (ContentPropertyAttribute ) o [0];

			return cpa.Name;
		}
#endregion
	}
}
