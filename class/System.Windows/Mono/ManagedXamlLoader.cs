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
using System.Text.RegularExpressions;
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

		public ManagedXamlLoader (Assembly assembly, string resourceBase, IntPtr surface, IntPtr plugin) : base (resourceBase, surface, plugin)
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
			unsafe {
				callbacks.lookup_object = new LookupObjectCallback (cb_lookup_object);
				callbacks.create_gchandle = new CreateGCHandleCallback (cb_create_gchandle);
				callbacks.set_property = new SetPropertyCallback (cb_set_property);
				callbacks.import_xaml_xmlns = new ImportXamlNamespaceCallback (cb_import_xaml_xmlns);
				callbacks.get_content_property_name = new GetContentPropertyNameCallback (cb_get_content_property_name);
				callbacks.add_child = new AddChildCallback (cb_add_child);
			}

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
			return CreateObjectFromString (xaml, createNamescope, false);
		}

		public override object CreateObjectFromString (string xaml, bool createNamescope, bool validateTemplates)
		{
			if (xaml == null)
				throw new ArgumentNullException ("xaml");

			IntPtr top;
			object result;
			Kind kind;
			
			DependencyObject.Initialize ();
			
			top = CreateFromString (xaml, createNamescope, validateTemplates, out kind);
			
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

		private unsafe bool TryGetDefaultAssemblyName (Value* top_level, out string assembly_name)
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

		private unsafe bool LookupObject (Value *top_level, Value *parent, string xmlns, string name, bool create, bool is_property, out Value value)
		{
			if (name == null)
				throw new ArgumentNullException ("type_name");

			
			if (is_property) {
				int dot = name.IndexOf (".");
				return LookupPropertyObject (top_level, parent, xmlns, name, dot, create, out value);
			}

			if (top_level == null && xmlns == null) {
				return LookupComponentFromName (top_level, name, create, out value);
			}

			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string full_name = string.IsNullOrEmpty (clr_namespace) ? name : clr_namespace + "." + name;

			Type type = LookupType (top_level, assembly_name, full_name);
			if (type == null) {
				Console.Error.WriteLine ("ManagedXamlLoader::LookupObject: GetType ({0}) failed using assembly: {1} ({2}, {3}).", name, assembly_name, xmlns, full_name);
				value = Value.Empty;
				return false;
			}

			if (create) {

				if (!type.IsPublic) {
					value = Value.Empty;
					throw new XamlParseException ("Attempting to create a private type");
				}

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

		private unsafe bool LookupPropertyObject (Value* top_level, Value* parent_value, string xmlns, string name, int dot, bool create, out Value value)
		{
			string prop_name = name.Substring (dot + 1);
			object parent = Value.ToObject (null, parent_value);

			if (parent == null) {
				value = Value.Empty;
				return false;
			}

			PropertyInfo pi = null;
			bool is_attached = true;
			string type_name = name.Substring (0, dot);

			Type t = parent.GetType ();
			while (t != typeof (object)) {
				if (t.Name == type_name) {
					is_attached = false;
					break;
				}
				t = t.BaseType;
			}

			if (is_attached) {
				Type attach_type = null;
				string full_type_name = type_name;
				if (xmlns != null) {
					string ns = ClrNamespaceFromXmlns (xmlns);
					full_type_name = String.Concat (ns, ".", type_name);
				}

				MethodInfo get_method = GetGetMethodForAttachedProperty (top_level, xmlns, type_name, full_type_name, prop_name);

				if (get_method != null)
					attach_type = get_method.ReturnType;

				if (attach_type == null) {
					value = Value.Empty;
					return false;
				}

				ManagedType mt = Deployment.Current.Types.Find (attach_type);
				value = Value.Empty;
				value.IsNull = true;
				value.k = mt.native_handle;
				return true;
			} else {

				pi = parent.GetType ().GetProperty (name.Substring (dot + 1), BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

				if (pi == null) {
					value = Value.Empty;
					return false;
				}

				ManagedType mt = Deployment.Current.Types.Find (pi.PropertyType);
				value = Value.Empty;
				value.k = mt.native_handle;
				value.IsNull = true;

				return true;
			}
		}

		private unsafe bool LookupComponentFromName (Value* top_level, string name, bool create, out Value value)
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

		private unsafe bool IsAttachedProperty (XamlCallbackData *data, object target, string xmlns, string prop_xmlns, string name)
		{
			string type_name = null;
			string full_type_name = null;

			name = GetNameForAttachedProperty (xmlns, prop_xmlns, name, out type_name, out full_type_name);

			if (name == null)
				return false;

			MethodInfo set_method = GetSetMethodForAttachedProperty (data->top_level, prop_xmlns, type_name, full_type_name, name);
			if (set_method == null)
				return false;

			return !target.GetType ().IsSubclassOf (set_method.DeclaringType);
		}

		private unsafe DependencyProperty LookupDependencyPropertyForBinding (XamlCallbackData *data, FrameworkElement fwe, string type_name, string propertyName)
		{
			// map the property name + type_name to an actual DependencyProperty
			Kind kind;
			Type type = string.IsNullOrEmpty (type_name) ? null : TypeFromString (data, type_name);
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


		private unsafe bool TrySetExpression (XamlCallbackData *data, string xmlns, object target, IntPtr target_data, Value* target_parent_ptr, string type_name, string prop_xmlns, string name, string full_name, Value* value_ptr, IntPtr value_data)
		{
			FrameworkElement dob = target as FrameworkElement;
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			if (str_value == null)
				return false;
			
			if (!str_value.StartsWith ("{"))
				return false;

			MarkupExpressionParser p = new MarkupExpressionParser (target, name, data->parser, target_data);
			string expression = str_value;
			object o = p.ParseExpression (ref expression);

			if (o == null)
				return false;

			
			if (o is Binding) {
				Binding binding = o as Binding;
				DependencyProperty prop = null;

				if (dob != null) {
					string full_type_name = type_name;
					if (IsAttachedProperty (full_name))
						GetNameForAttachedProperty (xmlns, prop_xmlns, full_name, out type_name, out full_type_name);
					prop = LookupDependencyPropertyForBinding (data, dob, full_type_name, name);
				}

				// If it's null we should look for a regular CLR property
				if (prop != null) {
					dob.SetBinding (prop, binding);
					return true;
				}
			}
			if (o is TemplateBindingExpression) {
				// Applying a {TemplateBinding} to a DO which is not a FrameworkElement should silently discard
				// the binding.
				if (dob == null)
					return true;

				TemplateBindingExpression tb = o as TemplateBindingExpression;

				IntPtr context = NativeMethods.xaml_loader_get_context (data->loader);
				IntPtr source_ptr = NativeMethods.xaml_context_get_template_binding_source (context);

				DependencyObject templateSourceObject = NativeDependencyObjectHelper.FromIntPtr (source_ptr) as DependencyObject;

				if (templateSourceObject == null)
					return false;

				DependencyProperty sourceProperty = DependencyProperty.Lookup (templateSourceObject.GetKind(),
												       tb.SourcePropertyName);
				if (sourceProperty == null)
					return false;

				DependencyProperty prop = null;

				if (dob != null)
					prop = LookupDependencyPropertyForBinding (data, dob, type_name, name);

				if (prop == null)
					return false;

				tb.TargetProperty = prop;
				tb.Source = templateSourceObject as Control;
				tb.SourceProperty = sourceProperty;

				dob.SetTemplateBinding (prop, tb);

				return true;
			}

			if (IsAttachedProperty (full_name))
				return TrySetAttachedProperty (data, xmlns, target, target_data, prop_xmlns, full_name, o);

			PropertyInfo pi = target.GetType ().GetProperty (name, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

			o = ConvertType (pi, pi.PropertyType, o);
			SetValue (data, target_data, pi, target, o);
			return true;
		}

		private string GetNameForAttachedProperty (string xmlns, string prop_xmlns, string name, out string type_name, out string full_type_name)
		{
			int dot = name.IndexOf ('.');

			if (dot >= 0) {
				type_name = name.Substring (0, dot);
				full_type_name = type_name;
				if (prop_xmlns != null || xmlns != null) {
					string ns = ClrNamespaceFromXmlns (prop_xmlns == null ? xmlns : prop_xmlns);
					if (ns != null)
						full_type_name = String.Concat (ns, ".", type_name);
				}
				name = name.Substring (++dot, name.Length - dot);
			} else {
				full_type_name = null;
				type_name = null;
				return null;
			}

			return name;
		}

		private unsafe bool TrySetAttachedProperty (XamlCallbackData *data, string xmlns, object target, IntPtr target_data, string prop_xmlns, string name, Value* value_ptr)
		{
			string full_name = name;
			string type_name = null;
			string full_type_name = null;

			name = GetNameForAttachedProperty (xmlns, prop_xmlns, name, out type_name, out full_type_name);

			if (name == null)
				return false;

			string error = null;
			object o_value = GetObjectValue (target, target_data, name, data->parser, value_ptr, out error);
			
			return TrySetAttachedProperty (data, xmlns, target, target_data, prop_xmlns, full_name, o_value);
		}

		private unsafe bool TrySetAttachedProperty (XamlCallbackData *data, string xmlns, object target, IntPtr target_data, string prop_xmlns, string name, object o_value)
		{
			string type_name = null;
			string full_type_name = null;

			name = GetNameForAttachedProperty (xmlns, prop_xmlns, name, out type_name, out full_type_name);

			if (name == null)
				return false;

			MethodInfo set_method = GetSetMethodForAttachedProperty (data->top_level, prop_xmlns, type_name, full_type_name, name);
			if (set_method == null) {
				Console.Error.WriteLine ("set method is null: {0}  {1}", String.Concat ("Set", name), prop_xmlns);
				return false;
			}

			ParameterInfo [] set_params = set_method.GetParameters ();
			if (set_params == null || set_params.Length < 2) {
				Console.Error.WriteLine ("set method signature is incorrect.");
				return false;
			}

			MethodInfo get_method = set_method.DeclaringType.GetMethod (String.Concat ("Get", name), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);

			//
			// The Setter might actually want a collection, in this case we grab the old collection with the getter
			// and then add the new object to the collection
			//
			// TODO: Check if the setter method still gets called on Silverlight
			if (typeof (IList).IsAssignableFrom (set_params [1].ParameterType) && !(o_value is IList)) {
				
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

			o_value = ConvertType (get_method, set_params [1].ParameterType, o_value);
			set_method.Invoke (null, new object [] {target, o_value});
			return true;
		}

		private unsafe bool TrySetPropertyReflection (XamlCallbackData *data, string xmlns, object target, IntPtr target_data, Value* target_parent_ptr, string type_name, string name, Value* value_ptr, IntPtr value_data, out string error)
		{	
			PropertyInfo pi = target.GetType ().GetProperty (name, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

			if (pi == null) {
				error = "Property does not exist.";
				return false;
			}

			if (!SetPropertyFromValue (data, target, target_data, target_parent_ptr, pi, value_ptr, value_data, out error))
				return false;

			error = null;
			return true;
		}

		private unsafe bool TrySetEventReflection (XamlCallbackData *data, string xmlns, object publisher, string type_name, string name, Value* value_ptr, out string error)
		{
			object subscriber = null;
			EventInfo ie = publisher.GetType ().GetEvent (name);
			string handler_name = Value.ToObject (null, value_ptr) as string;

			try {
				subscriber = Value.ToObject (null, data->top_level);
			} catch {

			}
				
			//Console.WriteLine ("TrySetEventReflection ({0}, {1}, {2}, {3}, {4}, {5}) handler_name: {6}", data->top_level, xmlns, publisher, type_name, name, value_ptr, handler_name);
			
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
				Console.Error.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): unable to create delegate (src={3} target={4}).", (IntPtr) data->top_level, name, (IntPtr)value_ptr, ie.EventHandlerType, publisher);
				error = "Can not create even delegate.";
				return false;
			}

			// Console.Error.WriteLine ("ManagedXamlLoader::HookupEvent ({0}, {1}, {2}): Successfully created delegate (src={3} target={4}).", (IntPtr) data->top_level, name, value_ptr, ie.EventHandlerType, publisher);
				
			error = null;
			ie.AddEventHandler (publisher, d);
			return true;
		}

		private unsafe bool TrySetEnumContentProperty (XamlCallbackData *data, string xmlns, object target, Value* target_ptr, IntPtr target_data, Value* value_ptr, IntPtr value_data)
		{
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			if (str_value == null)
				return false;
			
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string type_name = NativeMethods.xaml_get_element_name (data->parser, target_data);
			string full_name = String.IsNullOrEmpty (clr_namespace) ? type_name : clr_namespace + "." + type_name;

			Type type = LookupType (data->top_level, assembly_name, full_name);

			if (type == null || !type.IsEnum)
				return false;

			object e = Enum.Parse (type, str_value, true);

			NativeMethods.value_free_value2 ((IntPtr)target_ptr);

			unsafe {
				Value *val = (Value *) target_ptr;

				GCHandle handle = GCHandle.Alloc (e);
				val->k = Kind.MANAGED;
				val->u.p = GCHandle.ToIntPtr (handle);
			}

			return true;
		}

		private unsafe bool TrySetCollectionContentProperty (string xmlns, object target, Value* target_ptr, IntPtr target_data, Value* value_ptr, IntPtr value_data)
		{
			IList list = target as IList;

			if (list == null)
				return false;

			object value = Value.ToObject (null, value_ptr);

			list.Add (value);
			return true;
		}

		private unsafe bool TrySetObjectTextProperty (XamlCallbackData *data, string xmlns, object target, Value* target_ptr, IntPtr target_data, Value* value_ptr, IntPtr value_data)
		{
			object obj_value = Value.ToObject (null, value_ptr);
			string str_value = obj_value as string;

			if (str_value == null)
				return false;
			
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string type_name = NativeMethods.xaml_get_element_name (data->parser, target_data);
			string full_name = String.IsNullOrEmpty (clr_namespace) ? type_name : clr_namespace + "." + type_name;

			Type type = LookupType (data->top_level, assembly_name, full_name);

			if (type == null || type.IsSubclassOf (typeof (DependencyObject)))
				return false;

			// For now just trim the string right here, in the future this should probably be done in the xaml parser
			object e = ConvertType (null, type, str_value.Trim ());

			NativeMethods.value_free_value2 ((IntPtr)target_ptr);

			unsafe {
				Value *val = (Value *) target_ptr;

				GCHandle handle = GCHandle.Alloc (e);
				val->k = Kind.MANAGED;
				val->u.p = GCHandle.ToIntPtr (handle);
			}

			return true;
		}

		private unsafe bool SetProperty (XamlCallbackData *data, string xmlns, Value* target_ptr, IntPtr target_data, Value* target_parent_ptr, string prop_xmlns, string name, Value* value_ptr, IntPtr value_data)
		{
			string error;
			object target = Value.ToObject (null, target_ptr);

			if (target == null) {
				Console.Error.WriteLine ("target is null: {0} {1} {2}", (IntPtr)target_ptr, name, xmlns);
				return false;
			}

			if (name == null) {
				if (TrySetEnumContentProperty (data, xmlns, target, target_ptr, target_data, value_ptr, value_data))
					return true;
				if (TrySetCollectionContentProperty (xmlns, target, target_ptr, target_data, value_ptr, value_data))
					return true;
				if (TrySetObjectTextProperty (data, xmlns, target, target_ptr, target_data, value_ptr, value_data))
					return true;
				Console.Error.WriteLine ("no property name supplied");
				return false;
			}

			string full_name = name;
			int dot = name.IndexOf ('.');
			string type_name = null;

			if (dot >= 0) {
				type_name = name.Substring (0, dot);
				if (xmlns != null) {
					string ns = ClrNamespaceFromXmlns (xmlns);
					if (ns != null)
						type_name = String.Concat (ns, ".", type_name);
				}
				name = name.Substring (++dot, name.Length - dot);
			}

			if (TrySetExpression (data, xmlns, target, target_data, target_parent_ptr, type_name, prop_xmlns, name, full_name, value_ptr, value_data))
				return true;

			if (!IsAttachedProperty (data, target, xmlns, prop_xmlns, full_name)) {
				if (TrySetPropertyReflection (data, xmlns, target, target_data, target_parent_ptr, type_name, name, value_ptr, value_data, out error))
					return true;

				if (TrySetEventReflection (data, xmlns, target, type_name, name, value_ptr, out error))
					return true;
			} else {
				if (TrySetAttachedProperty (data, xmlns, target, target_data, prop_xmlns, full_name, value_ptr))
					return true;
			}

			
			return false;
		}

		private unsafe bool AddChild (XamlCallbackData *data, Value* parent_parent_ptr, bool parent_is_property, string parent_xmlns, Value *parent_ptr, IntPtr parent_data, Value* child_ptr, IntPtr child_data)
		{
			object parent_parent = Value.ToObject (null, parent_parent_ptr);
			object parent = Value.ToObject (null, parent_ptr);
			object child = Value.ToObject (null, child_ptr);

			if (parent_is_property)
				return AddChildToProperty (data, parent_parent, parent_xmlns, parent, child, child_data);

			return AddChildToItem (data, parent_parent_ptr, parent, parent_data, child_ptr, child, child_data);
		}

		private unsafe bool AddChildToProperty (XamlCallbackData *data, object parent_parent, string parent_xmlns, object parent, object child, IntPtr child_data)
		{
			string full_prop_name = parent as string;

			if (full_prop_name == null) {
				Console.Error.WriteLine ("Attempting to add child to non string parent {0} as a property.", parent);
				return false;
			}

			
			int dot = full_prop_name.IndexOf ('.');
			if (dot < 1)
				return false;
			string type_name = full_prop_name.Substring (0, dot);
			string prop_name = full_prop_name.Substring (++dot, full_prop_name.Length - dot);

			Type target_type = TypeFromString (data, parent_xmlns, type_name);

			if (target_type == null) {
				Console.Error.WriteLine ("Type '{0}' with xmlns '{1}' could not be found", type_name, parent_xmlns);
				return false;
			}

			if (!target_type.IsAssignableFrom (parent_parent.GetType ())) {
				// This would happen with an attached property, we don't need to do anything here....do we?
				return false;
			}

			PropertyInfo pi = parent_parent.GetType ().GetProperty (prop_name, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

			if (pi == null) {
				Console.Error.WriteLine ("Property does not exist. {0}", prop_name);
				return false;
			}

			if (typeof (ResourceDictionary).IsAssignableFrom (pi.PropertyType) && !(child is ResourceDictionary)) {
				ResourceDictionary the_dict = (ResourceDictionary) pi.GetValue (parent_parent, null);
				string key_name = NativeMethods.xaml_get_element_key (data->parser, child_data);

				if (key_name == null) {
					throw new XamlParseException (2034, "Elements in a ResourceDictionary must have x:Key or x:Name attribute.");
				}

				if (the_dict == null) {
					the_dict = (ResourceDictionary) Activator.CreateInstance (pi.PropertyType);
					if (the_dict == null) {
						Console.Error.WriteLine ("Unable to create instance of dictionary: " + pi.PropertyType);
						return false;
					}
					pi.SetValue (parent_parent, the_dict, null);
				}

				try {
					the_dict.Add (key_name, child);
					if (child is DependencyObject && parent_parent is DependencyObject && !(the_dict is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject) child).native, ((DependencyObject) parent_parent).native);
					}

					return true;
				} catch (ArgumentException) {
					throw new XamlParseException (2273, "Elements in the same ResourceDictionary cannot have the same x:Key");
				}
			}

			if (typeof (IList).IsAssignableFrom (pi.PropertyType) && !(child is IList)) {
				IList the_list = (IList) pi.GetValue (parent_parent, null);

				if (the_list == null) {
					the_list = (IList) Activator.CreateInstance (pi.PropertyType);
					if (the_list == null) {
						Console.Error.WriteLine ("Unable to create instance of list: " + pi.PropertyType);
						return false;
					}
					pi.SetValue (parent_parent, the_list, null);
				}

				try {
					the_list.Add (child);

					if (child is DependencyObject && parent_parent is DependencyObject && !(the_list is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject)child).native, ((DependencyObject)parent_parent).native);
					}

					return true;
				}
				catch (Exception e) {
					Console.WriteLine (e);
					return false;
				}
			}

			return true;
		}

		private unsafe bool AddChildToItem (XamlCallbackData *data, Value *parent_parent_ptr, object parent, IntPtr parent_data, Value *child_ptr, object child, IntPtr child_data)
		{
			ResourceDictionary the_dict = parent as ResourceDictionary;
			if (the_dict != null) {
				string key_name = NativeMethods.xaml_get_element_key (data->parser, child_data);

				if (key_name == null) {
					Console.Error.WriteLine ("Attempting to add item to a resource dictionary without an x:Key or x:Name");
					throw new XamlParseException (-1, -1, "You must specify an x:Key or x:Name for elements in a ResourceDictionary");
				}

				try {
					the_dict.Add (key_name, child);
					if (child is DependencyObject && parent is DependencyObject && !(the_dict is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject) child).native, ((DependencyObject) parent).native);
					}

					return true;
				} catch (Exception e) {
					// Fall through to string
					Console.Error.WriteLine (e);
					return false;
				}
			}

			IList the_list = parent as IList;
			if (the_list != null) {

				try {
					the_list.Add (child);

					if (child is DependencyObject && parent is DependencyObject && !(the_list is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject)child).native, ((DependencyObject)parent).native);
					}

					return true;
				}
				catch {
					return false;
				}
			}

			Type parent_type = parent.GetType ();
			PropertyInfo pi = GetContentProperty (parent_type);

			if (pi == null) {
				Console.Error.WriteLine ("Unable to find content property on type {0}", parent_type);
				return false;
			}

			//
			// Is the content property a collection
			//
			if (typeof (IList).IsAssignableFrom (pi.PropertyType) && !(child is IList)) {
				the_list = (IList) pi.GetValue (parent, null);

				if (the_list == null) {
					the_list = (IList) Activator.CreateInstance (pi.PropertyType);
					if (the_list == null) {
						Console.Error.WriteLine ("Unable to create instance of list: " + pi.PropertyType);
						return false;
					}
					pi.SetValue (parent, the_list, null);
				}

				try {
					the_list.Add (child);

					if (child is DependencyObject && parent is DependencyObject && !(the_list is DependencyObject)) {
						NativeMethods.dependency_object_set_parent (((DependencyObject)child).native, ((DependencyObject)parent).native);
					}
					return true;
				}
				catch {
					return false;
				}
 			}

			string error;

			try {
				return SetPropertyFromValue (data, parent, parent_data, parent_parent_ptr, pi, child_ptr, child_data, out error);
			} catch {
				throw new XamlParseException (2010, String.Format ("{0} does not support {1} as content.", parent, child));
			}
		}

		private unsafe Type LookupType (Value* top_level, string assembly_name, string full_name)
		{
			Type res = null;

			if (assembly_name != null) {

				// if we're given an explicit assembly
				// name, try and load it, then get the
				// type from just that assembly

				Assembly assembly = null;
				if (LoadAssembly (assembly_name, out assembly) == AssemblyLoadResult.Success) {
					res = assembly.GetType (full_name);
					if (res != null)
						return res;
				}
				else {
					Console.Error.WriteLine ("unable to load assembly for target type.");
				}
			}
			else {

				// if we're not given an explicit
				// assembly name, loop over all
				// assemblies specified in
				// Deployment.Parts looking for the
				// type.
				foreach (Assembly a in Deployment.Current.Assemblies) {
					res = a.GetType (full_name);
					if (res != null)
						return res;
				}

				Assembly assembly = typeof (DependencyObject).Assembly;
				res = assembly.GetType (full_name);
				if (res != null && res.IsPublic)
					return res;

			}

			return Application.GetComponentTypeFromName (full_name);
		}

		private unsafe void SetCLRPropertyFromString (XamlCallbackData *data, IntPtr target_data, object target, PropertyInfo pi, string value, out string error, out IntPtr unmanaged_value)
		{
			unmanaged_value = IntPtr.Zero;
			error = null;

			object new_value = null;
			bool do_set = true;

			try {
				if (IsExplicitNull (value)) {
					Type t = pi.PropertyType;
					if (t.IsValueType && !(t.IsGenericType && t.GetGenericTypeDefinition () == typeof (Nullable<>))) {
						error = "Unable to set non nullable type to null.";
						return;
					}
					new_value = null;
				} else
					new_value = MoonlightTypeConverter.ConvertObject (pi, value, target.GetType ());
			} catch {
				do_set = false;
			}

			if (do_set) {
				try {
					SetValue (data, target_data, pi, target, new_value); 
					return;
				} catch (Exception ex) {
					error = ex.Message;
					return;
				}
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
		private unsafe Type TypeFromString (XamlCallbackData *data, string str)
		{
			string assembly_name = null;
			string full_name = str;

			int ps = str.IndexOf (':');
			if (ps > 0) {
				string xmlns = NativeMethods.xaml_uri_for_prefix (data->parser, str.Substring (0, ps));
				string name = str.Substring (ps + 1, str.Length - ps -1);

				return TypeFromString (data, xmlns, name);
			}

			return LookupType (data->top_level, assembly_name, full_name);
		}

		private unsafe Type TypeFromString (XamlCallbackData *data, string xmlns, string name)
		{
			string clr_namespace = ClrNamespaceFromXmlns (xmlns);
			string assembly_name = AssemblyNameFromXmlns (xmlns);

			string full_name = string.IsNullOrEmpty (clr_namespace) ? name : clr_namespace + "." + name;

			return LookupType (data->top_level, assembly_name, full_name);
		}

		private unsafe DependencyProperty DependencyPropertyFromString (XamlCallbackData *data, object otarget, Value* target_parent_ptr, string str_value)
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

				target_type = TypeFromString (data, type_name);
			}
			
			Types.Ensure (target_type);

			ManagedType mt = Deployment.Current.Types.Find (target_type);
			DependencyProperty dp = DependencyProperty.Lookup ((Kind) mt.native_handle, str_value);

			return dp;
		}
		
		private unsafe bool SetPropertyFromValue (XamlCallbackData *data, object target, IntPtr target_data, Value* target_parent_ptr, PropertyInfo pi, Value* value_ptr, IntPtr value_data, out string error)
		{
			error = null;
			object obj_value = Value.ToObject (null, value_ptr);
			
			if (pi.GetCustomAttributes (typeof (SetPropertyDelayedAttribute), true).Length > 0) {
				if ((data->flags & XamlCallbackFlags.SettingDelayedProperty) == 0) {
					Value v = *value_ptr;
					NativeMethods.xaml_delay_set_property (data->parser, target_data, null, pi.Name, ref v);
					return true;
				}
			}

			if (obj_value is Binding && target is FrameworkElement) {
				FrameworkElement fe = (FrameworkElement) target;
				fe.SetBinding (DependencyProperty.Lookup (fe.GetKind (), pi.Name), (Binding) obj_value);
				return true;
			}

			if (obj_value is StaticResource) {
				StaticResource sr = (StaticResource)obj_value;
				obj_value = "{StaticResource " + sr.ResourceKey + "}";
			}

			if (typeof (IList).IsAssignableFrom (pi.PropertyType) && !(obj_value is IList)) {
				// This case is handled in the AddChild code
				return true;
			}

			if (typeof (ResourceDictionary).IsAssignableFrom (pi.PropertyType) && !(obj_value is ResourceDictionary)) {
				// This case is handled in the AddChild code
				return true;
			}

			string str_value = obj_value as string;
			if (str_value != null) {
				IntPtr unmanaged_value;
				
				//
				// HACK: This really shouldn't be here, but I don't want to bother putting it in Helper, because that
				// code probably should be moved into this file
				//
				if (pi.PropertyType == typeof (Type)) {
					Type t = TypeFromString (data, str_value);
					if (t != null) {
						SetValue (data, target_data, pi, target, t);
						return true;
					}
				}

				if (pi.PropertyType == typeof (DependencyProperty)) {
					DependencyProperty dp = DependencyPropertyFromString (data, target, target_parent_ptr, str_value);
					if (dp != null) {
						SetValue (data, target_data, pi, target, dp);
						return true;
					}
				}

				if (typeof (System.Windows.Data.Binding).IsAssignableFrom (pi.PropertyType) && MarkupExpressionParser.IsBinding (str_value)) {
					MarkupExpressionParser p = new MarkupExpressionParser (null, pi.Name,  data->parser, target_data);

					string expression = str_value;
					obj_value = p.ParseExpression (ref expression);

					if (!(obj_value is Binding))
						return false;

					SetValue (data, target_data, pi, target, obj_value);
					return true;
				}

				if (MarkupExpressionParser.IsStaticResource (str_value)) {
					// FIXME: The NUnit tests show we need to use the parent of the target to resolve
					// the StaticResource, but are there any cases where we should use the actual target?
					DependencyObject parent = Value.ToObject (null, target_parent_ptr) as DependencyObject;
					if (parent == null)
						return false;
					MarkupExpressionParser p = new MarkupExpressionParser (parent, "", data->parser, target_data);
					obj_value = p.ParseExpression (ref str_value);

					obj_value = ConvertType (pi, pi.PropertyType, obj_value);

					SetValue (data, target_data, pi, target, obj_value);
					return true;
				}

				SetCLRPropertyFromString (data, target_data, target, pi, str_value, out error, out unmanaged_value);

				if (error == null && unmanaged_value != IntPtr.Zero)
					obj_value = Value.ToObject (null, unmanaged_value);
				else
					return error == null;
			} else {
				obj_value = Value.ToObject (pi.PropertyType, value_ptr);
			}

			obj_value = ConvertType (pi, pi.PropertyType, obj_value);
			SetValue (data, target_data, pi, target, obj_value);

			return true;
		}

		private static unsafe void SetValue (XamlCallbackData *data, IntPtr target_data, PropertyInfo pi, object target, object value)
		{
			SetterBase sb = target as SetterBase;
			
			if (sb != null)
				sb.IsSealed = false;

			try {
				if (NativeMethods.xaml_is_property_set (data->parser, target_data, pi.Name))
					throw new XamlParseException (2033, String.Format ("Cannot specify the value multiple times for property: {0}.", pi.Name));

				pi.SetValue (target, value, null);

				NativeMethods.xaml_mark_property_as_set (data->parser, target_data, pi.Name);

			} finally {
				if (sb != null)
					sb.IsSealed = true;
			}
		}

		private static object ConvertType (MemberInfo pi, Type t, object value)
		{
			if (value == null)
				return null;

			Type valueType = value.GetType ();
			if (valueType == t)
				return value;

			try {
				if (t.IsEnum) {
					string str_value = value as string;
					if (str_value != null)
						return Enum.Parse (t, str_value, true);
					if (Enum.IsDefined (t, value))
						return Enum.ToObject (t, value);
				}
			} catch {
			}

			TypeConverter converter = Helper.GetConverterFor (pi, t);
			if (converter == null) {
				try {
					converter = new MoonlightTypeConverter (pi == null ? null : pi.Name, t);
				} catch {
					converter = null;
				}
			}

			if (converter != null && converter.CanConvertFrom (value.GetType ()))
				return converter.ConvertFrom (value);

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

			int start = xmlns.IndexOf ("clr-namespace:");

			if (start < 0)
				return null;
			start += "clr-namespace:".Length;

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

		private static bool ValidateXmlns (string xmlns)
		{
			Uri dummy = null;
			if (Uri.TryCreate (xmlns, UriKind.Absolute, out dummy))
				return true;

			int start = xmlns.IndexOf ("clr-namespace");
			int end = start + "clr-namespace".Length;
			if (end >= xmlns.Length || xmlns [end] != ':')
				return false;

			start = xmlns.IndexOf ("assembly");
			if (start > 0) {
				end = start + "assembly".Length;
				if (end >= xmlns.Length || xmlns [end] != '=')
					return false;
			}

			return true;
		}

		
		private unsafe MethodInfo GetSetMethodForAttachedProperty (Value *top_level, string xmlns, string type_name, string full_type_name, string prop_name)
		{
			return GetMethodForAttachedProperty (top_level, xmlns, type_name, full_type_name, prop_name, "Set");
		}

		private unsafe MethodInfo GetGetMethodForAttachedProperty (Value *top_level, string xmlns, string type_name, string full_type_name, string prop_name)
		{
			return GetMethodForAttachedProperty (top_level, xmlns, type_name, full_type_name, prop_name, "Get");
		}

		private unsafe MethodInfo GetMethodForAttachedProperty (Value *top_level, string xmlns, string type_name, string full_type_name, string prop_name, string method_prefix)
		{
			string assembly_name = AssemblyNameFromXmlns (xmlns);
			string ns = ClrNamespaceFromXmlns (xmlns);
				
			if (assembly_name == null && !TryGetDefaultAssemblyName (top_level, out assembly_name)) {
				Console.Error.WriteLine ("Unable to find an assembly to load type from.");
				return null;
			}

			Assembly clientlib;
			if (LoadAssembly (assembly_name, out clientlib) != AssemblyLoadResult.Success) {
				Console.Error.WriteLine ("couldn't load assembly:  {0}   namespace:  {1}", assembly_name, ns);
				return null;
			}

			Type attach_type = clientlib.GetType (full_type_name, false);
			if (attach_type == null) {
				attach_type = Application.GetComponentTypeFromName (type_name);
				if (attach_type == null) {
					Console.Error.WriteLine ("attach type is null type name: {0} full type name: {1}", type_name, full_type_name);
					return null;
				}
			}

			MethodInfo set_method = attach_type.GetMethod (String.Concat (method_prefix, prop_name), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
			return set_method;
		}

		private static unsafe object GetObjectValue (object target, IntPtr target_data, string prop_name, IntPtr parser, Value* value_ptr, out string error)
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

		private static bool IsExplicitNull (string value)
		{
			 return Regex.IsMatch (value, "^{\\s*x:Null\\s*}");
		}

		private PropertyInfo GetContentProperty (Type t)
		{
			Type walk = t;
			string content_property = null;

			while (walk != null) {
				content_property = GetContentPropertyNameForType (walk);
				if (content_property != null)
					break;
				walk = walk.BaseType;
			}

			if (walk == null || content_property == null)
				return null;

			PropertyInfo pi = walk.GetProperty (content_property, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);
			return pi;
		}

		private string GetContentPropertyNameForType (Type t)
		{
			object [] o = t.GetCustomAttributes (typeof (ContentPropertyAttribute), false);
			
			if (o.Length == 0)
				return null;

			ContentPropertyAttribute cpa = (ContentPropertyAttribute ) o [0];
			return cpa.Name;
		}

		private string GetContentPropertyName (Type t)
		{
			object [] o = t.GetCustomAttributes (typeof (ContentPropertyAttribute), true);
			if (o.Length == 0)
				return null;
			ContentPropertyAttribute cpa = (ContentPropertyAttribute ) o [0];

			return cpa.Name;
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
		private unsafe bool cb_lookup_object (XamlCallbackData *data, Value* parent, string xmlns, string name, bool create, bool is_property, out Value value, ref MoonError error)
		{
			try {
				return LookupObject (data->top_level, parent, xmlns, name, create, is_property, out value);
			} catch (Exception ex) {
				Console.Error.WriteLine ("ManagedXamlLoader::LookupObject ({0}, {1}, {2}, {3}) failed: {3} ({4}).", (IntPtr) data->top_level, xmlns, create, name, ex.Message, ex.GetType ().FullName);
				Console.WriteLine (ex);
				value = Value.Empty;
				error = new MoonError (ex);
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
		private unsafe bool cb_set_property (XamlCallbackData *data, string xmlns, Value* target, IntPtr target_data, Value* target_parent, string prop_xmlns, string name, Value* value_ptr, IntPtr value_data, ref MoonError error)
		{
			try {
				return SetProperty (data, xmlns, target, target_data, target_parent, prop_xmlns, name, value_ptr, value_data);
			} catch (Exception ex) {
				try {
					Console.Error.WriteLine ("ManagedXamlLoader::SetProperty ({0}, {1}, {2}, {3}, {4}) threw an exception: {5}.", (IntPtr) data->top_level, xmlns, (IntPtr)target, name, (IntPtr)value_ptr, ex.Message);
					Console.Error.WriteLine (ex);
					error = new MoonError (ex);
					return false;
				}
				catch {
					return false;
				}
			}
		}

		private unsafe bool cb_import_xaml_xmlns (XamlCallbackData *data, string xmlns, ref MoonError error)
		{
			try {
				if (!ValidateXmlns (xmlns))
					return false;
				Application.ImportXamlNamespace (xmlns);
				return true;
			} catch (Exception ex) {
				Console.WriteLine ("Application::ImportXamlNamespace ({0}) threw an exception:\n{1}", xmlns, ex);
				error = new MoonError (ex);
				return false;
			}

		}

		private unsafe string cb_get_content_property_name (XamlCallbackData *data, Value* object_ptr, ref MoonError error)
		{
			object obj = Value.ToObject (null, object_ptr);

			if (obj == null)
				return null;

			Type t = obj.GetType ();
			return GetContentPropertyName (t);
			
		}

		private unsafe bool cb_add_child (XamlCallbackData *data, Value* parent_parent, bool parent_is_property, string parent_xmlns, Value *parent, IntPtr parent_data, Value* child, IntPtr child_data, ref MoonError error)
		{
			try {
				return AddChild (data, parent_parent, parent_is_property, parent_xmlns, parent, parent_data, child, child_data);
			} catch (Exception ex) {
				Console.Error.WriteLine (ex);
				error = new MoonError (ex);
				return false;
			}
				
		}

#endregion
	}
}
