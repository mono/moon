//
// Delegate Unit Tests
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
using System.Reflection;
using System.Security.Cryptography.X509Certificates;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class DelegateTest {

		delegate string StringGetter ();

		[TestMethod]
		public void Delegate_CreateDelegate_StaticCriticalProperty ()
		{
			PropertyInfo pi = typeof (Environment).GetProperty ("CurrentDirectory");
			MethodInfo miget = pi.GetGetMethod ();
			StringGetter dget = null;
			Assert.IsNotNull (miget, "CurrentDirectory");

			Assert.Throws<ArgumentException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget);
			}, "Delegate.CreateDelegate(Type,MethodInfo)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,MethodInfo,bool)");

			Assert.Throws<ArgumentException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, miget);
			}, "Delegate.CreateDelegate(Type,object,MethodInfo)");

			Assert.Throws<ArgumentNullException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_CurrentDirectory");
			}, "Delegate.CreateDelegate(Type,object,string)");

			Assert.Throws<ArgumentException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_CurrentDirectory");
			}, "Delegate.CreateDelegate(Type,Type,string)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, miget, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo,bool)");

			Assert.Throws<ArgumentNullException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_currentdirectory", true);
			}, "Delegate.CreateDelegate(Type,object,string,bool)");

			Assert.Throws<ArgumentException> (delegate {
				// delegate could not be created
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_currentdirectory", true);
			}, "Delegate.CreateDelegate(Type,Type,string,bool)");

			Assert.Throws<ArgumentNullException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_currentdirectory", true, false);
			}, "Delegate.CreateDelegate(Type,object,MethodInfo,bool,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_currentdirectory", true, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,Type,MethodInfo,bool,bool)");
		}

		[TestMethod]
		public void Delegate_CreateDelegate_StaticTransparentProperty ()
		{
			PropertyInfo pi = typeof (Environment).GetProperty ("NewLine");
			MethodInfo miget = pi.GetGetMethod ();

			StringGetter dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget);
			Assert.IsNotNull (dget (), "Delegate.CreateDelegate(Type,MethodInfo)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget, false);
			Assert.IsNotNull (dget (), "Delegate.CreateDelegate(Type,MethodInfo,boolean)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, miget);
			Assert.IsNotNull (dget (), "Delegate.CreateDelegate(Type,object,MethodInfo)");

			Assert.Throws<ArgumentNullException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_NewLine");
			}, "Delegate.CreateDelegate(Type,Type,string)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_NewLine");
			Assert.IsNotNull (dget (), "Delegate.CreateDelegate(Type,Type,string)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, miget, false);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo,bool)");

			Assert.Throws<ArgumentNullException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_newline", true);
			}, "Delegate.CreateDelegate(Type,object,string,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_newline", true);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,Type,string,bool)");

			Assert.Throws<ArgumentNullException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), null, "get_newline", true, false);
			}, "Delegate.CreateDelegate(Type,object,MethodInfo,bool,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (Environment), "get_newline", true, false);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,Type,MethodInfo,bool,bool)");
		}

		delegate IntPtr IntPtrGetter ();

		[TestMethod]
		public void Delegate_CreateDelegate_InstanceCriticalProperty ()
		{
			X509Certificate cert = new X509Certificate ();
			PropertyInfo pi = typeof (X509Certificate).GetProperty ("Handle");
			MethodInfo miget = pi.GetGetMethod ();
			IntPtrGetter dget = null;

			Assert.Throws<ArgumentException> (delegate {
				// Delegate.CreateDelegate(Type,MethodInfo) is for static members only
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), miget);
			}, "Delegate.CreateDelegate(Type,MethodInfo)");

			dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), miget, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,MethodInfo,bool)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), null, miget);
			}, "Delegate.CreateDelegate(Type,object,MethodInfo)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), cert, "get_Handle");
			}, "Delegate.CreateDelegate(Type,object,string)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), typeof (X509Certificate), "get_Handle");
			}, "Delegate.CreateDelegate(Type,Type,string)");

			dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), null, miget, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo,bool)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), cert, "get_handle", true);
			}, "Delegate.CreateDelegate(Type,object,string,bool)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), typeof (X509Certificate), "get_handle", true);
			}, "Delegate.CreateDelegate(Type,Type,string,bool)");

			dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), cert, "get_handle", true, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,object,string,bool,bool)");

			dget = (IntPtrGetter) Delegate.CreateDelegate (typeof (IntPtrGetter), typeof (X509Certificate), "get_handle", true, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,Type,string,bool,bool)");
		}

		[TestMethod]
		public void Delegate_CreateDelegate_InstanceTransparentProperty ()
		{
			X509Certificate cert = new X509Certificate ();
			PropertyInfo pi = typeof (X509Certificate).GetProperty ("Issuer");
			MethodInfo miget = pi.GetGetMethod ();
			StringGetter dget = null;

			Assert.Throws<ArgumentException> (delegate {
				// Delegate.CreateDelegate(Type,MethodInfo) is for static members only
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget);
			}, "Delegate.CreateDelegate(Type,MethodInfo)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), miget, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,MethodInfo,boolean)"); // not static

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), cert, miget);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), cert, "get_Issuer");
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,string)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (X509Certificate), "get_Issuer");
			}, "Delegate.CreateDelegate(Type,Type,string)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), cert, miget, false);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), cert, "get_issuer", true);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,string,bool)");

			Assert.Throws<ArgumentException> (delegate {
				dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (X509Certificate), "get_issuer", true);
			}, "Delegate.CreateDelegate(Type,Type,string,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), cert, "get_issuer", true, false);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,MethodInfo,bool,bool)");

			dget = (StringGetter) Delegate.CreateDelegate (typeof (StringGetter), typeof (X509Certificate), "get_issuer", true, false);
			Assert.IsNull (dget, "Delegate.CreateDelegate(Type,Type,MethodInfo,bool,bool)");
		}

		delegate void Caller ();

		[TestMethod]
		public void Delegate_CreateDelegate_PrivateMethod ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			MethodInfo mi = typeof (PrivateType).GetMethod ("Reset", BindingFlags.NonPublic | BindingFlags.Instance);
			Assert.IsNotNull (mi, "Reset");

			Assert.Throws<MethodAccessException> (delegate {
				Caller dcall = (Caller) Delegate.CreateDelegate (typeof (Caller), pt, mi);
			}, "Delegate.CreateDelegate(Type,object,MethodInfo)");

			Assert.Throws<MethodAccessException> (delegate {
				Caller dcall = (Caller) Delegate.CreateDelegate (typeof (Caller), pt, "Reset");
			}, "Delegate.CreateDelegate(Type,object,string)");
		}

		delegate double DoubleGetter ();

		[TestMethod]
		public void Delegate_CreateDelegate_InternalMethod ()
		{
			PrivateType pt = new PrivateType (1.0, 2.0);
			MethodInfo mi = typeof (PrivateType).GetMethod ("Add", BindingFlags.NonPublic | BindingFlags.Instance);
			Assert.IsNotNull (mi, "Add");

			DoubleGetter dget = (DoubleGetter) Delegate.CreateDelegate (typeof (DoubleGetter), pt, mi);
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,string)");

			Assert.AreEqual (3.0, dget (), "dget-1");

			dget = (DoubleGetter) Delegate.CreateDelegate (typeof (DoubleGetter), pt, "Add");
			Assert.IsNotNull (dget, "Delegate.CreateDelegate(Type,object,string)");

			Assert.AreEqual (3.0, dget (), "dget-2");
		}
	}
}
