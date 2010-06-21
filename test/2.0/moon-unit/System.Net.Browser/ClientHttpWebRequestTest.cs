//
// Unit tests for System.Net.Browser.ClientHttpWebRequest
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
using System.Net.Browser;
using System.Security;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Browser {

	[TestClass]
	public class ClientHttpWebRequestTest : BaseHttpWebRequestTest {

		protected override WebRequest GetWebRequest (Uri uri)
		{
			return WebRequestCreator.ClientHttp.Create (uri);
		}

		[TestMethod]
		public void Buffering_Set ()
		{
			HttpWebRequest wr = (HttpWebRequest) GetWebRequest (new Uri ("http://localhost"));
			wr.AllowReadStreamBuffering = false;
			wr.AllowWriteStreamBuffering = false;
		}

		[TestMethod]
		public void CookieContainer ()
		{
			HttpWebRequest wr = (HttpWebRequest) GetWebRequest (new Uri ("http://localhost"));
			Assert.IsTrue (wr.SupportsCookieContainer, "SupportsCookieContainer");
			Assert.IsNull (wr.CookieContainer, "CookieContainer");
		}

		[TestMethod]
		public void Credentials ()
		{
			WebRequest wr = GetWebRequest (localhost);
			Assert.IsNull (wr.Credentials, "get");
			wr.Credentials = new NetworkCredential ("me", "****", "here");
			Assert.IsNotNull (wr.Credentials, "set");
			wr.Credentials = null;
			Assert.IsNull (wr.Credentials, "reset");
		}

		[TestMethod]
		public void Methods_Extended ()
		{
			WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
			Assert.AreEqual ("GET", wr.Method, "GET");

			wr.Method = "get";
			Assert.AreEqual ("get", wr.Method, "get");

			wr.Method = "poSt";
			Assert.AreEqual ("poSt", wr.Method, "poSt");

			wr.Method = "HEAD";
			Assert.AreEqual ("HEAD", wr.Method, "HEAD");

			wr.Method = "PUT";
			Assert.AreEqual ("PUT", wr.Method, "PUT");

			wr.Method = "DELETE";
			Assert.AreEqual ("DELETE", wr.Method, "DELETE");

			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "TRACE";
			}, "TRACE");

			wr.Method = "OPTIONS";
			Assert.AreEqual ("OPTIONS", wr.Method, "OPTIONS");

			Assert.Throws<ArgumentException> (delegate {
				wr.Method = null;
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				wr.Method = String.Empty;
			}, "Empty");
		}

		[TestMethod]
		[Asynchronous]
		public void HeadersGetThrowingInGetResponse ()
		{
			// NotSupportedException is thrown on the browser stack
			TryWebRequest ("GET", "Cache-Control", typeof (SecurityException));

			Type pve = typeof (ProtocolViolationException);
			TryWebRequest ("GET", "Content-Encoding", pve);
			TryWebRequest ("GET", "Content-Language", pve);
			TryWebRequest ("GET", "Content-MD5", pve);
			TryWebRequest ("GET", "Expires", pve);

			EnqueueTestComplete ();
		}
	}
}

