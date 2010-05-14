//
// Generics-related Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class GenericsTest {

		class Custom<T> {
		}

		[TestMethod]
		[MoonlightBug ("we do not throw - but API is unusable")]
		public void Type_SecurityCritical ()
		{
			// framework types
			Assert.Throws<MethodAccessException> (delegate {
				new List<AppDomainManager> ();
			}, "List<critical>");
			Assert.Throws<MethodAccessException> (delegate {
				new HashSet<AppDomainManager> ();
			}, "HashSet<critical>");
			// user type
			Assert.Throws<MethodAccessException> (delegate {
				new Custom<AppDomainManager> ();
			}, "Custom<critical>");
			Assert.Throws<MethodAccessException> (delegate {
				new Custom<Custom<AppDomainManager>> ();
			}, "Custom<Custom<critical>>");
		}

		[TestMethod]
		public void Type_Transparent ()
		{
			Assert.IsNotNull (new List<string> (), "List");
			Assert.IsNotNull (new HashSet<TimeZoneInfo> (), "HashSet");
			Assert.IsNotNull (new Custom<GCHandle> (), "Custom");
			Assert.IsNotNull (new Custom<Custom<DateTime>> (), "Custom-Inner");
		}

		int Method<T> (T t)
		{
			return 0;
		}

		int Method<T> (int x)
		{
			return x;
		}

		T Method<T> (DateTime dt)
		{
			return default (T);
		}

		[TestMethod]
		[MoonlightBug ("we do not throw - but API is unusable")]
		public void Method_SecurityCritical ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Method<AppDomainManager> (null);
			}, "generic parameter");
			Assert.Throws<MethodAccessException> (delegate {
				Method<List<AppDomainManager>> (null);
			}, "generic parameter - inner");
			Assert.Throws<MethodAccessException> (delegate {
				Method<AppDomainManager> (0);
			}, "unused generic parameter");
			Assert.Throws<MethodAccessException> (delegate {
				Method<AppDomainManager> (DateTime.Now);
			}, "generic return value");
		}

		[TestMethod]
		public void Method_Transparent ()
		{
			Assert.AreEqual (0, Method<GCHandle> (new GCHandle ()), "generic parameter");
			Assert.AreEqual (1, Method<TimeZoneInfo> (1), "unused generic parameter");
			Assert.IsNull (Method<string> (DateTime.Now), "generic return value");
		}
	}
}
