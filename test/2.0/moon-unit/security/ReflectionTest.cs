//
// Reflection Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.IO;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Windows;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	// References:
	// * Security Considerations for Reflection
	//	http://msdn.microsoft.com/en-us/library/stfy7tfc(VS.95).aspx

	class PrivateType {
		private double px;
		internal double py;

		public PrivateType (double x, double y)
		{
			px = x;
			py = y;
		}

		public double X { 
			get { return px; }
		}

		public double Y { 
			get { return py; }
		}

		private void Reset ()
		{
			px = 0.0d;
			py = 0.0d;
		}

		internal double Add ()
		{
			return px + py;
		}
	}

	[TestClass]
	public class ReflectionTest {

		[TestMethod]
		public void ReflectTransparentPublicType ()
		{
			Type [] t = new Type [2] { typeof (double), typeof (double) };
			object [] p = new object [2] { 1.0d, 2.0d };
			Point pt = (Point) typeof (Point).GetConstructor (t).Invoke (p);
			Assert.AreEqual (1.0, pt.X, "X");
			Assert.AreEqual (2.0, pt.Y, "Y");
		}

		[TestMethod]
		public void ReflectTransparentPrivateType ()
		{
			Type [] t = new Type [2] { typeof (double), typeof (double) };
			object [] p = new object [2] { 1.0d, 2.0d };
			ConstructorInfo ci = typeof (PrivateType).GetConstructor (t);
			// we can find it...
			Assert.IsNotNull (ci, "ctor");
			// and we can use it since it's defined in this assembly
			// i.e. just like we could do a "new PrivateStruct();"
			PrivateType pt = (PrivateType) ci.Invoke (p);
			Assert.AreEqual (1.0, pt.X, "X");
			Assert.AreEqual (2.0, pt.Y, "Y");
		}

		[TestMethod]
		public void ReflectTransparentPrivateType_Outside ()
		{
			// find a private type inside mscorlib
			Type t = Type.GetType ("System.Security.Policy.Evidence");
			Assert.IsNotNull (t, "Type");
			Assert.IsTrue (t.IsNotPublic, "IsNotPublic");
			ConstructorInfo ci = ci = t.GetConstructor (Type.EmptyTypes);
			Assert.IsNotNull (ci, "ctor");
			// but we can't use it since it comes from another assembly (mscorlib)
			// from a non-public type
			Assert.Throws<MethodAccessException> (delegate {
				ci.Invoke (new object [0]);
			}, "call");
		}

		[TestMethod]
		public void ReflectTransparentPrivateType_PrivateMember ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			MethodInfo mi = typeof (PrivateType).GetMethod ("Reset", BindingFlags.Instance | BindingFlags.NonPublic);
			// we can find it...
			Assert.IsNotNull (mi, "Reset");
			// but we can't use it, just like we can't write "pt.Reset();"
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (pt, new object [0]);
			}, "Reset");
		}

		[TestMethod]
		public void ReflectTransparentPrivateType_InternalMember ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			MethodInfo mi = typeof (PrivateType).GetMethod ("Add", BindingFlags.Instance | BindingFlags.NonPublic);
			// we can find it...
			Assert.IsNotNull (mi, "Add");
			// and we can use it since it's internal
			// i.e. just like we could do a "pt.Clear();"
			double result = (double) mi.Invoke (pt, new object [0]);
			Assert.AreEqual (3.0, result, "Return Value");
		}

		[TestMethod]
		public void ReflectTransparentPrivateType_PrivateField ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			FieldInfo fi = typeof (PrivateType).GetField ("px", BindingFlags.Instance | BindingFlags.NonPublic);
			// we can find it...
			Assert.IsNotNull (fi, "px");
			// but we can't use it, just like we can't write "double d = pt.px;"
			Assert.Throws<FieldAccessException> (delegate {
				fi.GetValue (pt);
			}, "GetValue");
			// or "pt.px = -1.0;"
			Assert.Throws<FieldAccessException> (delegate {
				fi.SetValue (pt, -1.0);
			}, "SetValue");
		}

		[TestMethod]
		public void ReflectTransparentPrivateStruct_InternalField ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			FieldInfo fi = typeof (PrivateType).GetField ("py", BindingFlags.Instance | BindingFlags.NonPublic);
			// we can find it...
			Assert.IsNotNull (fi, "py");
			// and we can use it, just like we can write "double d = pt.py;"
			Assert.AreEqual (2.0d, (double) fi.GetValue (pt), "GetValue");
			// or "pt.py = -2.0;"
			fi.SetValue (pt, -2.0);
			Assert.AreEqual (-2.0d, pt.Y, "Y");
		}

		[TestMethod]
		public void ReflectTransparentPublicTypePrivateMethod_Outside ()
		{
			Socket s = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			Type t = typeof (Socket);
			foreach (MethodInfo mi in t.GetMethods (BindingFlags.Instance | BindingFlags.NonPublic)) {
				Assert.Throws<MethodAccessException> (delegate {
					mi.Invoke (s, new object [mi.GetParameters ().Length]);
				}, mi.ToString ());
			}
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_Ctor ()
		{
			ConstructorInfo ci = typeof (FileInfo).GetConstructor (new Type [1] { typeof (string) });
			Assert.Throws<MethodAccessException> (delegate {
				ci.Invoke (new object [1] { "/info" });
			}, "FileInfo.ctor(string)");
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_PublicProperty ()
		{
			X509Certificate cert = new X509Certificate ();

			PropertyInfo pi = typeof (X509Certificate).GetProperty ("Handle");
			Assert.Throws<MethodAccessException> (delegate {
				pi.GetValue (cert, null);
			}, "X509Certificate.Handle-GetValue");

			MethodInfo mi = pi.GetGetMethod ();
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (cert, new object [0]);
			}, "X509Certificate.Handle-Invoke-Getter");
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_PublicStaticProperty ()
		{
			PropertyInfo pi = typeof (Environment).GetProperty ("CurrentDirectory");
			Assert.Throws<MethodAccessException> (delegate {
				pi.GetValue (null, null);
			}, "Environment.CurrentDirectory-GetValue");
			Assert.Throws<MethodAccessException> (delegate {
				pi.SetValue (null, null, null);
			}, "Environment.CurrentDirectory-SetValue");

			MethodInfo mi = pi.GetGetMethod ();
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (null, new object [0]);
			}, "Environment.CurrentDirectory-Invoke-Getter");
			mi = pi.GetSetMethod ();
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (null, new object [1]);
			}, "Environment.CurrentDirectory-Invoke-Setter");
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_PublicStaticField ()
		{
			FieldInfo fi = typeof (Marshal).GetField ("SystemDefaultCharSize");
			Assert.Throws<FieldAccessException> (delegate {
				fi.GetValue (null);
			}, "Marshal.SystemDefaultCharSize-GetValue");
			Assert.Throws<FieldAccessException> (delegate {
				fi.SetValue (null, null);
			}, "Marshal.SystemDefaultCharSize-SetValue");
			Assert.Throws<FieldAccessException> (delegate {
				fi.SetValue (null, null, BindingFlags.Default, null, null);
			}, "Marshal.SystemDefaultCharSize-SetValue-2");
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_GetFields ()
		{
			FieldInfo[] fis = typeof (Marshal).GetFields ();
			Assert.AreEqual (1, fis.Length, "Length");
			Assert.Throws<FieldAccessException> (delegate {
				fis [0].GetValue (null);
			}, "Marshal.SystemDefaultCharSize");
		}

		[TestMethod]
		public void ReflectCriticalPublicClass_PublicEvent ()
		{
			AppDomain ad = AppDomain.CurrentDomain;
			EventInfo ei = typeof (AppDomain).GetEvent ("UnhandledException");
			Assert.Throws<MethodAccessException> (delegate {
				ei.AddEventHandler (ad, null);
			}, "AppDomain.UnhandledException-AddEventHandler");
			Assert.Throws<MethodAccessException> (delegate {
				ei.RemoveEventHandler (ad, null);
			}, "AppDomain.UnhandledException-RemoveEventHandler");

			MethodInfo mi = ei.GetAddMethod ();
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (ad, new object [1]);
			}, "AppDomain.UnhandledException-GetAddMethod-Invoke");
			mi = ei.GetRaiseMethod ();
			// C# and VB do not generate this method
			Assert.IsNull (mi, "GetRaiseMethod");
			mi = ei.GetRemoveMethod ();
			Assert.Throws<MethodAccessException> (delegate {
				mi.Invoke (ad, new object [1]);
			}, "AppDomain.UnhandledException-GetRemoveMethod-Invoke");
		}
	}
}
