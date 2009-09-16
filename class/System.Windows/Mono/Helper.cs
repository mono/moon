//
// Helper.cs: Exposes some methods that require access to mscorlib or
// System but are not exposed in the 2.1 profile.
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007, 2009 Novell, Inc.
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
using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Threading;
using System.Windows;

using Mono.Security.Cryptography;

namespace Mono {

	internal static partial class Helper {
		internal static CultureInfo DefaultCulture = CultureInfo.GetCultureInfo ("en-US");
		
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

			if (t == null || !typeof (TypeConverter).IsAssignableFrom (t))
				return null;

			ConstructorInfo ci = t.GetConstructor (new Type[] { typeof(Type) });
			if (ci != null)
				converter = (TypeConverter) ci.Invoke (new object[] { target_type });
			else
				converter = (TypeConverter) Activator.CreateInstance (t);

			return converter;
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
		
		static void CanonicalizeName (StringBuilder sb, string str, int n)
		{
			// Fix path separators and capitalization
			for (int i = 0; i < n; i++) {
				if (str[i] != '\\')
					sb.Append (Char.ToLower (str[i], CultureInfo.InvariantCulture));
				else
					sb.Append ('/');
			}
		}
		
		public static string CanonicalizeAssemblyPath (string path)
		{
			StringBuilder sb = new StringBuilder (path.Length);
			int i;
			
			for (i = path.Length; i > 0; i--) {
				if (path[i - 1] == '/' || path[i - 1] == '\\')
					break;
			}
			
			CanonicalizeName (sb, path, i);
			
			sb.Append (path, i, path.Length - i);
			
			return sb.ToString ();
		}
		
		public static string CanonicalizeResourceName (string resource)
		{
			StringBuilder sb = new StringBuilder (resource.Length);
			
			CanonicalizeName (sb, resource, resource.Length);
			
			return sb.ToString ();
		}

#if NET_2_1
		// only for the plugin, not for the desktop

		const int MoonWalker = 0x6e6f6f6d;
		const int BlockSize = 4096;

		static bool IsSigned (Stream stream)
		{
			byte [] marker = new byte [4];
			stream.Seek (-132, SeekOrigin.End);
			if (stream.Read (marker, 0, 4) != 4)
				return false;
			return (BitConverter.ToInt32 (marker, 0) == MoonWalker);
		}

		static byte [] HashStream (Stream stream, int length)
		{
			stream.Position = 0;
			byte [] buffer = new byte [BlockSize];
			using (SHA1Managed digest = new SHA1Managed ()) {
				while (length > 0) {
					int len = stream.Read (buffer, 0, System.Math.Min (length, BlockSize));
					if (len == length)
						digest.TransformFinalBlock (buffer, 0, len);
					else
						digest.TransformBlock (buffer, 0, len, null, 0);
					length -= len;
				}
				return digest.Hash;
			}
		}

		public static bool CheckFileIntegrity (string filename)
		{
			if (filename == null)
				return false;

			byte [] hash = null;
#if true
			RSA rsa = CryptoConvert.FromCapiKeyBlob (codec_public_key_blob);

			byte [] signature = new byte [128];
			using (FileStream fs = File.OpenRead (filename)) {
				if (!IsSigned (fs))
					return false;

				hash = HashStream (fs, (int) fs.Length - 132);

				fs.Seek (-128, SeekOrigin.End);
				if (fs.Read (signature, 0, 128) != 128)
					return false;
			}

			RSAPKCS1SignatureDeformatter def = new RSAPKCS1SignatureDeformatter (rsa);
			def.SetHashAlgorithm ("SHA1");
			return def.VerifySignature (hash, signature);
#else
			// current codecs downloaded from MS are not signed - but we know their SHA1 digest
			using (FileStream fs = File.OpenRead (filename)) {
				hash = HashStream (fs, (int) fs.Length);
				switch (BitConverter.ToString (hash)) {
				// x86
				case "DD-AC-09-75-DD-94-59-55-B5-8A-B5-0B-18-61-9D-B7-D3-93-B1-17":
					return true;
				// x86-64
				case "DE-02-54-46-D1-D7-8F-49-98-BD-AA-AD-36-80-19-37-56-F3-C5-3B":
					return true;
				default:
					return false;
				}
			}
#endif
		}
#endif
	}
}
