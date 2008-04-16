//
// System.Windows.Browser.Net.BrowserHttpWebRequest class
//
// Authors:
//	Jb Evain  <jbevain@novell.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using System;
using System.Collections;
using System.Globalization;
using System.IO;
using System.Net;

using Mono;

namespace System.Windows.Browser.Net
{
	class BrowserHttpWebRequest : HttpWebRequest
	{
		IntPtr native;
		Uri uri;
		string method = "GET";
		WebHeaderCollection headers = new WebHeaderCollection ();
		MemoryStream request;
		BrowserHttpWebResponse response;
		BrowserHttpWebAsyncResult async_result;

		public BrowserHttpWebRequest (Uri uri)
		{
			this.uri = uri;
		}

		~BrowserHttpWebRequest ()
		{
			if (async_result != null)
				async_result.Dispose ();

			if (native == IntPtr.Zero)
				return;

			NativeMethods.browser_http_request_destroy (native);
		}

		public override void Abort ()
		{
			if (native == IntPtr.Zero)
				return;

			NativeMethods.browser_http_request_abort (native);
		}

		static bool IsSameRangeKind (string range, string rangeSpecifier)
		{
			return range.ToLower (CultureInfo.InvariantCulture).StartsWith (
				rangeSpecifier.ToLower (CultureInfo.InvariantCulture));
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			throw new NotImplementedException ();
		}

		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			InitializeNativeRequest ();

			async_result = new BrowserHttpWebAsyncResult (callback, state);
			if (!NativeMethods.browser_http_request_get_async_response (native, OnAsyncResponseAvailable, IntPtr.Zero))
				throw new InvalidOperationException ();

			return async_result;
		}

		void OnAsyncResponseAvailable (IntPtr native, IntPtr context)
		{
			try {
				async_result.Response = new BrowserHttpWebResponse (this, native);
				async_result.SetComplete ();
			} catch (Exception e) {
				async_result.Exception = e;
			}
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			throw new NotImplementedException ();
		}

		public override WebResponse EndGetResponse (IAsyncResult asyncResult)
		{
			if (async_result != asyncResult)
				throw new ArgumentException ();

			if (!async_result.IsCompleted)
				async_result.AsyncWaitHandle.WaitOne ();

			if (async_result.HasException) {
				async_result.Dispose ();
				throw async_result.Exception;
			}

			response = async_result.Response;
			async_result.Dispose ();

			response.Read ();
			return response;
		}

		public Stream GetRequestStream ()
		{
			if (request == null)
				request = new MemoryStream ();

			return request;
		}

		static Uri GetBaseUri ()
		{
			//FIXME: there's most probably a better way to do this.

			string uri = HtmlPage.DocumentUri.AbsoluteUri;
			return new Uri (uri.Substring (0, uri.LastIndexOf ("/") + 1));
		}

		static Uri GetAbsoluteUri (Uri uri)
		{
			return new Uri (GetBaseUri (), uri);
		}

		void InitializeNativeRequest ()
		{
			if (native != IntPtr.Zero)
				return;

			Uri request_uri = uri.IsAbsoluteUri ? uri : GetAbsoluteUri (uri);

			native = NativeMethods.browser_http_request_new (method, request_uri.AbsoluteUri);

			foreach (string header in headers.Headers)
				NativeMethods.browser_http_request_set_header (native, header, headers [header]);

			if (request != null && request.Length > 0) {
				byte [] body = request.ToArray ();
				NativeMethods.browser_http_request_set_body (native, body, body.Length);
			}
		}

		public HttpWebResponse GetResponse ()
		{
			InitializeNativeRequest ();

			IntPtr resp = NativeMethods.browser_http_request_get_response (native);

			if (resp == IntPtr.Zero)
				throw new InvalidOperationException ();

			response = new BrowserHttpWebResponse (this, resp);
			response.Read ();

			return response;
		}

		public override string ContentType {
			get { return headers [HttpRequestHeader.ContentType]; }
			set { headers [HttpRequestHeader.ContentType] = value; }
		}

		public override bool HaveResponse {
			get { return response != null; }
		}

		public override WebHeaderCollection Headers {
			get { return headers; }
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
