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
		internal Type type;
		private Kind declaring_type;
		
		static DependencyProperty ()
		{
			NativeMethods.runtime_init (0);
		}
		
		public DependencyProperty ()
		{
			// useless constructor.
		}

		internal DependencyProperty (IntPtr handle, Type t, Kind dtype)
		{
			native = handle;
			type = t;
			declaring_type = dtype;
		}
		internal static DependencyProperty Lookup (Kind type, string name, Type ownerType)
		{
			IntPtr handle = NativeMethods.dependency_property_lookup (type, name);

			if (handle == IntPtr.Zero)
				throw new Exception (
					String.Format ("DependencyProperty.Lookup: {0} lacks {1}. This is normally because agclr.dll or agmono.dll and libmoon is out of sync. Updating /moon/src and /olive/class/, and do 'make install' in /moon/src and 'make clean install' TWICE in /olive/class/ will probably fix it. Remember to specify 'PROFILE=net_2_1' if that's your profile, only the net_3_0 profile is built by default.", type, name));

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
		
		internal Kind GetKind {
			get { return NativeMethods.dependency_property_get_value_type (native); }
		}
		
		internal bool IsValueType {
			get { return NativeMethods.type_get_value_type (GetKind); }
		}
		
		internal Kind DeclaringType {
			get { return declaring_type; }
		}		
	}
}
