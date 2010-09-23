//
// Unit tests for System.Net.IPAddress
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Net;
using System.Net.Sockets;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Sockets {

	[TestClass]
	public class IPAddressTest {

		[TestMethod]
		public void IsLoopback ()
		{
			// NRE is thrown by FX (even 4.0)
			Assert.Throws<ArgumentNullException> (delegate {
				IPAddress.IsLoopback (null);
			}, "null");
		}

		[TestMethod]
		public void IPv4_ScopeId ()
		{
			IPAddress addr = IPAddress.Loopback;
			Assert.Throws<SocketException> (delegate {
				Console.WriteLine (addr.ScopeId);
			}, "get_ScopeId");
			Assert.Throws<SocketException> (delegate {
				addr.ScopeId = 1;
			}, "set_ScopeId");
		}

		[TestMethod]
		public void IPv6_ScopeId ()
		{
			if (!Socket.OSSupportsIPv6)
				return;

			IPAddress addr = IPAddress.IPv6Loopback;
			Assert.AreEqual (0, addr.ScopeId, "ScopeId");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				addr.ScopeId = -1;
			}, "ScopeId / negative");

			addr.ScopeId = UInt32.MaxValue;
			Assert.AreEqual (UInt32.MaxValue, addr.ScopeId, "ScopeId / max");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				addr.ScopeId = (long) UInt32.MaxValue + 1;
			}, "ScopeId / too large");
		}
	}
}

