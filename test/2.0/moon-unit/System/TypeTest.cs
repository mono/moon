//
// Type Unit Tests
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
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Net;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System {

	[TestClass]
	public class TypeTest {

		[TestMethod]
		public void Type_InvokeMember_InstanceMethod_SecurityCritical ()
		{
			Type t = typeof (GCHandle);
			GCHandle h = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				// SecurityCritical method
				t.InvokeMember ("AddrOfPinnedObject", BindingFlags.Instance | BindingFlags.Public | BindingFlags.InvokeMethod, null, h, null);
			}, "InvokeMember");
		}

		[TestMethod]
		public void Type_InvokeMember_StaticMethod_SecurityCritical ()
		{
			Type t = typeof (File);
			GCHandle h = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				// SecurityCritical method
				t.InvokeMember ("OpenWrite", BindingFlags.Static | BindingFlags.Public | BindingFlags.InvokeMethod, null, h, new object [1] { "file.txt" }, null, CultureInfo.InvariantCulture, null);
			}, "InvokeMember");
		}

		[TestMethod]
		public void Type_InvokeMember_Getter_SecurityCritical ()
		{
			Type t = typeof (GCHandle);
			GCHandle h = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				// SecurityCritical getter
				t.InvokeMember ("Target", BindingFlags.Instance | BindingFlags.Public | BindingFlags.GetProperty, null, h, null);
			}, "InvokeMember");
		}

		[TestMethod]
		public void Type_InvokeMember_Setter_SecurityCritical ()
		{
			Type t = typeof (GCHandle);
			GCHandle h = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				// SecurityCritical setter
				t.InvokeMember ("Target", BindingFlags.Instance | BindingFlags.Public | BindingFlags.SetProperty, null, h, new object [1]);
			}, "InvokeMember");
		}

		[TestMethod]
		public void Type_InvokeMember_Transparent ()
		{
			Type t = typeof (GCHandle);
			GCHandle h = new GCHandle ();
			Assert.AreEqual (0, t.InvokeMember ("GetHashCode", BindingFlags.Instance | BindingFlags.Public | BindingFlags.InvokeMethod, null, h, null), "GetHashCode");
		}

		[TestMethod]
		public void Type_InvokeMember_Getter_Transparent ()
		{
			Type t = typeof (GCHandle);
			GCHandle h = new GCHandle ();
			Assert.IsFalse ((bool) t.InvokeMember ("IsAllocated", BindingFlags.Instance | BindingFlags.Public | BindingFlags.GetProperty, null, h, null), "IsAllocated");
		}

		[TestMethod]
		public void Type_IRefect_InvokeMember_InstanceMethod_SecurityCritical ()
		{
			IReflect r = typeof (GCHandle) as IReflect;
			GCHandle h = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				// SecurityCritical method
				r.InvokeMember ("AddrOfPinnedObject", BindingFlags.Instance | BindingFlags.Public | BindingFlags.InvokeMethod, null, h, null, null, CultureInfo.InvariantCulture, null);
			}, "InvokeMember");
		}
	}
}

