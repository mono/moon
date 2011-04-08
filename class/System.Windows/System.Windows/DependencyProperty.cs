//
// DependencyProperty.cs
//
// Author:
//   Iain McCoy (iain@mccoy.id.au)
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
//
// Copyright 2005 Iain McCoy
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
using Mono;
using System.Collections.Generic;

namespace System.Windows {

	internal delegate void ValueValidator (DependencyObject target, DependencyProperty property, object value);
	internal class CoreDependencyProperty : DependencyProperty {
		internal CoreDependencyProperty (IntPtr handle, string name, Type property_type, Type declaring_type) : base (handle, name, property_type, declaring_type) {}
	}
	
	public class DependencyProperty {

		static ValueValidator DefaultValidator = delegate { };
		
		string name;
		IntPtr native;
		Type property_type;
		Type declaring_type; 
		bool? attached;
		bool? hasHiddenDefaultValue;
		ValueValidator validator;
		internal PropertyChangedCallback property_changed_callback {
			get; private set;
		}
		
		static Dictionary <IntPtr, DependencyProperty> properties = new Dictionary<IntPtr, DependencyProperty> ();
		
		public static readonly object UnsetValue = new object ();
		
		internal DependencyProperty (IntPtr handle, string name, Type property_type, Type declaring_type)
		{
			this.native = handle;
			this.property_type = property_type;
			this.declaring_type = declaring_type;
			this.name = name;
			
			properties.Add (handle, this);

			//Console.WriteLine ("DependencyProperty.DependencyProperty ({0:X}, {1}, {2})", handle, property_type.FullName, declaring_type.FullName);
		}
		
		public PropertyMetadata GetMetadata (Type forType)
		{
			var kind = Deployment.Current.Types.TypeToNativeKind (forType);
			var defaultValue = UnsetValue;
			if (!HasHiddenDefaultValue)
				defaultValue = GetDefaultValue (kind) ?? UnsetValue;

			return new PropertyMetadata (defaultValue);
		}
		
		public static DependencyProperty Register (string name, Type propertyType, Type ownerType, PropertyMetadata typeMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, typeMetadata, false, false, true, true);
		}
		
		internal static DependencyProperty RegisterCore (string name, Type propertyType, Type ownerType, PropertyMetadata typeMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, typeMetadata, false, false, true, false);
		}
		
		public static DependencyProperty RegisterAttached (string name, Type propertyType, Type ownerType, PropertyMetadata defaultMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, defaultMetadata, true, false, true, true);
		}
		
		internal static DependencyProperty RegisterAttachedCore (string name, Type propertyType, Type ownerType, PropertyMetadata defaultMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, defaultMetadata, true, false, true, false);
		}

		// internally Silverlight use some read-only properties
		internal static DependencyProperty RegisterReadOnlyAttached (string name, Type propertyType, Type ownerType, PropertyMetadata defaultMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, defaultMetadata, true, true, true, true);
		}

		// internally Silverlight use some read-only properties
		internal static DependencyProperty RegisterReadOnlyCore (string name, Type propertyType, Type ownerType, PropertyMetadata defaultMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, defaultMetadata, true, true, true, false);
		}
		
		private static DependencyProperty RegisterAny (string name, Type propertyType, Type ownerType, PropertyMetadata metadata, bool attached, bool readOnly, bool setsParent, bool custom)
		{
			ManagedType property_type;
			ManagedType owner_type;
			UnmanagedPropertyChangeHandler handler = null;
			CustomDependencyProperty result;
			bool is_nullable = false;
			object default_value = DependencyProperty.UnsetValue;
			PropertyChangedCallback property_changed_callback = null;
			
			if (name == null)
				throw new ArgumentNullException ("name");
			
			if (name.Length == 0)
				throw new ArgumentException("The 'name' argument cannot be an empty string");
			
			if (propertyType == null)
				throw new ArgumentNullException ("propertyType");
			
			if (ownerType == null)
				throw new ArgumentNullException ("ownerType");

			if (propertyType.IsGenericType && propertyType.GetGenericTypeDefinition () == typeof (Nullable<>)) {
				is_nullable = true;
				// Console.WriteLine ("DependencyProperty.RegisterAny (): found nullable {0}, got nullable {1}", propertyType.FullName, propertyType.GetGenericArguments () [0].FullName);
				propertyType = propertyType.GetGenericArguments () [0];
			}
			
			Types types = Deployment.Current.Types;

			property_type = types.Find (propertyType);
			owner_type = types.Find (ownerType);

			if (metadata != null) {
				default_value = metadata.DefaultValue ?? UnsetValue;
				property_changed_callback = metadata.property_changed_callback;
			}

			if ((default_value == DependencyProperty.UnsetValue) && propertyType.IsValueType && !is_nullable)
				default_value = Activator.CreateInstance (propertyType);

			if (default_value != null && default_value != UnsetValue && !propertyType.IsAssignableFrom (default_value.GetType ()))
				throw new ArgumentException (string.Format ("DefaultValue is of type {0} which is not compatible with type {1}", default_value.GetType (), propertyType));

			if (property_changed_callback != null)
				handler = UnmanagedPropertyChangedCallbackSafe;

			Value v;
			if (default_value == DependencyProperty.UnsetValue) {
				v = new Value { k = types.TypeToKind (propertyType), IsNull = true };
				default_value = null;
			} else {
				v = Value.FromObject (default_value, true);
			}

			IntPtr handle;
			using (v) {
				var val = v;
				if (custom)
					handle = NativeMethods.dependency_property_register_custom_property (name, property_type.native_handle, owner_type.native_handle, ref val, attached, readOnly, handler);
				else
					handle = NativeMethods.dependency_property_register_core_property (name, property_type.native_handle, owner_type.native_handle, ref val, attached, readOnly, handler);
			}
			
			if (handle == IntPtr.Zero)
				return null;

			if (is_nullable)
				NativeMethods.dependency_property_set_is_nullable (handle, true);
			
			result = new CustomDependencyProperty (handle, name, property_type, owner_type, default_value);
			result.attached = attached;
			result.PropertyChangedHandler = handler;
			result.property_changed_callback = property_changed_callback;
			
			return result;
		}

		private static void UnmanagedPropertyChangedCallbackSafe (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused)
		{
			try {
				try {
					UnmanagedPropertyChangedCallback (dependency_object,
									  NativeMethods.property_changed_event_args_get_property (propertyChangeArgs),
									  NativeMethods.property_changed_event_args_get_old_value (propertyChangeArgs),
									  NativeMethods.property_changed_event_args_get_new_value (propertyChangeArgs));
				} catch (Exception ex) {
					error = new MoonError (ex);
				}
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in DependencyProperty.UnmanagedPropertyChangedCallback: {0}",
							   ex.Message);
				} catch {
				}
			}
		}
		
		private static void UnmanagedPropertyChangedCallback (IntPtr dependency_object, IntPtr dependency_property, IntPtr old_value, IntPtr new_value)
		{		
			DependencyProperty property;
			CustomDependencyProperty custom_property;
			DependencyObject obj;
			object old_obj, new_obj;
			
			if (!properties.TryGetValue (dependency_property, out property)) {
				Console.Error.WriteLine ("DependencyProperty.UnmanagedPropertyChangedCallback: Couldn't find the managed DependencyProperty corresponding with native {0}", dependency_property);
				return;
			}
			
			custom_property = property as CustomDependencyProperty;
			
			if (custom_property == null) {
				Console.Error.WriteLine ("DependencyProperty.UnmanagedPropertyChangedCallback: Got the event for a builtin dependency property.");
				return;
			}

			// Console.WriteLine ("UnmanagedPropertyChangedCallback {3} {2} {0} {1}", old_value, new_value, custom_property.name, custom_property.property_type.FullName);
			
			if (custom_property.property_changed_callback == null)
				return;				

			obj = NativeDependencyObjectHelper.Lookup (dependency_object) as DependencyObject;
			
			if (obj == null)
				return;
			
			old_obj = Value.ToObject (property.property_type, old_value);
			new_obj = Value.ToObject (property.property_type, new_value);
			
			InvokeChangedCallback (obj, property, custom_property.property_changed_callback, old_obj, new_obj);
		}

		static void InvokeChangedCallback (DependencyObject obj, DependencyProperty property, PropertyChangedCallback callback,
						   object old_obj, object new_obj)
		{
			if (old_obj == null && property.property_type.IsValueType && !property.IsNullable) {
				old_obj = property.GetDefaultValue (obj);
				Console.WriteLine ("WARNING: Got a null value for {0}.{1} which is a value type", property.DeclaringType.Name, property.Name);
			}

			if (Helper.AreEqual (property.PropertyType, old_obj, new_obj))
				return;

			var args = new DependencyPropertyChangedEventArgs (old_obj, new_obj, property);

			// note: since callbacks might throw exceptions but we cannot catch them
			callback (obj, args);
		}

		internal static DependencyProperty Lookup (IntPtr native)
		{
			DependencyProperty prop;
			return properties.TryGetValue (native, out prop) ? prop : null;
		}

		internal static DependencyProperty Lookup (Kind declaring_kind, string name)
		{
			return Lookup (declaring_kind, name, null, false);
		}

		internal static DependencyProperty Lookup (Kind declaring_kind, string name, Type property_type)
		{
			return Lookup (declaring_kind, name, property_type, true);
		}

		private static DependencyProperty Lookup (Kind declaring_kind, string name, Type property_type, bool create)
		{
			DependencyProperty prop;
			if (!LookupInternal (declaring_kind, name, property_type, create, out prop)) {
				throw new Exception (
					String.Format ("DependencyProperty.Lookup: {0} lacks {1}.", Deployment.Current.Types.KindToType (declaring_kind), name));
			}
			return prop;
		}

		private static bool LookupInternal (Kind declaring_kind, string name, Type property_type, bool create, out DependencyProperty property)
		{
			IntPtr handle;

			if (name == null)
				throw new ArgumentNullException ("name");

			if (create && property_type == null)
				throw new ArgumentNullException ("property_type");	
			
			if (declaring_kind == Kind.INVALID)
				throw new ArgumentOutOfRangeException ("declaring_kind");

			if (property_type != null)
				Deployment.Current.Types.Find (property_type);

			handle = NativeMethods.dependency_property_get_dependency_property_full (declaring_kind, name, true);

			if (handle == IntPtr.Zero) {
				property = null;
				return false;
			}
			if (properties.TryGetValue (handle, out property))
				return true;

			if (create) {
				property = new CoreDependencyProperty (handle, name, property_type, Deployment.Current.Types.KindToType (declaring_kind));
			}

			return property != null;
		}

		internal static bool TryLookup (Kind declaring_kind, string name, out DependencyProperty property)
		{
			return LookupInternal (declaring_kind, name, null, false, out property);
		}

		internal PropertyChangedCallback change_cb;

		internal void AddPropertyChangeCallback (PropertyChangedCallback callback)
		{
			if (this is CustomDependencyProperty) {
				Console.WriteLine ("this should really just be done by registering the property with metadata");
				CustomDependencyProperty cdp = (CustomDependencyProperty)this;
				if (cdp.property_changed_callback != null)
					throw new InvalidOperationException ("this DP was registered with a PropertyChangedCallback already");
			}

			if (change_cb != null)
				throw new InvalidOperationException ("this DP already has a change callback registered");

			change_cb = callback;

			NativeMethods.dependency_property_set_property_changed_callback (native,
											 CustomUnmanagedPropertyChangedCallbackSafe);
		}

		private static void CustomUnmanagedPropertyChangedCallbackSafe (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused)
		{
			DependencyProperty property;
			DependencyObject obj;

			try {
				try {
					IntPtr uprop = NativeMethods.property_changed_event_args_get_property (propertyChangeArgs);
					if (!properties.TryGetValue (uprop, out property)) {
						Console.Error.WriteLine ("DependencyProperty.CustomUnmanagedPropertyChangedCallback: Couldn't find the managed DependencyProperty corresponding with native {0}/{1}", uprop, NativeMethods.property_changed_event_args_get_id (propertyChangeArgs));
						return;
					}

					obj = NativeDependencyObjectHelper.Lookup (dependency_object) as DependencyObject;
			
					if (obj == null)
						return;

					InvokeChangedCallback (obj, property, property.change_cb,
					       Value.ToObject (property.PropertyType, NativeMethods.property_changed_event_args_get_old_value (propertyChangeArgs)),
					       Value.ToObject (property.PropertyType, NativeMethods.property_changed_event_args_get_new_value (propertyChangeArgs)));
				} catch (Exception ex) {
					error = new MoonError (ex);
				}
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in DependencyProperty.UnmanagedPropertyChangedCallback: {0}", ex.Message);
				} catch {
				}
			}
		}

		bool HasHiddenDefaultValue {
			get {
				if (!hasHiddenDefaultValue.HasValue)
					hasHiddenDefaultValue = Mono.NativeMethods.dependency_property_get_has_hidden_default_value (native);
				return hasHiddenDefaultValue.Value;
			}
		}

		internal string Name {
			get { return name; }
		}
		
		internal bool IsAttached {
			get {
				if (!attached.HasValue)
					attached = NativeMethods.dependency_property_is_attached (native);
				return attached.Value;
			}
		}

		internal bool IsNullable {
			get { return NativeMethods.dependency_property_is_nullable (native); }
		}
				
		internal bool IsValueType {
			get { return NativeMethods.type_get_value_type (Deployment.Current.Types.TypeToKind (PropertyType)); }
		}
		
		internal Type DeclaringType {
			get { return declaring_type; }
		}
		
		internal object GetDefaultValue (object ob)
		{
			return GetDefaultValue (Deployment.Current.Types.TypeToNativeKind (ob.GetType ()));
		}
		
		internal virtual object GetDefaultValue (Kind kind)
		{
			IntPtr ptr;
			object ret = null;

			// The value from GetDefaultValue does not need to be deleted
			ptr = Mono.NativeMethods.dependency_property_get_default_value (native, kind);
			if (ptr == IntPtr.Zero) {
				// The value from GetAutoCreatedValue does need to be deleted
				ptr = NativeMethods.dependency_property_get_auto_created_value (native, kind, IntPtr.Zero);
				ret = Value.ToObject (ptr);
				NativeMethods.value_delete_value2 (ptr);
			} else {
				ret = Value.ToObject (PropertyType, ptr);
			}

			return ret;
		}

		internal IntPtr Native {
			get { return native; }
		}
		
		internal Type PropertyType {
			get { return property_type; }
		}

		internal bool IsReadOnly {
			get { return NativeMethods.dependency_property_is_read_only (native); }
		}

		internal ValueValidator Validate {
			get { return validator ?? DefaultValidator; }
			set { validator = value; }
		}
	}
}
