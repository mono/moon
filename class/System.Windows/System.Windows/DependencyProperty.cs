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

namespace System.Windows {
	public class DependencyProperty {
		internal IntPtr native;
		internal Type property_type;
		private Kind declaring_type;
		
		public static readonly object UnsetValue = new object ();
		
		static DependencyProperty ()
		{
			NativeMethods.runtime_init (0);
		}
		
		internal DependencyProperty ()
		{
			// useless constructor.
		}

		internal DependencyProperty (IntPtr handle, Type property_type, Kind owner_type)
		{
			this.native = handle;
			this.property_type = property_type;
			this.declaring_type = owner_type;
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
			Surface surface = Mono.Xaml.XamlLoader.SurfaceObjectInDomain;
			ManagedType property_type = surface.FindType (propertyType);
			ManagedType owner_type = surface.FindType (ownerType);
			
			IntPtr handle = NativeMethods.dependency_property_register_managed_property (surface.Native, name, property_type.native_handle, owner_type.native_handle, attached);
			
			if (handle == IntPtr.Zero)
				return null;
			
			return new CustomDependencyProperty (handle, name, property_type, owner_type, metadata);
		}
		
		
		internal static DependencyProperty Lookup (Kind type, string name, Type ownerType)
		{
			IntPtr handle = NativeMethods.dependency_property_lookup (type, name);

			if (handle == IntPtr.Zero)
				throw new Exception (
					String.Format ("DependencyProperty.Lookup: {0} lacks {1}. This is normally because System.Windows.dll or Mono.Moonlight.dll and libmoon is out of sync. Update /moon and do 'make install' in /moon/src and 'make clean install' TWICE in /moon/class/ will probably fix it.", type, name));

			if (ownerType == null)
				throw new ArgumentNullException ("ownerType");	
			
			return new DependencyProperty (handle, ownerType, type);
		}
		
		internal string Name {
			get { return NativeMethods.dependency_property_get_name (native); }
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
		
		internal Kind DeclaringType {
			get { return declaring_type; }
		}
		
		internal Type Type {
			get { return property_type; }
		}
	}
}
