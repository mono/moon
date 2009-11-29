//
// Activator Unit Tests
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
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class ActivatorTest {

		class Private {
			public Private ()
			{
			}
		}

		class Thrower {
			public Thrower ()
			{
				throw new NotFiniteNumberException ();
			}
		}

		[TestMethod]
		public void CreateInstance_Type ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				Activator.CreateInstance (null);
			}, "null");

			Assert.AreEqual (0, Activator.CreateInstance (typeof (int)), "int");
			Assert.IsNull (Activator.CreateInstance (typeof (int?)), "int?");
			Assert.IsNotNull (Activator.CreateInstance (typeof (object)), "object");
			Assert.IsNotNull (Activator.CreateInstance (typeof (Private)), "Private");

			Assert.IsNotNull (Activator.CreateInstance (typeof (List<string>)), "generics");
			Assert.Throws<ArgumentException> (delegate {
				Activator.CreateInstance (typeof (List<>));
			}, "generic-open");

			Assert.Throws<TargetInvocationException> (delegate {
				Activator.CreateInstance (typeof (Thrower));
			}, "exception in .ctor");

			Assert.Throws<NotSupportedException> (delegate {
				Activator.CreateInstance (typeof (RuntimeArgumentHandle));
			}, "RuntimeArgumentHandle");
			Assert.Throws<NotSupportedException> (delegate {
				Activator.CreateInstance (typeof (TypedReference));
			}, "TypedReference");
			Assert.Throws<NotSupportedException> (delegate {
				Activator.CreateInstance (typeof (void));
			}, "void");

			Assert.Throws<MissingMethodException> (delegate {
				Activator.CreateInstance (typeof (Type));
			}, "Type");
		}

		[TestMethod]
		public void CreateInstance_Type_Objects ()
		{
			object[] empty = new object [0];

			Assert.Throws<ArgumentNullException> (delegate {
				Activator.CreateInstance (null, empty);
			}, "null, empty");

			Assert.Throws<MissingMethodException> (delegate {
				Activator.CreateInstance (typeof (int), 5);
			}, "int, 5");

			Assert.IsNull (Activator.CreateInstance (typeof (int?), null), "int?");

			Assert.IsNotNull (Activator.CreateInstance (typeof (object), null), "object");
		}

		[TestMethod]
		public void CreateInstance_SecurityCritical ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Activator.CreateInstance (typeof (AppDomainManager));
			}, "AppDomainManager-1");
			Assert.Throws<MethodAccessException> (delegate {
				Activator.CreateInstance<AppDomainManager> ();
			}, "AppDomainManager-2");
		}

		[TestMethod]
		public void CreateInstance_Transparent ()
		{
			Assert.IsNotNull (Activator.CreateInstance (typeof (GCHandle)), "GCHandle-1");
			Assert.IsNotNull (Activator.CreateInstance<GCHandle> (), "GCHandle-2");
		}

		[TestMethod]
		public void CreateInstance_OtherAssembly ()
		{
			var moon_testing = typeof (Mono.Moonlight.UnitTesting.MoonLogProvider).Assembly;
			var log_request = moon_testing.GetType (
				"Mono.Moonlight.UnitTesting.MoonLogProvider+LogRequest");

			Assert.Throws<MethodAccessException> (() => Activator.CreateInstance (log_request));
		}
	}
}

