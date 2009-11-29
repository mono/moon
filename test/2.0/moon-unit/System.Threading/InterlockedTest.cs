//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Author:
//   Luca Barbieri (luca.barbieri@gmail.com)
//
// (C) 2004 Luca Barbieri
// Copyright (c) 2009 Novell, Inc.

using System;
using System.Threading;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Threading {

	[TestClass]
	public class InterlockedTest {

		int int32;
		long int64;

		const int int32_1 = 0x12490082;
		const int int32_2 = 0x24981071;
		const int int32_3 = 0x36078912;
		const long int64_1 = 0x1412803412472901L;
		const long int64_2 = 0x2470232089701124L;
		const long int64_3 = 0x3056432945919433L;

		[TestMethod]
		public void CompareExchange_Int32 ()
		{
			int32 = int32_1;
			Assert.AreEqual (int32_1, Interlocked.CompareExchange (ref int32, int32_2, int32_3), "a");
			Assert.AreEqual (int32_1, int32, "b");
		}

		[TestMethod]
		public void CompareExchange_Int64 ()
		{
			int64 = int64_1;
			Assert.AreEqual (int64_1, Interlocked.CompareExchange (ref int64, int64_2, int64_3), "a");
			Assert.AreEqual (int64_1, int64, "b");
		}

		[TestMethod]
		public void CompareExchange_Generic ()
		{
			object a = null;
			Assert.IsNull (Interlocked.CompareExchange<object> (ref a, a, a), "null,null,null");
			object b = new object ();
			Assert.IsNull (Interlocked.CompareExchange<object> (ref a, a, b), "null,non-null,non-null");
			Assert.IsNull (Interlocked.CompareExchange<object> (ref a, b, a), "null,non-null,null");
			Assert.AreSame (b, Interlocked.CompareExchange<object> (ref a, b, b), "null,null,non-null");

			Assert.AreSame (b, Interlocked.CompareExchange<object> (ref b, a, a), "non-null,null,null");
			Assert.AreSame (b, Interlocked.CompareExchange<object> (ref b, a, b), "non-null,null,non-null");
			Assert.AreSame (b, Interlocked.CompareExchange<object> (ref b, b, a), "non-null,non-null,null");
			Assert.AreSame (b, Interlocked.CompareExchange<object> (ref b, b, b), "non-null,non-null,non-null");
		}

		[TestMethod]
		public void Exchange_Generic ()
		{
			object a = null;
			Assert.IsNull (Interlocked.Exchange<object> (ref a, a), "null,null");
			object b = new object ();
			Assert.IsNull (Interlocked.Exchange<object> (ref a, b), "null,non-null");
			Assert.AreSame (b, Interlocked.Exchange<object> (ref b, a), "non-null,null");
			Assert.AreSame (b, Interlocked.Exchange<object> (ref b, b), "non-null,non-null");
		}
	}
}

