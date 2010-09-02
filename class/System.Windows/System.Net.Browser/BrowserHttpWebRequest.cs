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

		public override bool AllowWriteStreamBuffering {
			get { return true; }
			set {
				// cannot be false - but can be set to (default) true
				if (!value)
					throw new NotSupportedException ();
			}
		}

		public override IWebRequestCreate CreatorInstance { 
			get { return WebRequestCreator.BrowserHttp; }
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			WebClient.CallbackData callback_data = (state as WebClient.CallbackData);
			if (callback_data != null) {
				long length = callback_data.data.Length;
				// if Content-Length has been set previously and does not match the data length...
				if ((ContentLength != -1) && (ContentLength != length))
					throw new ProtocolViolationException ();
				ContentLength = length;
			}

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

		protected override void CheckProtocolViolation ()
		{
			if (String.Compare (Method, "POST", StringComparison.OrdinalIgnoreCase) == 0) {
				// if the property Content-Length was not set (or set to zero) then Headers should not contain it
				if ((ContentLength > 0) && Headers.ContainsKey ("Content-Length"))
					throw new ProtocolViolationException ();
			} else {
				foreach (string header in Headers) {
					switch (header.ToLowerInvariant ()) {
					case "content-encoding":
					case "content-language":
					case "content-md5":
					case "content-type":
					case "expires":
						throw new ProtocolViolationException ();
					case "accept":
					case "authorization":
					case "cache-control":
					case "range":
						throw new NotSupportedException ();
					default:
						// cross-domain GET cannot set headers for the browser stack, see:
						// http://msdn.microsoft.com/en-us/library/cc838250(VS.95).aspx
						if (!IsSiteOfOrigin ())
							throw new NotSupportedException ("x");
						if (IsMultilineValue (Headers [header]))
							throw new NotSupportedException ();
						break;
					}
				}
			}
		}
	}
}

#endif
