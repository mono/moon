//
// Unit tests for System.Net.WebResponse
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

	class ConcreteWebResponse : WebResponse {

		public override long ContentLength {
			get { throw new NotImplementedException (); }
		}

		public override string ContentType {
			get { throw new NotImplementedException (); }
		}

		public override Uri ResponseUri	{
			get { throw new NotImplementedException (); }
		}

		public override void Close ()
		{
			throw new NotImplementedException ();
		}

		public override Stream GetResponseStream ()
		{
			throw new NotImplementedException ();
		}
	}

	[TestClass]
	public class WebResponseTest {

		[TestMethod]
		public void Defaults ()
		{
			ConcreteWebResponse wr = new ConcreteWebResponse ();

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (wr.ContentLength);
			}, "ContentLength-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (wr.ContentType);
			}, "ContentType-get");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNull (wr.ResponseUri);
			}, "ResponseUri-get");

			Assert.Throws<NotImplementedException> (delegate {
				wr.Close ();
			}, "Close");

			Assert.Throws<NotImplementedException> (delegate {
				wr.GetResponseStream ();
			}, "GetResponseStream");

			Assert.Throws<NotImplementedException> (delegate {
				Assert.IsNotNull (wr.Headers);
			}, "Headers");
			Assert.IsFalse (wr.SupportsHeaders, "SupportsHeaders");
		}
	}
}


