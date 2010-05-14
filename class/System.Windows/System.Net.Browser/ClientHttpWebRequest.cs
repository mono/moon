//
// System.Net.Browser.ClientHttpWebRequest
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
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

#if NET_2_1

using System.IO;
using System.Security;

namespace System.Net.Browser {

	// this type inherits System.Net (SL) version of HttpWebRequest while it use (internally) System.dll version of the same
	// (named) class
	internal class ClientHttpWebRequest : PolicyBasedWebRequest {

		internal ClientHttpWebRequestInternal request;
		private Stream stream;

		internal ClientHttpWebRequest (Uri uri)
			: base (uri)
		{
		}

		public override ICredentials Credentials {
			get; set;
		}

		public override CookieContainer CookieContainer {
			get; set;
		}

		public override IWebRequestCreate CreatorInstance { 
			get { return WebRequestCreator.ClientHttp; }
		}

		public override bool HaveResponse {
			get { return (response != null); }
		}

		public override bool SupportsCookieContainer {
			get { return true; }
		}

		public override bool UseDefaultCredentials {
			get; set;
		}

		public override void Abort ()
		{
			if (request != null)
				request.Abort ();

			base.Abort ();
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			return GetHttpWebRequest (RequestUri).BeginGetRequestStream (callback, state);
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			if (stream == null)
				stream = GetHttpWebRequest (RequestUri).EndGetRequestStream (asyncResult);

			return stream;
		}

		protected override HttpWebRequest GetHttpWebRequest (Uri uri)
		{
			if (request == null)
				request = new ClientHttpWebRequestInternal (this, uri);
			return request;
		}

		static bool CheckCharacters (string s)
		{
			if (String.IsNullOrEmpty (s))
				return false;

			foreach (char c in s) {
				if (Char.IsWhiteSpace (c) || !Char.IsLetterOrDigit (c) || ((int) c > 127))
					return false;
			}
			return true;
		}

		protected override void CheckMethod (string method)
		{
			// whitespaces, non-7bits ascii characters and special characters are not allowed
			if (!CheckCharacters (method))
				throw new ArgumentException ("method");

			switch (method.ToLowerInvariant ()) {
			// some verbs are not allowed
			case "connect":
			case "trace":
			case "track":
				throw new NotSupportedException ();
			default:
				break;
			}
		}

		static string[] bad_get_headers = { "Content-Encoding", "Content-Language", "Content-MD5", "Expires" };

		protected override void CheckProtocolViolation ()
		{
			if (Headers.ContainsKey ("Cache-Control"))
				throw new SecurityException ();

			bool is_get = (String.Compare (Method, "GET", StringComparison.OrdinalIgnoreCase) == 0);

			// most headers are checked when set, but some are checked much later
			foreach (string header in bad_get_headers) {
				// case insensitive check to internal Headers dictionary
				if (Headers.ContainsKey (header)) {
					if (is_get)
						throw new ProtocolViolationException ();
					else
						throw new SecurityException ();
				}
			}
		}
	}
}

#endif

