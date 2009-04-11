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

			Assert.IsNull (hwr.Accept, "Accept-get");
			hwr.Accept = String.Empty;
			// still null
			Assert.IsNull (hwr.Accept, "Accept-set-null");
			hwr.Accept = "a";
			Assert.AreEqual ("a", hwr.Accept, "Accept-set");
			// reset to null with empty
			hwr.Accept = String.Empty;
			Assert.IsNull (hwr.Accept, "Accept-set-empty");

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

			Assert.IsNull (hwr.ContentType, "ContentType-get");
			hwr.ContentType = String.Empty;
			// still null
			Assert.IsNull (hwr.ContentType, "ContentType-set-null");
			hwr.ContentType = "a";
			Assert.AreEqual ("a", hwr.ContentType, "ContentType-set");
			// reset to null with empty
			hwr.ContentType = String.Empty;
			Assert.IsNull (hwr.ContentType, "ContentType-set-empty");

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
	}
}

