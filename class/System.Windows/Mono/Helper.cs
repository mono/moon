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
	}
}
