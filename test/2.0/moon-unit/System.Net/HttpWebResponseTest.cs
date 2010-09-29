//
// Unit tests for System.Net.HttpWebResponse
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
using System.IO;
using System.Net;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	class ConcreteHttpWebResponse : HttpWebResponse {

		public override void Close ()
		{
			throw new NotImplementedException ();
		}

		public override long ContentLength {
			get { throw new NotImplementedException (); }
		}

		public override string ContentType {
			get { throw new NotImplementedException (); }
		}

		public override Stream GetResponseStream ()
		{
			throw new NotImplementedException ();
		}

		public override Uri ResponseUri	{
			get { throw new NotImplementedException (); }
		}
	}

	[TestClass]
	public class HttpWebResponseTest {

		[TestMethod]
		public void Defaults ()
		{
			ConcreteHttpWebResponse hwr = new ConcreteHttpWebResponse ();

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.ContentLength);
			}, "ContentLength-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.ContentType);
			}, "ContentType-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.Cookies);
			}, "Cookies-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.Method);
			}, "Method-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.ResponseUri);
			}, "ResponseUri-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.StatusCode);
			}, "StatusCode-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (hwr.StatusDescription);
			}, "StatusDescription-get");


			Assert.Throws<NotImplementedException> (delegate {
				hwr.Close ();
			}, "Close");

			Assert.Throws<NotImplementedException> (delegate {
				hwr.GetResponseStream ();
			}, "GetResponseStream");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNotNull (hwr.Headers);
			}, "Headers");
			Assert.IsFalse (hwr.SupportsHeaders, "SupportsHeaders");
		}
	}
}

