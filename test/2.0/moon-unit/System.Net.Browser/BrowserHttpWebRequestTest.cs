//
// Unit tests for System.Net.Browser.BrowserHttpWebRequest
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

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Browser {

	[TestClass]
	public class BrowserHttpWebRequestTest : BaseHttpWebRequestTest {

		protected override WebRequest  GetWebRequest(Uri uri)
		{
			return WebRequestCreator.BrowserHttp.Create (uri);
		}

		[TestMethod]
		public void Buffering_Set ()
		{
			HttpWebRequest wr = (HttpWebRequest) GetWebRequest (new Uri ("http://localhost"));
			wr.AllowReadStreamBuffering = false;
			Assert.Throws<NotSupportedException> (delegate {
				wr.AllowWriteStreamBuffering = false;
			}, "AllowWriteStreamBuffering-false");
			// that's ok
			wr.AllowWriteStreamBuffering = true;
		}

		[TestMethod]
		public void CookieContainer ()
		{
			HttpWebRequest wr = (HttpWebRequest) GetWebRequest (new Uri ("http://localhost"));
			Assert.IsFalse (wr.SupportsCookieContainer, "SupportsCookieContainer");
			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNotNull (wr.CookieContainer);
			}, "CookieContainer");
		}

		[TestMethod]
		public void Credentials ()
		{
			WebRequest wr = GetWebRequest (localhost);
			Assert.Throws<NotImplementedException> (delegate {
				// not sure then this is enabled (both browser and client throws)
				Assert.IsNotNull (wr.Credentials);
			}, "Credentials-get");
			Assert.Throws<NotImplementedException> (delegate {
				wr.Credentials = new NetworkCredential ("me", "****", "here");
			}, "Credentials-set");
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

			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "HEAD";
			}, "HEAD");
			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "PUT";
			}, "PUT");
			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "DELETE";
			}, "DELETE");
			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "TRACE";
			}, "PUT");
			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = "OPTIONS";
			}, "OPTIONS");

			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = null;
			}, "null");
			Assert.Throws<NotSupportedException> (delegate {
				wr.Method = String.Empty;
			}, "Empty");
		}

		[TestMethod]
		[Asynchronous]
		public void HeadersGetThrowingInGetResponse ()
		{
			TryWebRequest ("GET", "Cache-Control", typeof (NotSupportedException));

			Type pve = typeof (ProtocolViolationException);
			TryWebRequest ("GET", "Content-Encoding", pve);
			TryWebRequest ("GET", "Content-Language", pve);
			TryWebRequest ("GET", "Content-MD5", pve);
			TryWebRequest ("GET", "Expires", pve);

			EnqueueTestComplete ();
		}
	}
}

