//
// DependencyProperty.cs
//
// Author:
//   Iain McCoy (iain@mccoy.id.au)
//   Miguel de Icaza (miguel@novell.com)
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
	public class DependencyProperty {
		private string name;
		private IntPtr native;
		private Type property_type;
		private Kind declaring_kind = Kind.INVALID; // This should be obsoleted
		private Type declaring_type; 
		private object default_value;
		
		private static Dictionary <IntPtr, DependencyProperty> properties = new Dictionary<System.IntPtr,DependencyProperty> ();
		
		public static readonly object UnsetValue = new object ();
		
		static DependencyProperty ()
		{
			NativeMethods.runtime_init (0);
		}

		internal DependencyProperty (IntPtr handle, Type property_type, Kind declaring_kind, string name)
		{
			this.native = handle;
			this.property_type = property_type;
			this.declaring_kind = declaring_kind;
			this.name = name;
			
			properties.Add (handle, this);
			
			if (this.property_type.IsValueType)
				this.default_value = Activator.CreateInstance (this.property_type);
			//Console.WriteLine ("DependencyProperty.DependencyProperty ({0:X}, {1}, {2})", handle, property_type.FullName, declaring_kind);
		}
		
		internal DependencyProperty (IntPtr handle, Type property_type, Type declaring_type, string name)
		{
			this.native = handle;
			this.property_type = property_type;
			this.declaring_type = declaring_type;
			this.name = name;
			
			properties.Add (handle, this);
			
			if (this.property_type.IsValueType)
				this.default_value = Activator.CreateInstance (this.property_type);
			//Console.WriteLine ("DependencyProperty.DependencyProperty ({0:X}, {1}, {2})", handle, property_type.FullName, declaring_type.FullName);
		}
		
		public static DependencyProperty Register (string name, Type propertyType, Type ownerType, PropertyMetadata typeMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, typeMetadata, false);
		}
		
		public static DependencyProperty RegisterAttached (string name, Type propertyType, Type ownerType, PropertyMetadata defaultMetadata)
		{
			return RegisterAny (name, propertyType, ownerType, defaultMetadata, true);
		}
		
		private static DependencyProperty RegisterAny (string name, Type propertyType, Type ownerType, PropertyMetadata metadata, bool attached)
		{
			ManagedType property_type = Types.Find (propertyType);
			ManagedType owner_type = Types.Find (ownerType);
			NativePropertyChangedHandler handler;
			
			if (name == null)
				throw new System.ArgumentNullException ("name");
			
			if (name == string.Empty)
				throw new ArgumentException("The 'name' argument cannot be an empty string");
			
			if (propertyType == null)
				throw new System.ArgumentNullException ("propertyType");
			
			if (ownerType == null)
				throw new System.ArgumentNullException ("ownerType");
			
			if (metadata != null)
				handler = new NativePropertyChangedHandler(NativePropertyChangedCallback);
			else
				handler = null;
			IntPtr handle = NativeMethods.dependency_property_register_managed_property (Types.Native, name, property_type.native_handle, owner_type.native_handle, attached, handler);
			
			if (handle == IntPtr.Zero)
				return null;
			
			return new CustomDependencyProperty (handle, name, property_type, owner_type, metadata);
		}
		
		private static void NativePropertyChangedCallback (IntPtr dependency_property, IntPtr dependency_object, IntPtr old_value, IntPtr new_value)
		{		
			DependencyProperty property;
			CustomDependencyProperty custom_property;
			DependencyObject obj = null;
			object old_obj = null, new_obj = null;
			DependencyPropertyChangedEventArgs args;
			
			if (!properties.TryGetValue (dependency_property, out property)) {
				Console.Error.WriteLine ("DependencyProperty.NativePropertyChangedCallback: Couldn't find the managed DependencyProperty corresponding with native {0}", dependency_property);
				return;
			}
			
			custom_property = property as CustomDependencyProperty;
			
			if (custom_property == null) {
				Console.Error.WriteLine ("DependencyProperty.NativePropertyChangedCallback: Got the event for a builtin dependency property.");
				return;
			}
			
			if (custom_property.Metadata == null || custom_property.Metadata.property_changed_callback == null)
				return;				
			
			obj = DependencyObject.Lookup (dependency_object);
		
			
			if (obj == null)
				return;
			
			old_obj = DependencyObject.ValueToObject (property.property_type, old_value);
			new_obj = DependencyObject.ValueToObject (property.property_type, new_value);
			
			if (old_obj == null && property.property_type.IsValueType)
				old_obj = property.DefaultValue;
			
			if (old_obj == null && new_obj == null)
				return; // Nothing changed.
			
			if (old_obj == new_obj)
				return; // Nothing changed
			
			if (old_obj != null && new_obj != null)
				if (object.Equals (old_obj, new_obj))
					return; // Nothing changed
			
			args = new DependencyPropertyChangedEventArgs (old_obj, new_obj, property);
			
			custom_property.Metadata.property_changed_callback (obj, args);
			
		}
		
		internal static DependencyProperty Lookup (Kind type, string name, Type ownerType)
		{
			IntPtr handle = NativeMethods.dependency_property_get_dependency_property (type, name);
			DependencyProperty result;

			if (handle == IntPtr.Zero)
				throw new Exception (
					String.Format ("DependencyProperty.Lookup: {0} lacks {1}. This is normally because System.Windows.dll or Mono.Moonlight.dll and libmoon is out of sync. Update /moon and do 'make install' in /moon/src and 'make clean install' TWICE in /moon/class/ will probably fix it.", type, name));

			if (ownerType == null)
				throw new ArgumentNullException ("ownerType");	
			
			if (properties.TryGetValue (handle, out result))
				return result;
			
			return new DependencyProperty (handle, ownerType, type, name);
		}
		
		internal string Name {
			get { return name; }
		}
		
		internal bool IsNullable {
			get { return NativeMethods.dependency_property_is_nullable (native); }
		}
		
		internal Kind Kind {
			get { return NativeMethods.dependency_property_get_property_type (native); }
		}
		
		internal bool IsValueType {
			get { return NativeMethods.type_get_value_type (this.Kind); }
		}
		
		internal Type DeclaringType {
			get { return declaring_type; }
		}
		
		internal IntPtr Native {
			get { return native; }
		}
		
		internal Type PropertyType {
			get { return property_type; }
		}
		
		internal Kind DeclaringKind {
			get { return declaring_kind; }
		}
		
		internal object DefaultValue {
			get { return default_value; }
		}
	}
}
