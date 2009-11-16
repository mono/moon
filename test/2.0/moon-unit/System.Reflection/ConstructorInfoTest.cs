//
// ConstructorInfo Unit Tests
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Reflection {

	[TestClass]
	public class ConstructorInfoTest {

		[TestMethod]
		public void Invoke ()
		{
			ConstructorInfo ci = typeof (FileInfo).GetConstructor (new Type [] { typeof (string) });
			Assert.IsNotNull (ci, ".ctor(string)");

			Assert.Throws<MethodAccessException> (delegate {
				ci.Invoke (new object [] { "secret.data" });
			}, "Invoke(object[])");

			Assert.Throws<MethodAccessException> (delegate {
				ci.Invoke (BindingFlags.Public | BindingFlags.Instance, null, new object [] { "secret.data" }, CultureInfo.InvariantCulture);
			}, "Invoke(BindingFlags,Binder,object[],CultureInfo");
		}
	}
}

