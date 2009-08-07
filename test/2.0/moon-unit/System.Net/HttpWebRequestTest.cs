//
// Unit tests for System.Net.HttpWebRequest
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

	class ConcreteHttpWebRequest : HttpWebRequest {
	}

	[TestClass]
	public class HttpWebRequestTest {

		[TestMethod]
		public void Defaults ()
		{
			ConcreteHttpWebRequest hwr = new ConcreteHttpWebRequest ();

			Assert.Throws<NotImplementedException> (delegate {
				hwr.Abort ();
			}, "Abort");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsFalse (hwr.AllowReadStreamBuffering);
			}, "AllowReadStreamBuffering-get");
			Assert.Throws<NotImplementedException> (delegate {
				hwr.AllowReadStreamBuffering = true;
			}, "AllowReadStreamBuffering-set");

			Assert.Throws<NotImplementedException> (delegate {
				hwr.BeginGetRequestStream (null, null);
			}, "BeginGetRequestStream");

			Assert.Throws<NotImplementedException> (delegate {
				hwr.BeginGetResponse (null, null);
			}, "BeginGetResponse");

			Assert.Throws<NotImplementedException> (delegate {
				hwr.EndGetRequestStream (null);
			}, "EndGetRequestStream");

			Assert.Throws<NotImplementedException> (delegate {
				hwr.EndGetResponse (null);
			}, "EndGetResponse");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsFalse (hwr.HaveResponse);
			}, "HaveResponse");

			Assert.AreEqual (0, hwr.Headers.Count, "Headers-get");
			Assert.Throws<NullReferenceException> (delegate {
				hwr.Headers = null;
			}, "Headers-set-null");
			hwr.Headers = new WebHeaderCollection ();
			Assert.IsNotNull (hwr.Headers, "Headers-set");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.Method);
			}, "Method-get");
			Assert.Throws<NotImplementedException> (delegate {
				hwr.Method = null;
			}, "Method-set");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.RequestUri);
			}, "RequestUri-get");
		}

		[TestMethod]
		public void Headers_Validation ()
		{
			// a WebHeaderCollection can contain any header
			WebHeaderCollection whc = new WebHeaderCollection ();
			whc [HttpRequestHeader.Allow] = "yup";

			ConcreteHttpWebRequest wr = new ConcreteHttpWebRequest ();
			WebHeaderCollection c2 = wr.Headers;
			c2 [HttpRequestHeader.CacheControl] = "often";
			Assert.Throws<ArgumentException> (delegate {
				wr.Headers = whc;
			}, "collection with bad header");
			Assert.AreEqual (1, c2.Count, "Count");
			Assert.AreEqual ("often", wr.Headers [HttpRequestHeader.CacheControl], "CacheControl");

			// this is NOT a field assignation but a copy of the data
			Assert.IsFalse (Object.ReferenceEquals (whc, wr.Headers), "Assigned?");
			whc [HttpRequestHeader.KeepAlive] = "sure";

			Assert.IsTrue (Object.ReferenceEquals (c2, wr.Headers), "NotAssigned?");
			Assert.Throws<ArgumentException> (delegate {
				c2 [HttpRequestHeader.KeepAlive] = "sure";
			}, "KeepAlive");
		}

		[TestMethod]
		public void Accept ()
		{
			// Fails in Silverlight 3
			ConcreteHttpWebRequest hwr = new ConcreteHttpWebRequest ();

			Assert.IsNull (hwr.Accept, "Accept-get");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.Accept], "Headers[HttpRequestHeader.Accept]-get");
			Assert.IsNull (hwr.Headers ["Accept"], "Headers['Accept']-get");
			Assert.AreEqual (0, hwr.Headers.Count, "Count-a");

			hwr.Accept = String.Empty;
			// still null
			Assert.IsNull (hwr.Accept, "Accept-set-null");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.Accept], "Headers[HttpRequestHeader.Accept]-set-null");
			Assert.IsNull (hwr.Headers ["Accept"], "Headers['Accept']-set-null");
			Assert.AreEqual (0, hwr.Headers.Count, "Count-b");

			hwr.Accept = "a";
			Assert.AreEqual ("a", hwr.Accept, "Accept-set");
			Assert.AreEqual ("a", hwr.Headers [HttpRequestHeader.Accept], "Headers[HttpRequestHeader.Accept]-set");
			Assert.AreEqual ("a", hwr.Headers ["Accept"], "Headers['Accept']-set");
			Assert.AreEqual (1, hwr.Headers.Count, "Count-c");

			// reset to null with empty
			hwr.Accept = String.Empty;
			Assert.AreEqual (0, hwr.Headers.Count, "Count-d");
			Assert.IsNull (hwr.Accept, "Accept-set-empty");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.Accept], "Headers[HttpRequestHeader.Accept]-set-empty");
			Assert.IsNull (hwr.Headers ["Accept"], "Headers['Accept']-set-empty");
		}

		[TestMethod]
		public void ContentType ()
		{
			// Fails in Silverlight 3
			ConcreteHttpWebRequest hwr = new ConcreteHttpWebRequest ();

			Assert.IsNull (hwr.ContentType, "ContentType-get");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.ContentType], "Headers[HttpRequestHeader.ContentType]-get");
			Assert.IsNull (hwr.Headers ["Content-Type"], "Headers['Content-Type']-get");
			Assert.AreEqual (0, hwr.Headers.Count, "Count-a");

			hwr.ContentType = String.Empty;
			// still null
			Assert.IsNull (hwr.ContentType, "ContentType-set-null");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.ContentType], "Headers[HttpRequestHeader.ContentType]-set-null");
			Assert.IsNull (hwr.Headers ["Content-Type"], "Headers['Content-Type']-set-null");
			Assert.AreEqual (0, hwr.Headers.Count, "Count-b");

			hwr.ContentType = "a";
			Assert.AreEqual ("a", hwr.ContentType, "ContentType-set");
			Assert.AreEqual ("a", hwr.Headers [HttpRequestHeader.ContentType], "Headers[HttpRequestHeader.ContentType]-set");
			Assert.AreEqual ("a", hwr.Headers ["Content-Type"], "Headers['Content-Type']-set");
			Assert.AreEqual (1, hwr.Headers.Count, "Count-c");

			// reset to null with empty
			hwr.ContentType = String.Empty;
			Assert.AreEqual (0, hwr.Headers.Count, "Count-d");
			Assert.IsNull (hwr.ContentType, "ContentType-set-empty");
			Assert.IsNull (hwr.Headers [HttpRequestHeader.ContentType], "Headers[HttpRequestHeader.ContentType]-set-empty");
			Assert.IsNull (hwr.Headers ["Content-Type"], "Headers['Content-Type']-set-empty");
		}
	}
}

