//
// Assembly Unit Tests
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
using System.Reflection;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Reflection {

	[TestClass]
	public class AssemblyTest {

		[TestMethod]
		[MoonlightBug ("mscorlib is the caller in moonlight ?")]
		public void GetCallingAssembly ()
		{
			Assembly a = Assembly.GetCallingAssembly ();
			Assert.IsTrue (a.FullName.StartsWith ("Microsoft.Silverlight.Testing, Version="), a.FullName);
		}

		[TestMethod]
		public void GetExecutingAssembly ()
		{
			Assembly a = Assembly.GetExecutingAssembly ();
			Assert.IsTrue (a.FullName.StartsWith ("moon-unit, Version="), a.FullName);
		}

		[TestMethod]
		public void Corlib ()
		{
			string [] corlib = { 
				// mscorlib full name
				"mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e",
				// wrong version
				"mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e",
				// without public key
				"mscorlib, Version=2.0.5.0, Culture=neutral",
				// without culture
				"mscorlib, Version=2.0.5.0, PublicKeyToken=7cec85d7bea7798e",
				// without version
				"mscorlib, Culture=neutral, PublicKeyToken=7cec85d7bea7798e",
				// minimal
				"mscorlib"
			};
			// full, partial or wrong - we always get corlib
			foreach (string name in corlib) {
				Assembly a = Assembly.Load (name);
				Assert.AreEqual ("mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e", a.FullName, name);
			}
		}

		[TestMethod]
		public void System_FullName ()
		{
			// test case for a platform assembly
			Assembly a = Assembly.Load ("System, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e");
			Assert.IsTrue (a.FullName.StartsWith ("System, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e"), "System-FullName");
		}

		[TestMethod]
		[MoonlightBug ("assembly loaded using partial name")]
		public void System_PartialName ()
		{
			// test case for a platform assembly
			Assert.Throws<FileNotFoundException> (delegate {
				// missing public key token
				Assembly.Load ("System, Version=2.0.5.0, Culture=neutral");
			}, "Partial");
		}

		[TestMethod]
		public void ExistsOnlyInGAC ()
		{
			string [] system_security = { 
				// does not exists (SL version number and public key token)
				"System.Security, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e",
				// regular framework (2.0)
				"System.Security, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a",
				// without public key token
				"System.Security, Version=2.0.0.0, Culture=neutral",
				// without culture
				"System.Security, Version=2.0.0.0, PublicKeyToken=b03f5f7f11d50a3a",
				// without version
				"System.Security, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a",
				// minimal
				"System.Security"
			};
			Assembly a = null;
			// full, partial or wrong - we always get corlib
			foreach (string name in system_security) {
				Assert.Throws<FileNotFoundException> (delegate {
					Assembly.Load (name);
				}, name);
			}
		}

		[TestMethod]
		public void MoonUnit_FullName ()
		{
			Assembly a = Assembly.Load (this.GetType ().Assembly.FullName);
			Assert.IsNotNull (a, "FullName");
		}

		[TestMethod]
		[MoonlightBug ("assembly loaded using partial name")]
		public void MoonUnit_PartialName ()
		{
			Assert.Throws<FileLoadException> (delegate {
				Assembly.Load ("moon-unit");
			}, "moon-unit");

			Assert.Throws<FileNotFoundException> (delegate {
				Assembly.Load ("moon-unit.dll");
			}, "moon-unit.dll");
		}

		[TestMethod]
		public void UnitTestAssembly ()
		{
			Assembly a = typeof (TestClassAttribute).Assembly;
			Assert.IsNotNull ("Assembly-Property");
			a = Assembly.Load (a.FullName);
			Assert.IsNotNull ("Assembly-FullName");
		}


		[TestMethod]
		public void Assembly_CreateInstance_SecurityCritical ()
		{
			Assembly corlib = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				corlib.CreateInstance ("System.AppDomainManager");
			}, "AppDomainManager");
		}

		[TestMethod]
		public void Assembly_CreateInstance_Transparent ()
		{
			Assembly corlib = typeof (int).Assembly;
			Assert.IsNotNull (corlib.CreateInstance ("System.Runtime.InteropServices.GCHandle"), "GCHandle");
		}
	}
}

