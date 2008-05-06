//
// Helper.cs: Exposes some methods that require access to mscorlib or
// System but are not exposed in the 2.1 profile.   This is necessary
// to avoid making moonlight.exe a friend of the System and mscorlib
// assemblies.
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
//   Chris Toshok (toshok@novell.com)
//   Jb Evain (jbevain@novell.com)
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
using System.IO;

namespace Mono {

	public class Helper {
		public static Assembly Agclr;

		private static TypeConverter GetConverterFor (PropertyInfo info)
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

			if (at == null || at == TypeConverterAttribute.Default)
				converter = TypeDescriptor.GetConverter (info.PropertyType);
			else {
				Type t = Type.GetType (at.ConverterTypeName);
				if (t == null) {
					converter = TypeDescriptor.GetConverter (info.PropertyType);
				}
				else {
					ConstructorInfo ci = t.GetConstructor (new Type[] { typeof(Type) });
					if (ci != null)
						converter = (TypeConverter) ci.Invoke (new object[] { info.PropertyType });
					else
						converter = (TypeConverter) Activator.CreateInstance (t);
				}
			}
			return converter;
		}
		
		public static bool SetPropertyFromString (object target, string name, string value, out string error, out IntPtr unmanaged_value)
		{
			unmanaged_value = IntPtr.Zero;

			PropertyInfo pi = target.GetType ().GetProperty (name);
			if (pi == null){
				error = "no property descriptor found";
				return false;
			}

			TypeConverter converter = GetConverterFor (pi);
			if (!converter.CanConvertFrom (typeof (string))){
				//
				// Attempt to create an unmanaged Value* object, if one is created, the managed parser
				// will create a managed wrapper for the object and call SetPropertyFromValue with the
				// managed object
				//

				unmanaged_value = NativeMethods.value_from_str_with_typename (pi.PropertyType.Name, name, value);
				if (unmanaged_value == IntPtr.Zero)
					error = "unable to convert to this type from a string";

				error = "Unmanaged object created, a managed wrapper must be created to set this property.";
				return false;
			}

			error = null;
			try {
				pi.SetValue (target, converter.ConvertFrom (value), null);
			} catch (Exception e) {
				error = e.ToString ();
			}

			return true;
		}

		public static bool SetPropertyFromValue (object target, string name, object value, out string error)
		{
			PropertyInfo pi = target.GetType ().GetProperty (name);
			if (pi == null){
				error = "no property descriptor found";
				return false;
			}

			error = null;
			try {
				pi.SetValue (target, value, null);
			} catch (Exception e) {
				error = e.ToString ();
				return false;
			}

			return true;
		}

		public static object ChangeType (object obj, Type type)
		{
			return Convert.ChangeType (obj, type);
		}

		public static string GetStackTrace ()
		{
			return Environment.StackTrace;
		}

		public static AppDomain CreateDomain (IntPtr key)
		{
			AppDomain a = AppDomain.CreateDomain ("moonlight-" + key);

			return a;
		}

		public static void UnloadDomain (AppDomain domain)
		{
			AppDomain.Unload (domain);
		}
		
		public static object CreateInstanceAndUnwrap (AppDomain target, string assemblyName, string typeName)
		{
			return target.CreateInstanceAndUnwrap (assemblyName, typeName);
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

		public static void FreeHGlobal (IntPtr ptr)
		{
			Marshal.FreeHGlobal (ptr);
		}

		public static string PtrToStringAuto (IntPtr ptr)
		{
			return Marshal.PtrToStringAuto (ptr);
		}

		public static String[] Split (String s, String[] separator)
		{
			return s.Split (separator, StringSplitOptions.None);
		}

		public static void ThreadMemoryBarrier ()
		{
			Thread.MemoryBarrier ();
		}
		
		internal static Assembly GetAgclr ()
		{
			return Agclr;
		}
		
		/// <summary>
		/// Looks up a dependency object given the native pointer.
		/// The calling code must free the native pointer when it's finished with it.
		/// </summary>
		public static object LookupDependencyObject (Kind k, IntPtr ptr)
		{
			Assembly agclr = GetAgclr ();
			Type depobj = agclr.GetType ("System.Windows.DependencyObject");
			MethodInfo lookup = depobj.GetMethod ("Lookup", BindingFlags.NonPublic | BindingFlags.Static, null, new Type [] {typeof (Kind), typeof (IntPtr)}, null);
			return lookup.Invoke (null, new object [] {k, ptr});
		}
		
		public static IntPtr GetNativeObject (object dependency_object)
		{
			if (dependency_object == null)
				return IntPtr.Zero;
			
			Assembly agclr = GetAgclr ();
			Type depobj = agclr.GetType ("System.Windows.DependencyObject");
			FieldInfo field = depobj.GetField ("_native", BindingFlags.Instance | BindingFlags.NonPublic);
			return (IntPtr) field.GetValue (dependency_object);
		}

		public static void DeleteDirectory (string path)
		{
			Directory.Delete (path, true);
		}
	}
}
