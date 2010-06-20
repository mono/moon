//
// System.Windows.Browser.Net.BrowserHttpWebResponse class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System.Globalization;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;

using Mono;

namespace System.Windows.Browser.Net
{
	sealed class BrowserHttpWebResponse : HttpWebResponse
	{
		HttpWebRequest request;
		Stream response;
		bool aborted;
		internal string method;

		int real_status_code;
		string status_desc;
		GCHandle handle;
		bool disposed;

		public BrowserHttpWebResponse (HttpWebRequest request, IntPtr native)
		{
			this.request = request;
			this.response = new MemoryStream ();
			this.aborted = false;
			Headers = new WebHeaderCollection ();
			method = request.Method;

			if (native == IntPtr.Zero)
				return;
			
			// Get the status code and status text asap, this way we don't have to 
			// ref/unref the native ptr
			real_status_code = NativeMethods.downloader_response_get_response_status (native);

			// Silverlight only returns OK or NotFound - but we keep the real value for ourselves
			switch (real_status_code) {
			case 200:
			case 404:
				status_desc = NativeMethods.downloader_response_get_response_status_text (native);
				break;
			default:
				status_desc = "Requested resource was not found";
				break;
			}

			// TODO: this is leaking in certain cases
			handle = GCHandle.Alloc (this);
			NativeMethods.downloader_response_set_header_visitor (native, OnHttpHeader, GCHandle.ToIntPtr (handle));
		}

		~BrowserHttpWebResponse () /* thread-safe: no p/invokes */
		{
			InternalAbort ();
		}

		void Dispose ()
		{
			if (!disposed) {
				handle.Free ();
				disposed = true;
			}
		}

		public void Abort ()
		{
			Dispose ();
			InternalAbort ();
		}

		internal void InternalAbort () {
			if (aborted)
				return;
			
			aborted = true;
		}

		static void OnHttpHeader (IntPtr context, IntPtr name, IntPtr value)
		{
			try {
				BrowserHttpWebResponse response = (BrowserHttpWebResponse) GCHandle.FromIntPtr (context).Target;
				response.Headers [Marshal.PtrToStringAnsi (name)] = Marshal.PtrToStringAnsi (value);
			} catch (Exception ex) {
				Console.WriteLine ("Error while retrieving http header: {0}", ex);
			}
		}

		public override void Close ()
		{
			response.Dispose ();
		}

		internal void Write (IntPtr buffer, int count)
		{
			byte[] data = new byte [count];
			Marshal.Copy (buffer, data, 0, count);
			response.Write (data, 0, count);
		}

		public override Stream GetResponseStream ()
		{
			response.Seek (0, SeekOrigin.Begin);
			// the stream we return must be read-only, so we wrap arround our MemoryStream
			return new InternalWebResponseStreamWrapper (response);
		}

		public override long ContentLength {
			get {
				long result;
				if (!Int64.TryParse (Headers ["Content-Length"], out result))
					result = 0;
				return (IsCompressed && (response.Length > result)) ? response.Length : result;
			}
		}

		public override string ContentType {
			get { return Headers [HttpRequestHeader.ContentType]; }
		}

		// this returns the "original" Method (if there was redirection involved)
		public override string Method {
			get { return method; }
		}

		internal bool IsCompressed {
			get {
				// http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
				switch (Headers ["Content-Encoding"]) {
				case "gzip":
				case "compress":
				case "zlib":
					return true;
				default:
					return false;
				}
			}
		}

		// this returns the last Uri (if there was redirection involved)
		public override Uri ResponseUri {
			get { return request.RequestUri; }
		}

		// since Silverlight hides most of this data, we keep it available for the BCL
		internal int RealStatusCode {
			get {
				return real_status_code;
			}
		}

		public override HttpStatusCode StatusCode {
			get {
				return (RealStatusCode == 200) ? HttpStatusCode.OK : HttpStatusCode.NotFound;
			}
		}

		public override string StatusDescription {
			get {
				return status_desc;
			}
		}
	}
}

#endif
