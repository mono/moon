//
// Unit tests for System.AppDomain
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Reflection.Emit;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Input {

	[TestClass]
	public class AppDomainTest {

		const string FriendlyName = "Silverlight AppDomain";
		static string ToString = "Name:" + FriendlyName + Environment.NewLine +
			"There are no context policies." + Environment.NewLine;

		[TestMethod]
		public void CurrentDomain ()
		{
			AppDomain ad = AppDomain.CurrentDomain;
			Assert.IsNotNull (ad, "CurrentDomain");

			Assert.AreEqual ("Silverlight AppDomain", ad.FriendlyName, "FriendlyName");
			Assert.AreEqual (typeof (AppDomain), ad.GetType (), "GetType");
			Assert.AreEqual (ToString, ad.ToString (), "ToString");
		}

		// Silverlight only defines AssemblyBuilderAccess.Run but the normal framework
		// supports Save (2), RunAndSave (3), ReflectionOnly (6 in FX 2.0) and RunAndCollect (9 in FX 4.0)
		const AssemblyBuilderAccess Save = (AssemblyBuilderAccess) 2;
		const AssemblyBuilderAccess RunAndSave = (AssemblyBuilderAccess) 3;
		const AssemblyBuilderAccess ReflectionOnly = (AssemblyBuilderAccess) 6;
		const AssemblyBuilderAccess RunAndCollect = (AssemblyBuilderAccess) 9;

		[TestMethod]
		public void DefineDynamicAssembly ()
		{
			AppDomain ad = AppDomain.CurrentDomain;
			Assert.Throws<ArgumentNullException> (delegate {
				ad.DefineDynamicAssembly (null, AssemblyBuilderAccess.Run);
			}, "null,AssemblyBuilderAccess");

			AssemblyName an = new AssemblyName ("AppDomainTest.DynamicAssembly");

			Assert.Throws<ArgumentException> (delegate {
				ad.DefineDynamicAssembly (an, Save);
			}, "AssemblyName,Save");
			Assert.Throws<ArgumentException> (delegate {
				ad.DefineDynamicAssembly (an, RunAndSave);
			}, "AssemblyName,RunAndSave");
			Assert.Throws<ArgumentException> (delegate {
				ad.DefineDynamicAssembly (an, ReflectionOnly);
			}, "AssemblyName,ReflectionOnly");
			Assert.Throws<ArgumentException> (delegate {
				ad.DefineDynamicAssembly (an, RunAndCollect);
			}, "AssemblyName,RunAndCollect");

			Assert.Throws<ArgumentException> (delegate {
				ad.DefineDynamicAssembly (an, (AssemblyBuilderAccess)Int32.MinValue);
			}, "AssemblyName,invalid");

			AssemblyBuilder ab = ad.DefineDynamicAssembly (an, AssemblyBuilderAccess.Run);
			Assert.IsNotNull (ab, "DefineDynamicAssembly");
			Assert.AreEqual ("AppDomainTest.DynamicAssembly, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", ab.FullName, "FullName");
			Assert.AreEqual ("v2.0.50727", ab.ImageRuntimeVersion, "ImageRuntimeVersion");
			Assert.AreEqual ("AppDomainTest.DynamicAssembly, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", ab.ToString (), "ToString");
		}
	}
}

