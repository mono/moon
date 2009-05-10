//
// System.Windows.Browser.Net.BrowserHttpWebRequest class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007,2009 Novell, Inc (http://www.novell.com)
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
using System.Diagnostics;
using System.Threading;
using System.Collections;
using System.Globalization;
using System.IO;
using System.Net;
using System.Windows.Interop;
using System.Runtime.InteropServices;

using Mono;
using Mono.Xaml;

namespace System.Windows.Browser.Net
{
	class BrowserHttpWebRequest : HttpWebRequest
	{
		IntPtr native;
		GCHandle managed;
		IntPtr downloader;
		Uri uri;
		long bytes_read;
		bool aborted;
		bool allow_read_buffering;
		string method = "GET";
		WebHeaderCollection headers = new WebHeaderCollection (true);
		BrowserHttpWebRequestStream request;
		BrowserHttpWebResponse response;
		BrowserHttpWebAsyncResult async_result;
		ManualResetEvent wait_handle = new ManualResetEvent (false);
		
		DownloaderResponseStartedDelegate started;
		DownloaderResponseAvailableDelegate available;
		DownloaderResponseFinishedDelegate finished;
 		
		//NOTE: This field name needs to stay in sync with WebRequest_2_1.cs in Systme.Net
 		Delegate progress_delegate;

 		public BrowserHttpWebRequest (Uri uri)
 		{
			started = new DownloaderResponseStartedDelegate (OnAsyncResponseStartedSafe);
			available = new DownloaderResponseAvailableDelegate (OnAsyncDataAvailableSafe);
			finished = new DownloaderResponseFinishedDelegate (OnAsyncResponseFinishedSafe);
 			this.uri = uri;
			managed = GCHandle.Alloc (this, GCHandleType.Normal);
			aborted = false;
			allow_read_buffering = true;
		}

		~BrowserHttpWebRequest ()
		{
			Abort ();

			if (async_result != null)
				async_result.Dispose ();

			if (native == IntPtr.Zero)
				return;

			NativeMethods.downloader_request_free (native);
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
			if (aborted)
				return;

			NativeMethods.downloader_request_abort (native);
			aborted = true;
			managed.Free ();
		}

		static bool IsSameRangeKind (string range, string rangeSpecifier)
		{
			return range.ToLower (CultureInfo.InvariantCulture).StartsWith (
				rangeSpecifier.ToLower (CultureInfo.InvariantCulture));
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
			async_result = new BrowserHttpWebAsyncResult (callback, state);
			
			if (NativeMethods.surface_in_main_thread ()) {
				InitializeNativeRequest (IntPtr.Zero);
			} else {
				TickCallHandler tch = new TickCallHandler (InitializeNativeRequestSafe);

				NativeMethods.time_manager_add_tick_call (NativeMethods.surface_get_time_manager (NativeMethods.plugin_instance_get_surface (PluginHost.Handle)), tch, IntPtr.Zero);

				wait_handle.WaitOne ();

				GC.KeepAlive (tch);
			}

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
		
		static uint OnAsyncResponseStarted (IntPtr native, IntPtr context)
		{
			GCHandle handle = GCHandle.FromIntPtr (context);
			BrowserHttpWebRequest obj = (BrowserHttpWebRequest) handle.Target;
			
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
			GCHandle handle = GCHandle.FromIntPtr (context);
			BrowserHttpWebRequest obj = (BrowserHttpWebRequest) handle.Target;
			
			try {
				obj.async_result.SetComplete ();
			} catch (Exception e) {
				obj.async_result.Exception = e;
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
			GCHandle handle = GCHandle.FromIntPtr (context);
			BrowserHttpWebRequest obj = (BrowserHttpWebRequest) handle.Target;
			
			try {
				obj.bytes_read += length;
				if (obj.progress_delegate != null)
					obj.progress_delegate.DynamicInvoke (new object[] { obj.bytes_read, obj.async_result.Response.ContentLength, obj.async_result.AsyncState});
			} catch {}

			try {
				obj.async_result.Response.Write (data, (int) length);
			} catch (Exception e) {
				obj.async_result.Exception = e;
			}
			return 0;
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			if (request == null) {
				request = new BrowserHttpWebRequestStream ();
				request.WriteByte ((byte) '\n');
			}
			return request;
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

			managed.Free ();
			
			return response;
		}

		static Uri GetBaseUri ()
		{
			//FIXME: there's most probably a better way to do this.

			string uri = NativeMethods.plugin_instance_get_source_location (PluginHost.Handle);
			return new Uri (uri.Substring (0, uri.LastIndexOf ("/") + 1));
		}

		static Uri GetAbsoluteUri (Uri uri)
		{
			return new Uri (GetBaseUri (), uri);
		}

		void InitializeNativeRequestSafe (IntPtr context)
		{
			try {
				InitializeNativeRequest (context);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BrowserHttpWebRequest.InitializeNativeRequestSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		void InitializeNativeRequest (IntPtr context)
		{
			if (native != IntPtr.Zero)
				return;

			Uri request_uri = uri.IsAbsoluteUri ? uri : GetAbsoluteUri (uri);

			downloader = NativeMethods.surface_create_downloader (XamlLoader.SurfaceInDomain);
			if (downloader == null)
				throw new NotSupportedException ("Failed to create unmanaged downloader");
			native = NativeMethods.downloader_create_web_request (downloader, method, request_uri.AbsoluteUri);
			if (native == IntPtr.Zero)
				throw new NotSupportedException ("Failed to create unmanaged WebHttpRequest object.  unsupported browser.");

			if (request != null && request.Length > 1) {
				NativeMethods.downloader_request_set_http_header (native, "Content-Length", (request.Length - 1).ToString ());
			}
			
			foreach (string header in headers.AllKeys)
				NativeMethods.downloader_request_set_http_header (native, header, headers [header]);

			if (request != null && request.Length > 1) {
				byte [] body = request.stream.ToArray ();
				NativeMethods.downloader_request_set_body (native, body, body.Length);
			}
			
			NativeMethods.downloader_request_get_response (native, started, available, finished, GCHandle.ToIntPtr (managed));

			wait_handle.Set ();
		}

		[MonoTODO ("value is unused")]
		public override bool AllowReadStreamBuffering {
			get { return allow_read_buffering; }
			set { allow_read_buffering = value; }
		}

		public override string ContentType {
			get { return headers [HttpRequestHeader.ContentType]; }
			set { headers [HttpRequestHeader.ContentType] = value; }
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

		public override WebHeaderCollection Headers {
			get { return headers; }
			set {
				// note: this is not a field assignment but a copy (see unit tests)
				// make sure everything we're supplied is valid...
				string[] keys = value.AllKeys;
				foreach (string header in keys) {
					// anything bad will throw
					WebHeaderCollection.ValidateHeader (header);
				}
				// ... before making those values our own
				headers.headers.Clear ();
				foreach (string header in keys) {
					headers [header] = value [header];
				}
			}
		}

		public override string Method {
			get { return method; }
			set {
				if (String.IsNullOrEmpty (value))
					throw new NotSupportedException ("Method");

				switch (value.ToUpperInvariant ()) {
				case "GET":
				case "POST":
					method = value;
					break;
				default:
					throw new NotSupportedException ("Method " + value);
				}
			}
		}

		public override Uri RequestUri {
			get { return uri; }
		}
	}
}

#endif
