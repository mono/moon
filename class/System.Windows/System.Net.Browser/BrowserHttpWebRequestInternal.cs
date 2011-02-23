//
// System.Windows.Browser.Net.BrowserHttpWebRequestInternal class
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
using System.Runtime.InteropServices;
using System.Windows.Threading;

using Mono;
using Mono.Xaml;

namespace System.Net.Browser {

	// This class maps with the browser. 
	// One request is one exchange with the server.

	sealed class BrowserHttpWebRequestInternal : HttpWebRequestCore {
		IntPtr native;
		GCHandle managed;
		long bytes_read;
		bool aborted;
		Dispatcher dispatcher;

		InternalWebRequestStreamWrapper request;
		BrowserHttpWebResponse response;
		HttpWebAsyncResult async_result;
		
		UnmanagedEventHandlerInvoker started;
		UnmanagedEventHandlerInvoker available;
		UnmanagedEventHandlerInvoker finished;

 		public BrowserHttpWebRequestInternal (BrowserHttpWebRequest wreq, Uri uri)
			: base (wreq, uri)
 		{
			started = (sender, event_id, token, calldata, closure) => {
				var x = Events.SafeDispatcher (OnAsyncResponseStarted);
				x (sender, calldata, closure);
			};
			available = (sender, event_id, token, calldata, closure) => {
				var x = Events.SafeDispatcher (OnAsyncDataAvailable);
				x (sender, calldata, closure);
			};
			finished = (sender, event_id, token, calldata, closure) => {
				var x = Events.SafeDispatcher (OnAsyncResponseStopped);
				x (sender, calldata, closure);
			};
			managed = GCHandle.Alloc (this, GCHandleType.Normal);
			aborted = false;
			if (wreq != null) {
				request = wreq.request;
				Headers = wreq.Headers;
			}
			dispatcher = new Dispatcher ();
		}

		~BrowserHttpWebRequestInternal () /* thread-safe: all p/invokes are thread-safe */
		{
			Abort ();

			if (async_result != null)
				async_result.Dispose ();

			if (native != IntPtr.Zero)
				NativeMethods.event_object_unref (native); /* thread-safe */
		}

		public override void Abort ()
		{
			if (native == IntPtr.Zero)
				return;

			aborted = true;
		}

		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			// we're not allowed to reuse an aborted request
			if (aborted)
				throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);

			async_result = new HttpWebAsyncResult (callback, state);

			dispatcher.BeginInvoke (new Action (InitializeNativeRequestSafe), null);

			return async_result;
		}

		void OnAsyncResponseStarted (IntPtr sender, IntPtr calldata, IntPtr closure)
		{
			try {
				IntPtr response = NativeMethods.http_request_get_response (native);
				bytes_read = 0;
				async_result.Response = new BrowserHttpWebResponse (this, response);
			} catch (Exception e) {
				async_result.Exception = e;
			}
		}

		void OnAsyncResponseStopped (IntPtr sender, IntPtr calldata, IntPtr closure)
		{
			try {
				if (progress != null) {
					BrowserHttpWebResponse response = async_result.Response as BrowserHttpWebResponse;
					// report the 100% progress on compressed (or without Content-Length)
					if (response.IsCompressed) {
						long length = response.ContentLength;
						progress (length, length);
					}
				}
			}
			finally {
				async_result.SetComplete ();
			}
		}
		
		void OnAsyncDataAvailable (IntPtr sender, IntPtr calldata, IntPtr closure)
		{
			IntPtr data = IntPtr.Zero;
			uint length = 0;
			BrowserHttpWebResponse response = async_result.Response as BrowserHttpWebResponse;
			try {
				data = NativeMethods.http_request_write_event_args_get_data (calldata);
				length = NativeMethods.http_request_write_event_args_get_count (calldata);
				bytes_read += length;
				if (progress != null) {
					// if Content-Encoding is gzip (or other compressed) then Content-Length cannot be trusted,
					// if present, since it does not (generally) correspond to the uncompressed length
					long content_length = response.ContentLength;
					bool compressed = (response.IsCompressed || (content_length == 0));
					long total_bytes_to_receive = compressed ? -1 : content_length;
					progress (bytes_read, total_bytes_to_receive);
				}
			} catch (Exception e) {
				async_result.Exception = e;
			}

			try {
				if (data != IntPtr.Zero && length != 0)
					response.Write (data, checked ((int) length));
			} catch (Exception e) {
				async_result.Exception = e;
			}
		}

		public override WebResponse EndGetResponse (IAsyncResult asyncResult)
		{
			try {
				if (async_result != asyncResult)
					throw new ArgumentException ("asyncResult");

				if (aborted) {
					NativeMethods.http_request_abort (native);
					throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);
				}

				if (!async_result.IsCompleted)
					async_result.AsyncWaitHandle.WaitOne ();

				if (async_result.HasException) {
					throw async_result.Exception;
				}

				response = async_result.Response as BrowserHttpWebResponse;
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
			native = NativeMethods.deployment_create_http_request (System.Windows.Deployment.Current.native, HttpRequestOptions.CustomHeaders);
			if (native == IntPtr.Zero)
				throw new NotSupportedException ("Failed to create unmanaged HttpRequest object");

			Events.AddHandler (native, EventIds.HttpRequest_StartedEvent, started);
			Events.AddHandler (native, EventIds.HttpRequest_WriteEvent, available);
			Events.AddHandler (native, EventIds.HttpRequest_StoppedEvent, finished);

			var nativeUri = UriHelper.ToNativeUri (RequestUri);
			NativeMethods.http_request_open (native, Method, nativeUri, DownloaderAccessPolicy.NoPolicy);
			NativeMethods.uri_free (nativeUri);

			long request_length = 0;
			byte[] body = null;
			if (request != null) {
				request.Close ();
				body = request.GetData ();
				request_length = body.Length;
			}

			if (request_length > 1) {
				// this header cannot be set directly inside the collection (hence the helper)
				NativeMethods.http_request_set_header (native, "content-length", request_length.ToString (), false);
			}

			foreach (string header in Headers)
				NativeMethods.http_request_set_header (native, header, Headers [header], false);

			if (request_length > 1) {
				NativeMethods.http_request_set_body (native, body, body.Length);
			}
			
			NativeMethods.http_request_send (native);
		}
	}
}

#endif
