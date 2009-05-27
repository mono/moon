//
// ManagedDownloader.cs: The managed implementation that uses System.Net
// to provide services to the Downloader class.
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
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
using System;
using System.Threading;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Net;
using Mono;

namespace Moonlight.Gtk {

	internal class ManagedDownloader {
		static int keyid;
		static Hashtable downloaders = new Hashtable ();

		// The pointer to the agclr downloader object
		IntPtr downloader;

		// Where we track the download
		delegate void DownloadDelegate ();
		
		int count;
		byte [] buffer;
		AutoResetEvent auto_reset;
		DownloadDelegate down;
		IAsyncResult async_result;
		volatile bool downloading = true;

		//
		// Either request is non-null, or fname is non-null.
		// This is merely an optimization: for fname, instead of going
		// through FileWebRequest threads and the gtk main loop, we just
		// emit the ready signal as soon as we can.
		//
		WebRequest request = null;
		string fname;
		
		Mono.TickCallHandler tick_call;

		ManagedDownloader (IntPtr native)
		{
			downloader = native;
		}

		void Download ()
		{
			// we can't call event_object_get_surface (or other stuff) if the download has been aborted
			if (!downloading)
				return;

			IntPtr time_manager = NativeMethods.surface_get_time_manager (NativeMethods.event_object_get_surface (downloader));
			// Special case: local file, just notify that we are done
			
			if (fname != null) {
				if (File.Exists (fname)) {
					NativeMethods.time_manager_add_tick_call (time_manager, tick_call = delegate (IntPtr data) {
						if (!downloading)
							return;
						tick_call = null;
						NativeMethods.downloader_notify_finished (downloader, fname);
					}, IntPtr.Zero);
				}
				else {
					NativeMethods.time_manager_add_tick_call (time_manager, tick_call = delegate (IntPtr data) {
						if (!downloading)
							return;
						tick_call = null;
						NativeMethods.downloader_notify_failed (downloader, String.Format ("File `{0}' not found", fname));
					}, IntPtr.Zero);
				}
				return;
			}


			if (request == null)
				throw new Exception ("This Downloader has not been configured yet, call Open");

			string path;
			int offset;
			
			try {
				using (WebResponse r = request.GetResponse ()){
					NativeMethods.time_manager_add_tick_call (time_manager, tick_call = delegate (IntPtr data) {
						if (!downloading)
							return;
						NativeMethods.downloader_notify_size (downloader, r.ContentLength);
						tick_call = null;
						auto_reset.Set ();
					}, IntPtr.Zero);
					auto_reset.WaitOne ();

					using (Stream rstream = r.GetResponseStream ()){
						buffer = new byte [32*1024];
						count = 0;
						offset = 0;
						
						while (downloading){
							count = rstream.Read (buffer, 0, buffer.Length);
							if (count == 0){
								buffer = null;
								break;
							}

							NativeMethods.time_manager_add_tick_call (time_manager, tick_call = delegate (IntPtr data) {
								if (!downloading)
									return;
								tick_call = null;
								unsafe {
									fixed (byte *buf = &buffer[0]) {
										NativeMethods.downloader_write (downloader, (IntPtr)buf, offset, count);
									}
								}
								auto_reset.Set ();
							}, IntPtr.Zero);
							auto_reset.WaitOne ();
							offset += count;
						}
					}
					// We are done
					NativeMethods.time_manager_add_tick_call (time_manager, tick_call = delegate (IntPtr data) {
						if (!downloading)
							return;
						tick_call = null;
						NativeMethods.downloader_notify_finished (downloader, null);
					}, IntPtr.Zero);
				}
			} catch (Exception e){
				Console.Error.WriteLine ("There was an error {0}", e);
				NativeMethods.downloader_notify_failed (downloader, e.Message);
			}
		}

		//
		// Initiates the download
		//
		void Start ()
		{
			//
			// Need to use a separate thread
			//
			auto_reset = new AutoResetEvent (false);
			down = new DownloadDelegate (Download);
			downloading = true;
			async_result = down.BeginInvoke (new AsyncCallback (DownloadDone), null);
		}
		
		public static void Send (IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

			m.Start ();
		}

		void DownloadDone (IAsyncResult async_result)
		{
			down.EndInvoke (async_result);
			this.async_result = null;
		}
		
		void Open (string verb, string uri)
		{
			if (verb != "GET" && verb.ToUpper () != "GET"){
				Console.WriteLine ("Do not know what to do with verb {0}", verb);
				return;
			}

			try {
				if (uri.StartsWith ("file://"))
					fname = uri.Substring (7);
				if (uri.StartsWith ("/"))
					fname = uri;
				else {
					try {
						request = WebRequest.Create (uri);
					} catch (UriFormatException){
						fname = Path.GetFullPath (uri);
					}
				}
			} catch (Exception e){
				Console.WriteLine ("An error happened with the given url {0}", e);
				// Do something with this
			}
			if (request == null && fname == null){
				Console.WriteLine ("An error happened with the given url, the result was null");
				return;
			}
		}

		public static IntPtr CreateDownloader (IntPtr native)
		{
			ManagedDownloader m = new ManagedDownloader (native);
			int key = keyid++;
			downloaders [(IntPtr) key] = m;

			return (IntPtr) key;
		}

		public static void DestroyDownloader (IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

			downloaders [state] = null;
			
			m.downloading = false;
		}

		public static void Open (IntPtr state, string verb, string uri, bool custom_header_support, bool disable_cache)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

			m.Open (verb, uri);
		}

		public static void Abort (IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

		        m.downloading = false;
		}
		
		public static void Header (IntPtr state, string header, string value)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;
			
			m.request.Headers [header] = value;
		}
		
		public static void Body (IntPtr state, IntPtr body, int length)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

			Stream stream = m.request.GetRequestStream ();
			
			byte[] buffer = new byte[length];

			Marshal.Copy (body, buffer, 0, length);
			stream.Write (buffer, 0, length);
		}
		
		public static IntPtr CreateWebRequest (string method, string uri, IntPtr context)
		{
			throw new NotImplementedException ();
		}

		public static string GetResponseText (string part, IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return null;

			// Ask toshok what he wants here.
			return null;
		}
	}
}
