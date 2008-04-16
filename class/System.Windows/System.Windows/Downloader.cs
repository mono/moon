//
// System.Windows.Downloader class
//
// Authors:
//	Sebastien Pouliot <sebastien@ximian.com>
//	Miguel de Icaza   <miguel@ximian.com>
//
// TODO:
//    Register callbacks so we can raise the various events
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
using Mono;
using System.IO;
using System.Runtime.InteropServices;

namespace System.Windows {

	public sealed class Downloader : DependencyObject {

		public static readonly DependencyProperty DownloadProgressProperty =
			DependencyProperty.Lookup (Kind.DOWNLOADER, "DownloadProgress", typeof (double));
		public static readonly DependencyProperty ResponseTextProperty =
			DependencyProperty.Lookup (Kind.DOWNLOADER, "ResponseText", typeof (string));
		public static readonly DependencyProperty StatusProperty =
			DependencyProperty.Lookup (Kind.DOWNLOADER, "Status", typeof (int));
		public static readonly DependencyProperty StatusTextProperty =
			DependencyProperty.Lookup (Kind.DOWNLOADER, "StatusText", typeof (string));
		public static readonly DependencyProperty UriProperty =
			DependencyProperty.Lookup (Kind.DOWNLOADER, "Uri", typeof (string));

		public Downloader () : base (NativeMethods.downloader_new ())
		{
		}
		
		internal Downloader (IntPtr raw) : base (raw)
		{
		}


		public double DownloadProgress {
			get { return (double) GetValue (DownloadProgressProperty); }
			set { SetValue (DownloadProgressProperty, value); }
		}

		public string ResponseText {
			get { return (string) GetValue (ResponseTextProperty); }
			set { SetValue (ResponseTextProperty, value); }
		}

		public int Status {
			get { return (int) GetValue (StatusProperty); }
			set { SetValue (StatusProperty, value); }
		}
		
		public string StatusText {
			get { return (string) GetValue (StatusTextProperty); }
			set { SetValue (StatusTextProperty, value); }
		}

		public Uri Uri {
			get {
				// Uri is not a DependencyObject, we save it as a string
				string uri = (string) GetValue (UriProperty);
				return new Uri (uri);
			}
			set {
				string uri = value.OriginalString;
				SetValue (UriProperty, uri); 
			}
		}

		public void Abort ()
		{
			NativeMethods.downloader_abort (native);
		}

		public string GetResponseText (string PartName)
		{
			long size;
			IntPtr n = NativeMethods.downloader_get_response_text (native, PartName, out size);

			string s = String.Empty;
			try {
				//
				// This returns the contents as string, but the data is a binary blob
				//
				unsafe {
					using (Stream u = new SimpleUnmanagedMemoryStream ((byte *) n, (int) size)){
						using (StreamReader sr = new StreamReader (u)){
							s = sr.ReadToEnd ();
						}
					}
				}
			}
			finally {
				if (n != IntPtr.Zero)
					Helper.FreeHGlobal (n);
			}
			
			return s;
		}

		public void Open (string verb, Uri URI)
		{
			NativeMethods.downloader_open (native, verb, URI.ToString ());
		}

		public void Send ()
		{
			NativeMethods.downloader_send (native);
		}

		static object CompletedEvent = new object();
		static object DownloadProgressChangedEvent = new object ();
		static object DownloadFailedEvent = new object ();

		public event EventHandler Completed {
			add {
				if (events[CompletedEvent] == null)
					Events.AddHandler (this, "Completed", completed_proxy);
				events.AddHandler (CompletedEvent, value);
			}
			remove {
				events.RemoveHandler (CompletedEvent, value);
				if (events[CompletedEvent] == null)
					Events.RemoveHandler (this, "Completed", completed_proxy);
			}
		}

		public event EventHandler DownloadProgressChanged {
			add {
				if (events[DownloadProgressChangedEvent] == null)
					Events.AddHandler (this, "DownloadProgressChanged", progress_changed_proxy);
				events.AddHandler (DownloadProgressChangedEvent, value);
			}
			remove {
				events.RemoveHandler (DownloadProgressChangedEvent, value);
				if (events[DownloadProgressChangedEvent] == null)
					Events.RemoveHandler (this, "DownloadProgressChanged", progress_changed_proxy);
			}
		}

		public event ErrorEventHandler DownloadFailed {
			add {
				if (events[DownloadFailedEvent] == null)
					Events.AddHandler (this, "DownloadFailed", failed_proxy);
				events.AddHandler (DownloadFailedEvent, value);
			}
			remove {
				events.RemoveHandler (DownloadFailedEvent, value);
				if (events[DownloadFailedEvent] == null)
					Events.RemoveHandler (this, "DownloadFailed", failed_proxy);
			}
		}


		static UnmanagedEventHandler completed_proxy = new UnmanagedEventHandler (UnmanagedCompleted);
		static void UnmanagedCompleted (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Downloader dl = (Downloader) Helper.GCHandleFromIntPtr (closure).Target;

			try {
				dl.InvokeEmptyEvent (CompletedEvent);
			} catch (Exception e){
				Console.Error.WriteLine (e);
			}
		}

		static UnmanagedEventHandler progress_changed_proxy = new UnmanagedEventHandler (UnmanagedDownloadProgressChanged);
		static void UnmanagedDownloadProgressChanged (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Downloader dl = (Downloader) Helper.GCHandleFromIntPtr (closure).Target;
			try {
				dl.InvokeEmptyEvent (DownloadProgressChangedEvent);
			} catch (Exception e){
				Console.Error.WriteLine (e);
			}
		}

		void InvokeEmptyEvent (object eventkey)
		{
			EventHandler h = (EventHandler)events[eventkey];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		static UnmanagedEventHandler failed_proxy = new UnmanagedEventHandler (UnmanagedDownloadFailed);
		static void UnmanagedDownloadFailed (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Downloader dl = (Downloader) Helper.GCHandleFromIntPtr (closure).Target;

			string filename = Helper.PtrToStringAuto (calldata);
			ErrorEventArgs eea = new ErrorEventArgs (4001, filename, ErrorType.DownloadError);

			try {
				dl.InvokeDownloadFailed (eea);
			} catch (Exception e){
				Console.Error.WriteLine (e);
			}
		}

		void InvokeDownloadFailed (ErrorEventArgs eea)
		{
			ErrorEventHandler h = (ErrorEventHandler)events[DownloadFailedEvent];
			if (h != null)
				h (this, eea);
		}
		
		internal override Kind GetKind ()
		{
			return Kind.DOWNLOADER;
		}
	}
}
