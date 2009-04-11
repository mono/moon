//
// Unit tests for System.Net.DnsEndPoint
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
using System.Net;
using System.Net.Sockets;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class DnsEndPointTest {

		[TestMethod]
		public void Constructor_StringInt ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DnsEndPoint (null, 0);
			}, "ctor(null,0)");
			Assert.Throws<ArgumentException> (delegate {
				new DnsEndPoint (String.Empty, 0);
			}, "ctor(Empty,0)");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new DnsEndPoint ("localhost", -1);
			}, "ctor(localhost,-1)");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new DnsEndPoint ("localhost", (0xffff + 1));
			}, "ctor(localhost,(0xffff + 1))");

			DnsEndPoint dep = new DnsEndPoint ("localhost", 65535);
			Assert.AreEqual (AddressFamily.Unspecified, dep.AddressFamily, "AddressFamily");
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (65535, dep.Port, "Port");
			Assert.IsFalse (dep.Equals (null), "Equals(null)");
			Assert.IsTrue (dep.Equals (dep), "Equals(self)");
			Assert.AreEqual ("Unspecified/localhost:65535", dep.ToString ());
		}

		[TestMethod]
		public void Constructor_StringIntAddressFamily ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DnsEndPoint (null, 0, AddressFamily.InterNetwork);
			}, "ctor(null,0,InterNetwork)");
			Assert.Throws<ArgumentException> (delegate {
				new DnsEndPoint (String.Empty, 0, AddressFamily.InterNetworkV6);
			}, "ctor(Empty,0,InterNetworkV6)");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new DnsEndPoint ("localhost", -1, AddressFamily.InterNetwork);
			}, "ctor(localhost,-1,InterNetwork)");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new DnsEndPoint ("localhost", (0xffff + 1), AddressFamily.InterNetworkV6);
			}, "ctor(localhost,(0xffff + 1),InterNetworkV6)");

			Assert.Throws<ArgumentException> (delegate {
				new DnsEndPoint ("localhost", 0, (AddressFamily) Int32.MinValue);
			}, "ctor(localhost,0,MinValue)");
		}

		[TestMethod]
		public void InterNetwork ()
		{
			DnsEndPoint dep = new DnsEndPoint ("localhost", 0, AddressFamily.InterNetwork);
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (AddressFamily.InterNetwork, dep.AddressFamily, "AddressFamily");
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (0, dep.Port, "Port");
			Assert.IsFalse (dep.Equals (null), "Equals(null)");
			Assert.IsTrue (dep.Equals (dep), "Equals(self)");
			Assert.AreEqual ("InterNetwork/localhost:0", dep.ToString ());
		}

		[TestMethod]
		public void InterNetworkV6 ()
		{
			DnsEndPoint dep = new DnsEndPoint ("localhost", 0, AddressFamily.InterNetworkV6);
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (AddressFamily.InterNetworkV6, dep.AddressFamily, "AddressFamily");
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (0, dep.Port, "Port");
			Assert.IsFalse (dep.Equals (null), "Equals(null)");
			Assert.IsTrue (dep.Equals (dep), "Equals(self)");
			Assert.AreEqual ("InterNetworkV6/localhost:0", dep.ToString ());
		}

		[TestMethod]
		public void Unspecified ()
		{
			DnsEndPoint dep = new DnsEndPoint ("localhost", 0, AddressFamily.Unspecified);
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (AddressFamily.Unspecified, dep.AddressFamily, "AddressFamily");
			Assert.AreEqual ("localhost", dep.Host, "Host");
			Assert.AreEqual (0, dep.Port, "Port");
			Assert.IsFalse (dep.Equals (null), "Equals(null)");
			Assert.IsTrue (dep.Equals (dep), "Equals(self)");
			Assert.AreEqual ("Unspecified/localhost:0", dep.ToString ());
		}

		[TestMethod]
		public void Unknown ()
		{
			Assert.Throws<ArgumentException> (delegate {
				new DnsEndPoint ("localhost", 0, AddressFamily.Unknown);
			}, "Unknown)");
		}
	}
}

