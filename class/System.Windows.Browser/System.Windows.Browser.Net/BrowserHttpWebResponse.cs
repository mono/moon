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
	class BrowserHttpWebResponse : HttpWebResponse
	{
		BrowserHttpWebRequest request;
		IntPtr native;
		Stream response;
		bool aborted;

		HttpStatusCode status_code;
		string status_desc;

		WebHeaderCollection headers = new WebHeaderCollection ();

		public BrowserHttpWebResponse (BrowserHttpWebRequest request, IntPtr native)
		{
			this.request = request;
			this.native = native;
			this.response = new MemoryStream ();
			this.aborted = false;

			if (native == IntPtr.Zero)
				return;

			NativeMethods.downloader_response_set_header_visitor (native, OnHttpHeader);
		}

		~BrowserHttpWebResponse ()
		{
			if (native == IntPtr.Zero)
				return;
			
			/* FIXME: Firefox will be releasing this object automatically but the managed side
			 * really doesn't know when this will happen
			 * Abort ();
			 *
			 * NativeMethods.downloader_response_free (native);
			 */
		}

		public void Abort ()
		{
			if (native == IntPtr.Zero)
				return;

			if (request != null)
				request.InternalAbort ();
			
			InternalAbort ();
		}

		internal void InternalAbort () {
			if (aborted)
				return;

			/* FIXME: Firefox will be releasing this object automatically but the managed side
			 * really doesn't know when this will happen
			 * NativeMethods.downloader_response_abort (native);
			 */
			aborted = true;
		}

		void OnHttpHeader (IntPtr name, IntPtr value)
		{
			try {
				headers [Marshal.PtrToStringAnsi (name)] = Marshal.PtrToStringAnsi (value);
			} catch {}
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
			BrowserHttpWebStreamWrapper stream = new BrowserHttpWebStreamWrapper (response);
			stream.SetReadOnly ();
			return stream;
		}

		public override long ContentLength {
			get {
				return long.Parse (headers ["Content-Length"]);
			}
		}

		public override string ContentType {
			get { return headers [HttpRequestHeader.ContentType]; }
		}

		public override string Method {
			get { return request.Method; }
		}


		public override Uri ResponseUri {
			get { return request.RequestUri; }
		}

		internal void GetStatus ()
		{
			if(0 != (int) status_code)
				return;

			if (native == IntPtr.Zero)
				return;

			status_desc = NativeMethods.downloader_response_get_response_status_text (native);
			status_code = (HttpStatusCode) NativeMethods.downloader_response_get_response_status (native);
		}

		public override HttpStatusCode StatusCode {
			get {
				GetStatus ();
				return status_code;
			}
		}

		public override string StatusDescription {
			get {
				GetStatus ();
				return status_desc;
			}
		}
	}
}

#endif
