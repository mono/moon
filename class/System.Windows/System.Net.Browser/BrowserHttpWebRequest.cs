//
// System.Windows.Browser.Net.BrowserHttpWebRequest class
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
using System.Net.Policy;

namespace System.Net.Browser {

	// This class maps with Silverlight exposed behavior for the browser http stack.
	// One request can represent multiple connections with a server.
	// e.g. requesting a policy for cross-domain requests
	// e.g. http redirection

	sealed class BrowserHttpWebRequest : PolicyBasedWebRequest {

		internal InternalWebRequestStreamWrapper request;

 		
 		public BrowserHttpWebRequest (Uri uri)
			: base (uri)
 		{
		}

		public override IWebRequestCreate CreatorInstance { 
			get { return WebRequestCreator.BrowserHttp; }
		}

		public override void Abort ()
		{
			BrowserHttpWebResponse wresp = (response as BrowserHttpWebResponse);
			if (wresp != null)
				wresp.Abort ();

			base.Abort ();
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			HttpWebAsyncResult result = new HttpWebAsyncResult (callback, state);
			result.SetComplete ();
			return result;
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			if (request == null)
				request = new InternalWebRequestStreamWrapper (new MemoryStream ());

			return request;
		}

		public override bool HaveResponse {
			get {
				if (response != null)
					return true;
				return ((async_result != null && async_result.Response != null));
			}
		}

		public override bool UseDefaultCredentials {
			get { return true; }
			set {
				// this is the default (and unchangeable) behavior of the browser stack
				if (!value)
					throw new NotSupportedException ();
			}
		}

		protected override HttpWebRequest GetHttpWebRequest (Uri uri)
		{
			return new BrowserHttpWebRequestInternal (this, uri);
		}

		protected override void CheckMethod (string method)
		{
			if (method == null)
				throw new NotSupportedException ("method");

			switch (method.ToLowerInvariant ()) {
			case "get":
			case "post":
				break;
			default:
				throw new NotSupportedException ();
			}
		}

		static string[] bad_get_headers = { "Content-Encoding", "Content-Language", "Content-MD5", "Expires", "Content-Type" };

		protected override void CheckProtocolViolation ()
		{
			if (String.Compare (Method, "GET", StringComparison.OrdinalIgnoreCase) != 0)
				return;

			if (Headers.ContainsKey ("Cache-Control"))
				throw new NotSupportedException ();

			// most headers are checked when set, but some are checked much later
			foreach (string header in bad_get_headers) {
				// case insensitive check to internal Headers dictionary
				if (Headers.ContainsKey (header))
					throw new ProtocolViolationException ();
			}
		}
	}
}

#endif
