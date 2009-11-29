//
// ConstructorBuilder Unit Tests
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
using System.Globalization;
using System.Reflection;
using System.Reflection.Emit;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Reflection.Emit {

	[TestClass]
	public class ConstructorBuilderTest {

		[TestMethod]
		public void Invoke ()
		{
			AssemblyName an = new AssemblyName ("moon-unit-emit-test");
			AssemblyBuilder ab = AppDomain.CurrentDomain.DefineDynamicAssembly (an, AssemblyBuilderAccess.Run);
			ModuleBuilder mb = ab.DefineDynamicModule ("emit-module");
			TypeBuilder tb = mb.DefineType ("typeConstructorBuilder");
			ConstructorBuilder cb = tb.DefineConstructor (MethodAttributes.Public, CallingConventions.Any, Type.EmptyTypes);
			Assert.IsNotNull (cb, ".ctor()");
			cb.GetILGenerator ().Emit (OpCodes.Ret);

			Assert.Throws<NotSupportedException> (delegate {
				cb.Invoke (new object [0]);
			}, "Invoke(object[])");

			Assert.Throws<NotSupportedException> (delegate {
				cb.Invoke (null, new object [0]);
			}, "Invoke(null,object[])");

			Assert.Throws<NotSupportedException> (delegate {
				cb.Invoke (BindingFlags.Public | BindingFlags.Instance, null, new object [0], CultureInfo.InvariantCulture);
			}, "Invoke(BindingFlags,Binder,object[],CultureInfo");

			Assert.Throws<NotSupportedException> (delegate {
				cb.Invoke (null, BindingFlags.Public | BindingFlags.Instance, null, new object [0], CultureInfo.InvariantCulture);
			}, "Invoke(BindingFlags,Binder,object[],CultureInfo");
		}
	}
}

