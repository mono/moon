//
// Reflection.Emit Unit Tests
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
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using Ast = System.Linq.Expressions.Expression;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	// References:
	// * Security Issues in Reflection Emit
	//	http://msdn.microsoft.com/en-us/library/9syytdak(VS.95).aspx

	[TestClass]
	public class ReflectionEmitTest {

		private static DynamicMethod Create (Type type)
		{
			ConstructorInfo ci = type.GetConstructor (Type.EmptyTypes);
			DynamicMethod dm = new DynamicMethod ("CreateInstance", type, Type.EmptyTypes);
			ILGenerator il = dm.GetILGenerator ();
			il.Emit (OpCodes.Newobj, ci);
			il.Emit (OpCodes.Ret);
			return dm;
		}

		[TestMethod]
		public void DynamicMethod_NewObj_Critical ()
		{
			// AppDomainManager type definition is decorated with [SecurityCritical]
			DynamicMethod dm = Create (typeof (AppDomainManager));
			Assert.Throws<TargetInvocationException, MethodAccessException> (delegate {
				dm.Invoke (null, new object [0]);
			}, "Invoke");
		}

		[TestMethod]
		public void DynamicMethod_NewObj_Transparent ()
		{
			DynamicMethod dm = Create (typeof (X509Certificate));
			X509Certificate cert = (X509Certificate) dm.Invoke (null, new object [0]);
			Assert.IsNotNull (cert, "Invoke");
		}

		[TestMethod]
		public void DynamicMethod_NewObj_NonAccessible ()
		{
			// find a private type inside mscorlib
			DynamicMethod dm = Create (Type.GetType ("System.Security.Policy.Evidence"));
			Assert.Throws<TargetInvocationException, MethodAccessException> (delegate {
				dm.Invoke (null, new object [0]);
			}, "Invoke");
		}

		[TestMethod]
		public void DynamicMethod_SkipVisibilityCheck_ReadPlatformCodeInternalField ()
		{
			var p = Ast.Parameter (typeof (Nullable<int>), "i");
			var lambda = Ast.Lambda<Func<Nullable<int>, int>> (
				Ast.Field (
					p,
					typeof (Nullable<int>).GetField ("value", BindingFlags.NonPublic | BindingFlags.Instance)),
				p);
	
			Assert.Throws<FieldAccessException> (() => lambda.Compile ());
		}

		[TestMethod]
		public void DynamicMethod_SkipVisibilityCheck_ReadUserCodePrivateField ()
		{
			var parameter = Ast.Parameter (typeof (IgnoreAttribute), "attribute");

			var lambda = Ast.Lambda<Func<IgnoreAttribute, string>> (
				Ast.Field (
					parameter,
					typeof (IgnoreAttribute).GetField ("reason", BindingFlags.NonPublic | BindingFlags.Instance)),
				parameter);

			var reader = lambda.Compile ();

			var attribute = new IgnoreAttribute ("foo");

			Assert.AreEqual ("foo", reader (attribute));
		}

		[TestMethod]
		public void DynamicMethod_SkipVisibilityCheck_New_UseCodePrivateType ()
		{
			var moon_testing = typeof (Mono.Moonlight.UnitTesting.MoonLogProvider).Assembly;
			var log_request = moon_testing.GetType (
				"Mono.Moonlight.UnitTesting.MoonLogProvider+LogRequest");

			var factory = Ast.Lambda<Func<object>> (
				Ast.New (log_request)).Compile ();

			var obj = factory ();

			Assert.IsNotNull (obj);
			Assert.AreEqual (log_request, obj.GetType ());
		}

		// TODO
		// method/property/event calls
		// fields access
		// unsafe code
		// ...
	}
}
