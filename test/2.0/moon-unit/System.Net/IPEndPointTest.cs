//
// Unit tests for System.Net.IPEndPoint
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
	public class IPEndPointTest {

		[TestMethod]
		public void Constants ()
		{
			Assert.AreEqual (0, IPEndPoint.MinPort, "MinPort");
			Assert.AreEqual (65535, IPEndPoint.MaxPort, "MaxPort");
		}

		[TestMethod]
		public void Ctor_LongInt ()
		{
			IPEndPoint ep = new IPEndPoint (0, 80);
			Assert.AreEqual (new IPAddress (0), ep.Address, "Address");
			Assert.AreEqual (AddressFamily.InterNetwork, ep.AddressFamily, "AddressFamily");
			Assert.AreEqual (80, ep.Port, "Port");

			Assert.Throws<ArgumentNullException> (delegate {
				ep.Create (null);
			}, "Create(null)");

			// note: documented as ArgumentException
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.InterNetwork, 1);
				Assert.IsTrue (sa.Size < 8, "Size");
				ep.Create (sa);
			}, "Create(bad-size)");

			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.InterNetworkV6);
				Assert.IsTrue (sa.Size >= 8, "SizeV6");
				ep.Create (sa);
			}, "Create(InterNetworkV6)");
			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.Unknown);
				ep.Create (sa);
			}, "Create(Unknown)");
			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.Unspecified);
				ep.Create (sa);
			}, "Create(Unspecified)");
			EndPoint ep2 = ep.Create (new SocketAddress (AddressFamily.InterNetwork));

			Assert.IsFalse (ep.Equals (null), "Equals(null)");
			Assert.IsTrue (ep.Equals (ep), "Equals(self)");
			Assert.IsFalse (ep.Equals (ep2), "Equals(Create)");

			Assert.AreEqual ("InterNetwork:16:{0,80,0,0,0,0,0,0,0,0,0,0,0,0}", ep.Serialize ().ToString (), "Serialize");
			Assert.AreEqual ("0.0.0.0:80", ep.ToString (), "ToString");
		}

		[TestMethod]
		public void Ctor_IPAddressInt ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new IPEndPoint (null, 80);
			}, "IPEndPoint(null,int)");

			IPAddress a = new IPAddress (new byte [16]);
			Assert.AreEqual (AddressFamily.InterNetworkV6, a.AddressFamily, "IPAddress.AddressFamily");
			IPEndPoint ep = new IPEndPoint (a, 0);
			Assert.IsTrue (Object.ReferenceEquals (a, ep.Address), "Address");
			Assert.AreEqual (AddressFamily.InterNetworkV6, ep.AddressFamily, "AddressFamily");
			Assert.AreEqual (0, ep.Port, "Port");

			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.InterNetwork);
				ep.Create (sa);
			}, "Create(InterNetwork)");
			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.Unknown);
				ep.Create (sa);
			}, "Create(Unknown)");
			Assert.Throws<ArgumentException> (delegate {
				SocketAddress sa = new SocketAddress (AddressFamily.Unspecified);
				ep.Create (sa);
			}, "Create(Unspecified)");
			EndPoint ep2 = ep.Create (new SocketAddress (AddressFamily.InterNetworkV6));

			Assert.IsFalse (ep.Equals (null), "Equals(null)");
			Assert.IsTrue (ep.Equals (ep), "Equals(self)");
			Assert.IsTrue (ep.Equals (ep2), "Equals(Create)");

			Assert.AreEqual ("InterNetworkV6:28:{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}", ep.Serialize ().ToString (), "Serialize");
//			Assert.AreEqual ("0000:0000:0000:0000:0000:0000:0.0.0.0:0", ep.ToString (), "ToString");
		}

		[TestMethod]
		public void Address ()
		{
			IPEndPoint ep = new IPEndPoint (0, 0);
			Assert.AreEqual (AddressFamily.InterNetwork, ep.AddressFamily, "AddressFamily");
			ep.Address = null;
			Assert.IsNull (ep.Address, "null");

			Assert.Throws<NullReferenceException> (delegate {
				Assert.AreEqual (AddressFamily.InterNetwork, ep.AddressFamily);
			}, "null-AddressFamily");
			Assert.Throws<NullReferenceException> (delegate {
				ep.ToString ();
			}, "null-ToString");
			Assert.Throws<NullReferenceException> (delegate {
				ep.Create (new SocketAddress (AddressFamily.InterNetworkV6));
			}, "null-Create");

			// change family
			IPAddress a = new IPAddress (new byte [16]);
			ep.Address = a;
			Assert.AreEqual (AddressFamily.InterNetworkV6, ep.AddressFamily, "v6-AddressFamily");
		}

		[TestMethod]
		public void Port ()
		{
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new IPEndPoint (0, IPEndPoint.MinPort - 1);
			}, "IPEndPoint(int,-1)");

			IPEndPoint ep = new IPEndPoint (0, IPEndPoint.MinPort);
			Assert.AreEqual (IPEndPoint.MinPort, ep.Port, "Port-Min");

			IPAddress a = new IPAddress (new byte [16]);
			ep = new IPEndPoint (a, IPEndPoint.MaxPort);
			Assert.AreEqual (IPEndPoint.MaxPort, ep.Port, "Port-Max");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new IPEndPoint (a, IPEndPoint.MaxPort + 1);
			}, "IPEndPoint(IPAddress,65536)");
		}
	}
}

