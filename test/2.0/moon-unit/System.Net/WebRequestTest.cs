//
// Unit tests for System.Net.WebRequest
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
using System.IO;
using System.Net;
using System.Net.Sockets;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	class ConcreteWebRequest : WebRequest, IWebRequestCreate {

		private Uri uri;

		public ConcreteWebRequest ()
		{
		}

		internal ConcreteWebRequest (Uri uri)
		{
			this.uri = uri;
		}

		public override void Abort ()
		{
			throw new NotImplementedException ();
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			throw new NotImplementedException ();
		}

		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			throw new NotImplementedException ();
		}

		public override string ContentType {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}

		public override global::System.IO.Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			throw new NotImplementedException ();
		}

		public override WebResponse EndGetResponse (IAsyncResult asyncResult)
		{
			throw new NotImplementedException ();
		}

		public override WebHeaderCollection Headers {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}

		public override string Method {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}

		public override Uri RequestUri {
			get { return uri; }
		}

		WebRequest IWebRequestCreate.Create (Uri uri)
		{
			return new ConcreteWebRequest (uri);
		}
	}

	[TestClass]
	public class WebRequestTest {

		[TestMethod]
		public void RegisterPrefix ()
		{
			IWebRequestCreate creator = (IWebRequestCreate) new ConcreteWebRequest ();
			Assert.Throws<ArgumentNullException> (delegate {
				WebRequest.RegisterPrefix (null, creator);
			}, "null,creator");
			Assert.Throws<ArgumentNullException> (delegate {
				WebRequest.RegisterPrefix ("ftp", null);
			}, "ftp,null");

			Assert.IsTrue (WebRequest.RegisterPrefix (String.Empty, creator), "Empty");
			Assert.IsTrue (WebRequest.RegisterPrefix ("ftp", creator), "ftp-1");
			// not twice
			Assert.IsFalse (WebRequest.RegisterPrefix ("ftp", creator), "ftp-2");
			// not case sensitive
			Assert.IsFalse (WebRequest.RegisterPrefix ("FTP", creator), "ftp-3");
		}
#if SL3
		// the string overload exists only in Silverlight 3 (beta)
		[TestMethod]
		public void Create_String ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				WebRequest.Create ((string) null);
			}, "string-null");

			Assert.Throws<NotSupportedException> (delegate {
				WebRequest.Create ("unknown://localhost/");
			}, "unknown");

			Assert.Throws<UriFormatException> (delegate {
				WebRequest.Create ("/myfile");
			}, "relative");
		}
#endif
		[TestMethod]
		public void Create_Uri ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				WebRequest.Create ((Uri) null);
			}, "Uri-null");

			Uri uri = new Uri ("unknown://localhost/");
			Assert.Throws<NotSupportedException> (delegate {
				WebRequest.Create (uri);
			}, "unknown");

			uri = new Uri ("/myfile", UriKind.Relative);
			Assert.Throws<InvalidOperationException> (delegate {
				WebRequest.Create (uri);
			}, "relative");
		}

		[TestMethod]
		public void RegisterCustomPrefix ()
		{
			IWebRequestCreate creator = (IWebRequestCreate) new ConcreteWebRequest ();
			Assert.IsTrue (WebRequest.RegisterPrefix ("httpx", creator), "httpx-1");
			Assert.IsFalse (WebRequest.RegisterPrefix ("httpx", creator), "httpx-2");

			WebRequest wr = WebRequest.Create (new Uri ("httpx://localhost/x"));
			Assert.AreEqual ("httpx://localhost/x", wr.RequestUri.OriginalString, "RequestUri");
			Assert.IsTrue (wr is ConcreteWebRequest, "ConcreteWebRequest");
		}

		[TestMethod]
		public void ReRegisterHttp ()
		{
			IWebRequestCreate creator = (IWebRequestCreate) new ConcreteWebRequest ();
			// according to documentation we cannot register something else for http
			// because it would "fail" since they are already "sysetm registered".
			// however it seems no one told the API
			Assert.IsTrue (WebRequest.RegisterPrefix ("http", creator), "http-1");
			// but you can't register twice (like any other)
			Assert.IsFalse (WebRequest.RegisterPrefix ("http", creator));

			WebRequest wr = WebRequest.Create (new Uri ("http://localhost/"));
			Assert.AreEqual ("http://localhost/", wr.RequestUri.OriginalString, "RequestUri");
			// works but it's not using our ConcreteWebRequest but an internal type
			Assert.IsFalse (wr is ConcreteWebRequest, "ConcreteWebRequest");
		}

		[TestMethod]
		public void ReRegisterHttps ()
		{
			IWebRequestCreate creator = (IWebRequestCreate) new ConcreteWebRequest ();
			// according to documentation we cannot register something else for https
			// because it would "fail" since they are already "sysetm registered".
			// however it seems no one told the API
			Assert.IsTrue (WebRequest.RegisterPrefix ("https", creator), "https-1");
			// but you can't register twice (like any other)
			Assert.IsFalse (WebRequest.RegisterPrefix ("https", creator), "https-2");

			WebRequest wr = WebRequest.Create (new Uri ("https://localhost/"));
			Assert.AreEqual ("https://localhost/", wr.RequestUri.OriginalString, "RequestUri");
			// works but it's not using our ConcreteWebRequest but an internal type
			Assert.IsFalse (wr is ConcreteWebRequest, "ConcreteWebRequest");
		}
	}
}

