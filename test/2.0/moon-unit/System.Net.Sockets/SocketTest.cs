//
// Unit tests for System.Net.Sockets.Socket
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
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Sockets {

	[TestClass]
	public class SocketTest : SilverlightTest {

		private bool complete;

		[TestMethod]
		public void Defaults ()
		{
			Socket s = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			Assert.AreEqual (AddressFamily.InterNetwork, s.AddressFamily, "AddressFamily");
			Assert.IsFalse (s.Connected, "Connected");
			Assert.IsFalse (s.NoDelay, "NoDelay");
			Assert.AreEqual (ProtocolType.Tcp, s.ProtocolType, "ProtocolType");
			Assert.IsNull (s.RemoteEndPoint, "RemoteEndPoint");
#if false
			Assert.AreEqual (8192, s.ReceiveBufferSize, "ReceiveBufferSize");
			Assert.AreEqual (8192, s.SendBufferSize, "SendBufferSize");
			Assert.AreEqual (32, s.Ttl, "Ttl");
#endif
			if (!Socket.OSSupportsIPv6) {
				Assert.Throws<SocketException> (delegate {
					new Socket (AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
				}, "InterNetworkV6");
			}

			Assert.Throws<SocketException> (delegate {
				new Socket (AddressFamily.Unknown, SocketType.Stream, ProtocolType.Tcp);
			}, "AddressFamily.Unknown");
			Assert.Throws<ArgumentException> (delegate {
				new Socket (AddressFamily.Unspecified, SocketType.Stream, ProtocolType.Tcp);
			}, "AddressFamily.Unspecified");

			Assert.Throws<SocketException> (delegate {
				new Socket (AddressFamily.InterNetwork, SocketType.Unknown, ProtocolType.Tcp);
			}, "SocketType.Unknown");

			Assert.Throws<SocketException> (delegate {
				new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Unknown);
			}, "ProtocolType.Unknown");

			s = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Unspecified);
			Assert.AreEqual (ProtocolType.Unspecified, s.ProtocolType, "ProtocolType-Unspecified");
		}

		[TestMethod]
		public void ConnectAsync_Static_Validations ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				Socket.ConnectAsync (SocketType.Stream, ProtocolType.Tcp, null);
			}, "SocketType,ProtocolType,null)");

			SocketAsyncEventArgs saea = new SocketAsyncEventArgs ();
			Assert.Throws<ArgumentNullException> (delegate {
				// weird since we can't create a socket with SocketType.Unknown
				Socket.ConnectAsync (SocketType.Unknown, ProtocolType.Tcp, saea);
			}, "Unknown,ProtocolType,SocketAsyncEventArgs-no-RemoteEndPoint");
			Assert.Throws<ArgumentNullException> (delegate {
				// weird since we can't create a socket with ProtocolType.Unknown
				Socket.ConnectAsync (SocketType.Stream, ProtocolType.Unknown, saea);
			}, "Stream,Unknown,SocketAsyncEventArgs-no-RemoteEndPoint");

			saea.RemoteEndPoint = new DnsEndPoint ("localhost", 0);
			saea.BufferList = new List<ArraySegment<byte>> ();
			Assert.Throws<ArgumentException> (delegate {
				Socket.ConnectAsync (SocketType.Unknown, ProtocolType.Tcp, saea);
			}, "Unknown,ProtocolType,SocketAsyncEventArgs-RemoteEndPoint+BufferList");
			Assert.Throws<ArgumentException> (delegate {
				Socket.ConnectAsync (SocketType.Stream, ProtocolType.Unknown, saea);
			}, "Stream,Unknown,SocketAsyncEventArgs-RemoteEndPoint+BufferList");
		}

		[TestMethod]
		[Asynchronous]
		public void ConnectAsync_Static ()
		{
			complete = false;
			SocketAsyncEventArgs saea = new SocketAsyncEventArgs ();
			saea.RemoteEndPoint = new DnsEndPoint ("localhost", 0);
			saea.Completed += new EventHandler<SocketAsyncEventArgs> (CompletedAccessDenied);
			Enqueue (() => {
				Assert.IsTrue (Socket.ConnectAsync (SocketType.Stream, ProtocolType.Tcp, saea), "SocketType,ProtocolType,SocketAsyncEventArgs");
			});
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ConnectAsync ()
		{
			Socket s = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			Assert.Throws<NullReferenceException> (delegate {
				s.ConnectAsync (null);
			}, "null)");

			SocketAsyncEventArgs saea = new SocketAsyncEventArgs ();
			Assert.Throws<ArgumentNullException> (delegate {
				s.ConnectAsync (saea);
			}, "SocketAsyncEventArgs-no-RemoteEndPoint)");

			saea.RemoteEndPoint = new DnsEndPoint ("localhost", 0);
			saea.BufferList = new List<ArraySegment<byte>> ();
			Assert.Throws<ArgumentException> (delegate {
				s.ConnectAsync (saea);
			}, "SocketAsyncEventArgs-RemoteEndPoint+BufferList");

			complete = false;
			saea.BufferList = null;
			saea.Completed += new EventHandler<SocketAsyncEventArgs> (CompletedAccessDenied);
			Enqueue (() => {
				Assert.IsTrue (s.ConnectAsync (saea), "SocketAsyncEventArgs");
			});
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		void CompletedAccessDenied (object sender, SocketAsyncEventArgs e)
		{
			Assert.IsNotNull (e);
			Assert.IsNull (e.Buffer, "Buffer");
			Assert.IsNull (e.BufferList, "BufferList");
			Assert.AreEqual (0, e.BytesTransferred, "BytesTransferred");
			Assert.IsNull (e.ConnectSocket, "ConnectSocket");
			Assert.AreEqual (0, e.Count, "Count");
			Assert.AreEqual (SocketAsyncOperation.Connect, e.LastOperation, "LastOperation");
			Assert.AreEqual (0, e.Offset, "Count");
			Assert.AreEqual ("Unspecified/localhost:0", e.RemoteEndPoint.ToString (), "RemoteEndPoint");
			Assert.AreEqual (SocketError.AccessDenied, e.SocketError, "SocketError");
			Assert.IsNull (e.UserToken, "UserToken");
			complete = true;
		}

		[TestMethod]
		public void CancelConnectAsync ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				Socket.CancelConnectAsync (null);
			}, "null");

			SocketAsyncEventArgs saea = new SocketAsyncEventArgs ();
			Assert.IsNull (saea.RemoteEndPoint, "RemoteEndPoint");
			Socket.CancelConnectAsync (saea);
			// no exception
		}
	}
}

