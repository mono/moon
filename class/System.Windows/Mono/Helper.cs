//
// Helper.cs: Exposes some methods that require access to mscorlib or
// System but are not exposed in the 2.1 profile.
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
		internal static System.Globalization.CultureInfo DefaultCulture = System.Globalization.CultureInfo.GetCultureInfo ("en-US");
		
		public static TypeConverter GetConverterFor (Type target_type)
		{
			return GetConverterFor (null, target_type);
		}

		public static TypeConverter GetConverterFor (MemberInfo info, Type target_type)
		{
			Attribute[] attrs;
			TypeConverterAttribute at = null;
			TypeConverter converter = null;
			Type t = null;

			// first check for a TypeConverter attribute on the property
			if (info != null) {
				attrs = (Attribute[])info.GetCustomAttributes (true);
				foreach (Attribute attr in attrs) {
					if (attr is TypeConverterAttribute) {
						at = (TypeConverterAttribute)attr;
						break;
					}
				}
			}

			if (at == null) {
				// we didn't find one on the property.
				// check for one on the Type.
				attrs = (Attribute[])target_type.GetCustomAttributes (true);
				foreach (Attribute attr in attrs) {
					if (attr is TypeConverterAttribute) {
						at = (TypeConverterAttribute)attr;
						break;
					}
				}
			}

			if (at == null) {
				if (target_type == typeof (bool?)) {
					t = typeof (NullableBoolConverter);
				} else {
					return null;
				}
			} else {
				t = Type.GetType (at.ConverterTypeName);
			}

			if (t == null)
				return null;

			ConstructorInfo ci = t.GetConstructor (new Type[] { typeof(Type) });
			if (ci != null)
				converter = (TypeConverter) ci.Invoke (new object[] { target_type });
			else
				converter = (TypeConverter) Activator.CreateInstance (t);

			return converter;
		}

		public static bool IsAssignableToIConvertible (Type type)
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

			bool result = NativeMethods.value_from_str_with_typename (TypeToMoonType (value_type), prop_name, value, out unmanaged_value);
			if (!result)
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
					pi.SetValue (target, converter.ConvertFrom (null, DefaultCulture, value), null);
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

			if (pi.PropertyType.IsEnum) {
				try {
					pi.SetValue (target, Enum.Parse (pi.PropertyType, value), null);
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
				Type t = Application.GetComponentTypeFromName (value);

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
			bool result = NativeMethods.value_from_str_with_typename (TypeToMoonType (pi.PropertyType), pi.Name, value, out unmanaged_value);
			if (!result) {
				error = string.Format ("unable to convert to type {0} from a string", pi.PropertyType);
			}
		}

		public static string GetStackTrace ()
		{
			return Environment.StackTrace;
		}

		public static void ThreadMemoryBarrier ()
		{
			Thread.MemoryBarrier ();
		}

		public static IntPtr AllocHGlobal (int cb)
		{
			return Marshal.AllocHGlobal (cb);
		}

		static MethodInfo inDomain;

		public static bool GCHandleInDomain (IntPtr ptr)
		{
			if (inDomain == null) {
				inDomain = typeof(GCHandle).GetMethod ("CheckCurrentDomain", BindingFlags.Static | BindingFlags.NonPublic);
				if (inDomain == null)
					throw new Exception ("argh");
			}

			return (bool)inDomain.Invoke (null, new object[1] { (int)ptr });
		}

		public static GCHandle GCHandleFromIntPtr (IntPtr ptr)
		{
			return GCHandle.FromIntPtr (ptr);
		}

		public static IntPtr GCHandleToIntPtr (GCHandle handle)
		{
			return GCHandle.ToIntPtr (handle);
		}

		public static object ObjectFromIntPtr (IntPtr ptr)
		{
			return NativeDependencyObjectHelper.Lookup (ptr);
		}
		
		public static void FreeHGlobal (IntPtr ptr)
		{
			Marshal.FreeHGlobal (ptr);
		}

		public static string PtrToStringAuto (IntPtr ptr)
		{
			return Marshal.PtrToStringAuto (ptr);
		}

		public static void DeleteDirectory (string path)
		{
			Directory.Delete (path, true);
		}

		public static IntPtr StreamToIntPtr (Stream stream)
		{
			byte[] buffer = new byte[1024];
			IntPtr buf = Marshal.AllocHGlobal ((int) stream.Length);
			int ofs = 0;
			int nread = 0;
			
			if (stream.CanSeek && stream.Position != 0)
				stream.Seek (0, SeekOrigin.Begin);

			do {
				nread = stream.Read (buffer, 0, 1024);
				Marshal.Copy (buffer, 0, (IntPtr) (((long)buf)+ofs), nread);
				ofs += nread;
			} while (nread != 0);

			return buf;
		}

		public static object ValueFromConvertible (Type type, IConvertible value)
		{
			if (type == typeof (string))
				return Convert.ToString (value);
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
