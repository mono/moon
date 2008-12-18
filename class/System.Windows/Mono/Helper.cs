//
// Helper.cs: Exposes some methods that require access to mscorlib or
// System but are not exposed in the 2.1 profile.   This is necessary
// to avoid making moonlight.exe a friend of the System and mscorlib
// assemblies.
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
using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.IO;

namespace Mono {

	internal static class Helper {

		public static Assembly Agclr { get; set; }

		private static TypeConverter GetConverterFor (MemberInfo info, Type target_type)
		{
			Attribute[] attrs = (Attribute[])info.GetCustomAttributes (true);
			TypeConverterAttribute at = null;
			TypeConverter converter = null;

			foreach (Attribute attr in attrs) {
				if (attr is TypeConverterAttribute) {
					at = (TypeConverterAttribute)attr;
					break;
				}
			}

			if (at == null)
				return null;

			Type t = Type.GetType (at.ConverterTypeName);
			if (t == null)
					return null;

			ConstructorInfo ci = t.GetConstructor (new Type[] { typeof(Type) });
			if (ci != null)
				converter = (TypeConverter) ci.Invoke (new object[] { target_type });
			else
				converter = (TypeConverter) Activator.CreateInstance (t);

			return converter;
		}

		static bool IsAssignableToIConvertible (Type type)
		{
			return typeof (IConvertible).IsAssignableFrom (type);
		}

		public static object ValueFromString (Type value_type, string value, string prop_name, out string error, out IntPtr unmanaged_value)
		{
			unmanaged_value = IntPtr.Zero;
			error = null;

			TypeConverter converter = GetConverterFor (value_type, value_type);
			if (converter != null && converter.CanConvertFrom (typeof (string))) {
				return converter.ConvertFrom (value);
			}

			if (IsAssignableToIConvertible (value_type)) {
				object res = ValueFromConvertible (value_type, value);
				if (res != null)
					return res;
			}

			//
			// XXX Add Special case for System.Type (see SetPropertyFromString)
			//

			unmanaged_value = NativeMethods.value_from_str_with_typename (TypeToMoonType (value_type), prop_name, value);
			if (unmanaged_value == IntPtr.Zero)
				error = string.Format ("unable to convert to type {0} from a string", value_type);

			return null;
		}

		public static void SetPropertyFromString (object target, PropertyInfo pi, string value, out string error, out IntPtr unmanaged_value)
		{
			unmanaged_value = IntPtr.Zero;
			error = null;

			// if the property has a TypeConverter
			// associated with it (or the property's type
			// does), try to use that first
			TypeConverter converter = GetConverterFor (pi, pi.PropertyType);
			if (converter != null && converter.CanConvertFrom (typeof (string))) {
				try {
					pi.SetValue (target, converter.ConvertFrom (value), null);
				} catch (Exception e) {
					error = e.ToString ();
				}
				return;
			}

			//
			// If the property is a simple IConvertible type we might
			// be able to just convert it in managed code.
			//
			if (IsAssignableToIConvertible (pi.PropertyType)) {
				object res = ValueFromConvertible (pi.PropertyType, value);
				if (res != null) {
					try {
						pi.SetValue (target, res, null);
					} catch (Exception e) {
						error = e.ToString ();
					}
					return;
				}
			}
			else if (pi.PropertyType == typeof (object)) {
				try {
					pi.SetValue (target, (object)value, null);
				} catch (Exception e) {
					error = e.ToString ();
				}
				return;
			}

			// special case System.Type properties (like
			// Style.TargetType and
			// ControlTemplate.TargetType)
			//
			// XXX this isn't working.
			//
			if (pi.PropertyType == typeof (Type)) {

				// try to find the type based on the name
				Type app = Agclr.GetType ("System.Windows.Application");
				MethodInfo lookup = app.GetMethod ("GetComponentTypeFromName", BindingFlags.NonPublic | BindingFlags.Static, null, new Type [] {typeof (string)}, null);
				Type t = (Type)lookup.Invoke (null, new object [] {value});
				
				if (t != null) {
					try {
						pi.SetValue (target, t, null);
					} catch (Exception e) {
						error = e.ToString ();
					}
					return;
				}
			}

			//
			// lastly, attempt to create an unmanaged Value* object, if one is created, the managed
			// parser will create a managed wrapper for the object and call SetPropertyFromValue with
			// the managed object
			//
			unmanaged_value = NativeMethods.value_from_str_with_typename (TypeToMoonType (pi.PropertyType), pi.Name, value);
			if (unmanaged_value == IntPtr.Zero)
				error = string.Format ("unable to convert to type {0} from a string", pi.PropertyType);
		}

		public static void SetPropertyFromValue (object target, PropertyInfo pi, object value, out string error)
		{
			error = null;
			try {
				pi.SetValue (target, value, null);
			} catch (Exception e) {
				error = e.ToString ();
			}
		}

		public static object ChangeType (object obj, Type type)
		{
			return Convert.ChangeType (obj, type);
		}

		public static string GetStackTrace ()
		{
			return Environment.StackTrace;
		}

		public static object CreateInstance (Type type, bool nonPublic)
		{
			return Activator.CreateInstance (type, nonPublic);
		}

		public static Assembly LoadFile (string path)
		{
			return Assembly.LoadFile (path);
		}

		public static AssemblyName [] GetReferencedAssemblies (Assembly ass)
		{
			return ass.GetReferencedAssemblies ();
		}

		public static IntPtr AllocHGlobal (int cb)
		{
			return Marshal.AllocHGlobal (cb);
		}

		public static GCHandle GCHandleFromIntPtr (IntPtr ptr)
		{
			return GCHandle.FromIntPtr (ptr);
		}

		public static IntPtr GCHandleToIntPtr (GCHandle handle)
		{
			return GCHandle.ToIntPtr (handle);
		}
		
		public static void FreeHGlobal (IntPtr ptr)
		{
			Marshal.FreeHGlobal (ptr);
		}

		public static string PtrToStringAuto (IntPtr ptr)
		{
			return Marshal.PtrToStringAuto (ptr);
		}

		public static void ThreadMemoryBarrier ()
		{
			Thread.MemoryBarrier ();
		}

		/// <summary>
		/// Looks up a dependency object given the native pointer.
		/// The calling code must free the native pointer when it's finished with it.
		/// </summary>
		public static object LookupDependencyObject (Kind k, IntPtr ptr)
		{
			Type depobj = Agclr.GetType ("System.Windows.DependencyObject");
			MethodInfo lookup = depobj.GetMethod ("Lookup", BindingFlags.NonPublic | BindingFlags.Static, null, new Type [] {typeof (Kind), typeof (IntPtr)}, null);
			return lookup.Invoke (null, new object [] {k, ptr});
		}
		
		public static IntPtr GetNativeObject (object dependency_object)
		{
			if (dependency_object == null)
				return IntPtr.Zero;
			
			Type depobj = Agclr.GetType ("System.Windows.DependencyObject");
			FieldInfo field = depobj.GetField ("_native", BindingFlags.Instance | BindingFlags.NonPublic);
			return (IntPtr) field.GetValue (dependency_object);
		}

		public static void DeleteDirectory (string path)
		{
			Directory.Delete (path, true);
		}

		private static object ValueFromConvertible (Type type, string value)
		{
			if (type == typeof (string))
				return value;
			if (type == typeof (bool))
				return Convert.ToBoolean (value);
			if (type == typeof (byte))
				return Convert.ToByte (value);
			if (type == typeof (char))
				return Convert.ToChar (value);
			if (type == typeof (DateTime))
				return Convert.ToDateTime (value);
			if (type == typeof (Decimal))
				return Convert.ToDecimal (value);
			if (type == typeof (double))
				return Convert.ToDouble (value);
			if (type == typeof (Int16))
				return Convert.ToInt16 (value);
			if (type == typeof (Int32))
				return Convert.ToInt32 (value);
			if (type == typeof (Int64))
				return Convert.ToInt64 (value);
			if (type == typeof (SByte))
				return Convert.ToSByte (value);
			if (type == typeof (Single))
				return Convert.ToSingle (value);
			if (type == typeof (UInt16))
				return Convert.ToUInt16 (value);
			if (type == typeof (UInt32))
				return Convert.ToUInt32 (value);
			if (type == typeof (UInt64))
				return Convert.ToUInt64 (value);
			return null;
		}

		private static string TypeToMoonType (Type t)
		{
			if (t == typeof (double))
				return "double";
			if (t == typeof (bool))
				return "bool";
			return t.Name;
		}
	}
}
