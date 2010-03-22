//
// System.Windows.Browser.Net.HttpWebRequestCore class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007,2009-2010 Novell, Inc (http://www.novell.com)
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

#if NET_2_1

using System.IO;

namespace System.Net.Browser {

	// This class implement the base core of both the actual request for the browser and client http stacks. 
	class HttpWebRequestCore : HttpWebRequest {

		private Uri uri;
		private string method;
		private WebHeaderCollection headers;

		public HttpWebRequestCore (HttpWebRequest parent, Uri uri)
		{
			if (uri == null)
				throw new ArgumentNullException ("uri");

			this.uri = uri;
			if (parent == null) {
				// special case used for policy
				method = "GET";
			} else {
				method = parent.Method;
				headers = parent.Headers;
			}
		}

		public override WebHeaderCollection Headers {
			get {
				if (headers == null)
					headers = new WebHeaderCollection ();
				return headers;
			}
			set { headers = value; }
		}

		public override string Method {
			get { return method; }
			set { method = value; }
		}

		public override Uri RequestUri {
			get { return uri; }
		}
	}
}

#endif

