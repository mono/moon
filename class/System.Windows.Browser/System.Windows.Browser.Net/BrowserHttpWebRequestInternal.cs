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
using System.Net;
using System.Runtime.InteropServices;
using System.Windows.Threading;

using Mono;
using Mono.Xaml;

namespace System.Windows.Browser.Net {

	// This class maps with the browser. 
	// One request is one exchange with the server.

	sealed class BrowserHttpWebRequestInternal : HttpWebRequest {
		IntPtr native;
		GCHandle managed;
		IntPtr downloader;
		Uri uri;
		long bytes_read;
		bool aborted;
		bool allow_read_buffering;
		string method = "GET";

		InternalWebRequestStreamWrapper request;
		BrowserHttpWebResponse response;
		BrowserHttpWebAsyncResult async_result;
		
		DownloaderResponseStartedDelegate started;
		DownloaderResponseAvailableDelegate available;
		DownloaderResponseFinishedDelegate finished;

		WebHeaderCollection headers;
 		
 		internal Action<long,long,object> progress;

 		public BrowserHttpWebRequestInternal (Uri uri)
		{
			started = new DownloaderResponseStartedDelegate (OnAsyncResponseStartedSafe);
			available = new DownloaderResponseAvailableDelegate (OnAsyncDataAvailableSafe);
			finished = new DownloaderResponseFinishedDelegate (OnAsyncResponseFinishedSafe);
 			this.uri = uri;
			managed = GCHandle.Alloc (this, GCHandleType.Normal);
			aborted = false;
		}

 		public BrowserHttpWebRequestInternal (BrowserHttpWebRequest wreq, Uri uri)
			: this (uri)
 		{
			allow_read_buffering = wreq.AllowReadStreamBuffering;
			method = wreq.Method;
			request = wreq.request;
			Headers = wreq.Headers;
			CookieContainer = wreq.CookieContainer; // FIXME
		}

		~BrowserHttpWebRequestInternal () /* thread-safe: all p/invokes are thread-safe */
		{
			Abort ();

			if (async_result != null)
				async_result.Dispose ();

			if (native != IntPtr.Zero)
				NativeMethods.downloader_request_free (native); /* this is thread-safe since this instance is the only place that has access to the native ptr */ 
			
			if (downloader != IntPtr.Zero)
				NativeMethods.event_object_unref (downloader); /* thread-safe */
		}

		// FIXME: to be moved to client stack only - but needed for SL3 as long as we share a single stack
		public override CookieContainer CookieContainer {
			get;
			set;
		}

		public override WebHeaderCollection Headers {
			get {
				if (headers == null)
					headers = new WebHeaderCollection ();
				return headers;
			}
			set { headers = value; }
		}

		public override void Abort ()
		{
			if (native == IntPtr.Zero)
				return;
			
			if (response != null)
				response.InternalAbort ();

			InternalAbort ();
		}

		internal void InternalAbort ()
		{
			aborted = true;
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			Console.WriteLine ("BrowserHttpWebRequest.BeginGetRequestStream: Should be async, doing it sync for now.");

			BrowserHttpWebAsyncResult result = new BrowserHttpWebAsyncResult (callback, state);
			result.SetComplete ();
			return result;
		}

		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			// we're not allowed to reuse an aborted request
			if (aborted)
				throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);

			async_result = new BrowserHttpWebAsyncResult (callback, state);

			new Dispatcher ().BeginInvoke (new Action (InitializeNativeRequestSafe), null);

			return async_result;
		}

		static uint OnAsyncResponseStartedSafe (IntPtr native, IntPtr context)
		{
			try {
				return OnAsyncResponseStarted (native, context);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BrowserHttpWebRequest.OnAsyncResponseStartedSafe: {0}", ex);
				} catch {
				}
			}
			return 0;
		}

		static BrowserHttpWebRequestInternal BrowserFromHandle (IntPtr context)
		{
			// accessing [SecurityCritical] GCHandle.Target inside its own method
			return (BrowserHttpWebRequestInternal) GCHandle.FromIntPtr (context).Target;
		}

		static uint OnAsyncResponseStarted (IntPtr native, IntPtr context)
		{
			BrowserHttpWebRequestInternal obj = BrowserFromHandle (context);
			
			try {
				obj.bytes_read = 0;
				obj.async_result.Response = new BrowserHttpWebResponse (obj, native);
			} catch (Exception e) {
				obj.async_result.Exception = e;
			}
			return 0;
		}
		
		static uint OnAsyncResponseFinishedSafe (IntPtr native, IntPtr context, bool success, IntPtr data)
		{
			try {
				return OnAsyncResponseFinished (native, context, success, data);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BrowserHttpWebRequest.OnAsyncResponseFinishedSafe: {0}", ex);
				} catch {
				}
			}
			return 0;
		}
		
		static uint OnAsyncResponseFinished (IntPtr native, IntPtr context, bool success, IntPtr data)
		{
			BrowserHttpWebRequestInternal obj = BrowserFromHandle (context);
			BrowserHttpWebAsyncResult async_result = obj.async_result;
			try {
				if (obj.progress != null) {
					BrowserHttpWebResponse response = async_result.Response;
					// report the 100% progress on compressed (or without Content-Length)
					if (response.IsCompressed) {
						long length = response.ContentLength;
						obj.progress.DynamicInvoke (new object[] { length, length, async_result.AsyncState});
					}
				}
			}
			finally {
				async_result.SetComplete ();
			}
			return 0;
		}
		
		static uint OnAsyncDataAvailableSafe (IntPtr native, IntPtr context, IntPtr data, uint length)
		{
			try {
				return OnAsyncDataAvailable (native, context, data, length);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BrowserHttpWebRequest.OnAsyncDataAvailableSafe: {0}", ex);
				} catch {
				}
			}
			return 0;
		}
		
		static uint OnAsyncDataAvailable (IntPtr native, IntPtr context, IntPtr data, uint length)
		{
			BrowserHttpWebRequestInternal obj = BrowserFromHandle (context);
			BrowserHttpWebAsyncResult async_result = obj.async_result;
			BrowserHttpWebResponse response = async_result.Response;
			try {
				obj.bytes_read += length;
				if (obj.progress != null) {
					// if Content-Encoding is gzip (or other compressed) then Content-Length cannot be trusted,
					// if present, since it does not (generally) correspond to the uncompressed length
					long content_length = response.ContentLength;
					bool compressed = (response.IsCompressed || (content_length == 0));
					long total_bytes_to_receive = compressed ? -1 : content_length;
					obj.progress.DynamicInvoke (new object[] { obj.bytes_read, total_bytes_to_receive, async_result.AsyncState});
				}
			} catch (Exception e) {
				async_result.Exception = e;
			}

			try {
				response.Write (data, checked ((int) length));
			} catch (Exception e) {
				async_result.Exception = e;
			}
			return 0;
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			if (request == null) {
				request = new InternalWebRequestStreamWrapper (new MemoryStream ());
				request.WriteByte ((byte) '\n');
			}
			return request;
		}

		public override WebResponse EndGetResponse (IAsyncResult asyncResult)
		{
			try {
				if (async_result != asyncResult)
					throw new ArgumentException ("asyncResult");

				if (aborted) {
					NativeMethods.downloader_request_abort (native);
					throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);
				}

				if (!async_result.IsCompleted)
					async_result.AsyncWaitHandle.WaitOne ();

				if (async_result.HasException) {
					throw async_result.Exception;
				}

				response = async_result.Response;
			}
			finally {
				async_result.Dispose ();
				managed.Free ();
			}

			return response;
		}


		void InitializeNativeRequestSafe ()
		{
			try {
				InitializeNativeRequest ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BrowserHttpWebRequest.InitializeNativeRequestSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		void InitializeNativeRequest ()
		{
			downloader = NativeMethods.surface_create_downloader (XamlLoader.SurfaceInDomain);
			if (downloader == IntPtr.Zero)
				throw new NotSupportedException ("Failed to create unmanaged downloader");
			native = NativeMethods.downloader_create_web_request (downloader, method, uri.AbsoluteUri);
			if (native == IntPtr.Zero)
				throw new NotSupportedException ("Failed to create unmanaged WebHttpRequest object.  unsupported browser.");

			long request_length = 0;
			byte[] body = null;
			try {
				if (request == null) {
					request_length = 0;
				} else {
					request_length = request.Length;
					body = (request.InnerStream as MemoryStream).ToArray ();
				}
			}
			catch (ObjectDisposedException) {
				body = request.GetData ();
				request_length = body.Length;
			}

			if (request_length > 1) {
				// this header cannot be set directly inside the collection (hence the helper)
				Headers.SetHeader ("content-length", (request_length - 1).ToString ());
			}

			if (CookieContainer != null && CookieContainer.Count > 0) {
				string cookieHeader = CookieContainer.GetCookieHeader (uri);
				if (cookieHeader != "")
					Headers ["Cookie"] = cookieHeader;
			}

			foreach (string header in Headers.AllKeys)
				NativeMethods.downloader_request_set_http_header (native, header, Headers [header]);

			if (request_length > 1) {
				NativeMethods.downloader_request_set_body (native, body, body.Length);
			}
			
			NativeMethods.downloader_request_get_response (native, started, available, finished, GCHandle.ToIntPtr (managed));
		}

		public override bool HaveResponse {
			get {
				if (response != null)
					return true;
				if (async_result != null && async_result.Response != null)
					return true;
				return false;
			}
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
