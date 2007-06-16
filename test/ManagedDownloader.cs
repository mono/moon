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
using Gtk;
using System.Collections;
using System.Runtime.InteropServices;
using System.Net;

namespace Gtk.Moonlight {

	internal class ManagedDownloader {
		[DllImport ("moon")]
		internal extern static void downloader_write (IntPtr downloader, byte []buf, int offset, int n);

		[DllImport ("moon")]
		internal extern static void downloader_notify_size (IntPtr downloader, long l);
		
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
		WebRequest request = null;
		
		ManagedDownloader (IntPtr native)
		{
			downloader = native;
		}

		void write ()
		{
			Console.WriteLine ("Writing {0} bytes", count);
			lock (buffer){
				downloader_write (downloader, buffer, 0, count);
			}
			auto_reset.Set ();
		}

							    
		void Download ()
		{
			if (request == null)
				throw new Exception ("This Downloader has not been configured yet, call Open");
			
			using (WebResponse r = request.GetResponse ()){
				Application.Invoke (delegate {
					downloader_notify_size (downloader, r.ContentLength);
					auto_reset.Set ();
				});
				auto_reset.WaitOne ();
				
				using (Stream rstream = r.GetResponseStream ()){
					buffer = new byte [16*1024];
					count = 0;

					while (downloading){
						lock (buffer){
							int n = rstream.Read (buffer, 0, buffer.Length);
							Console.WriteLine ("DOWN: Got {0} bytes", n);
							if (n == 0){
								// We are done.
								buffer = null;
								break;
							}
							count = n;
						}
						Application.Invoke (delegate {
							lock (buffer){
								downloader_write (downloader, buffer, 0, count);
							}
							auto_reset.Set ();
						});
						auto_reset.WaitOne ();
					}
				}
			}
		}

		void Start ()
		{
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
			if (verb != "GET"){
				Console.WriteLine ("Do not know what to do with verb {0}", verb);
				return;
			}
			
			try {
				try {
					request = WebRequest.Create (uri);
				} catch (UriFormatException){
					request = WebRequest.Create ("file://" + Path.GetFullPath (uri));
				}
			} catch (Exception e){
				Console.WriteLine ("An error happened with the given url {0}", e);
				// Do something with this
			}
			if (request == null){
				Console.WriteLine ("An error happened with the given url, the result was null");
				return;
			}
		}

		public static IntPtr CreateDownloader (IntPtr native)
		{
			ManagedDownloader m = new ManagedDownloader (native);
			int key = keyid++;
			downloaders [(IntPtr) key] = m;

			Console.WriteLine ("Created: {0}", key);
			return (IntPtr) key;
		}

		public static void DestroyDownloader (IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			if (m == null)
				return;

			downloaders [state] = null;
			
			if (m.async_result != null)
				m.downloading = false;
		}

		public static void Open (string verb, string uri, bool async, IntPtr state)
		{
			ManagedDownloader m = (ManagedDownloader) downloaders [state];
			Console.WriteLine ("Key: {0} -- {1}", state, m == null);
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
