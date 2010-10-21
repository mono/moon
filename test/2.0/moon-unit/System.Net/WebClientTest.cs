//
// Unit tests for System.Net.WebClient
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009-2010 Novell, Inc (http://www.novell.com)
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
using System.IO;
using System.Net;
using System.Reflection;
using System.Security;
using System.Threading;
using System.Windows;
using System.Windows.Browser;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class WebClientTest : SilverlightTest {

		static internal Uri IndexHtml;
		static internal Uri PostAspx;
		static internal Uri TimecodeLongWmv;
		static bool RunningFromHttp;

		static WebClientTest ()
		{
			Uri uri = HtmlPage.Document.DocumentUri;
			RunningFromHttp = (uri.Scheme == "http");
			IndexHtml = new Uri (uri, "../index4.html");
			PostAspx = new Uri (uri, "../POST.aspx");
			TimecodeLongWmv = new Uri (uri, "timecode-long.wmv");
		}

		private void CheckDefaults (WebClient wc)
		{
			Assert.IsTrue (wc.AllowReadStreamBuffering, "AllowReadStreamBuffering");
			Assert.IsTrue (wc.AllowWriteStreamBuffering, "AllowWriteStreamBuffering");
			string ba = wc.BaseAddress;
			Assert.IsTrue (ba.EndsWith (".xap"), "BaseAddress (.xap)");
			int last_slash = ba.LastIndexOf ('/');
			Assert.IsTrue (ba.Substring (last_slash, ba.Length - last_slash - 4).StartsWith ("/moon-unit"), "BaseAddress: " + ba);
			Assert.AreEqual ("utf-8", wc.Encoding.WebName, "Encoding");
			Assert.AreEqual (0, wc.Headers.Count, "Headers");
			Assert.IsFalse (wc.IsBusy, "IsBusy");
			Assert.IsNull (wc.Credentials, "Credentials");
			Assert.IsNull (wc.ResponseHeaders, "ResponseHeaders");
			Assert.IsTrue (wc.UseDefaultCredentials, "UseDefaultCredentials");
		}

		[TestMethod]
		public void Defaults ()
		{
			WebClient wc = new WebClient ();
			CheckDefaults (wc);
			// does nothing if no async operation is in progress
			wc.CancelAsync ();
		}

		[TestMethod]
		public void BaseAddress ()
		{
			WebClient wc = new WebClient ();
			Uri uri = new Uri (wc.BaseAddress);
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsTrue (wc.BaseAddress.EndsWith (".xap"), ".xap");

			wc.BaseAddress = "http://mono-project.com";
			// note the suffixed '/'
			Assert.AreEqual ("http://mono-project.com/", wc.BaseAddress, "BaseAddress-1");

			wc.BaseAddress = null;
			Assert.AreEqual (String.Empty, wc.BaseAddress, "BaseAddress-null");

			wc.BaseAddress = String.Empty;
			Assert.AreEqual (String.Empty, wc.BaseAddress, "BaseAddress-empty");

			wc.BaseAddress = "http://mono-project.com/oops there are spaces";
			Assert.AreEqual ("http://mono-project.com/oops there are spaces", wc.BaseAddress, "BaseAddress-4");

			wc.BaseAddress = uri.AbsoluteUri;
			Assert.AreEqual (Uri.UnescapeDataString (uri.AbsoluteUri), wc.BaseAddress, "BaseAddress-2");

			Assert.Throws<ArgumentException> (delegate {
				wc.BaseAddress = "/non-absolute-uri";
			}, "non absolute uri");

			Assert.Throws<ArgumentException> (delegate {
				wc.BaseAddress = "!";
			}, "invalid uri");
		}

		[TestMethod]
		public void Headers ()
		{
			WebClient wc = new WebClient ();
			wc.Headers = null;
			// not nullable ? or re-created ?
			Assert.IsNotNull (wc.Headers, "Headers");

			wc.Headers ["moon"] = "light";
			Assert.AreEqual (1, wc.Headers.Count, "Count-1");

			wc.Headers = null;
			Assert.AreEqual (0, wc.Headers.Count, "Count-2");
			// re-created!
		}

		[TestMethod]
		public void DownloadStringAsync_Null ()
		{
			WebClient wc = new WebClient ();
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
				Assert.Fail ("not called");
			};

			Assert.Throws<ArgumentNullException> (delegate {
				wc.DownloadStringAsync (null);
			}, "null");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.DownloadStringAsync (null, String.Empty);
			}, "null,object");
		}

		[TestMethod]
		[Asynchronous]
		public void DownloadStringAsync_Relative ()
		{
			WebClient wc = new WebClient ();
			// relative works only if the BaseAddress can be used to make an absolute http[s] Uri
			// e.g. it won't work if the original Uri is a file:// so it won't work if this XAP is loaded from a server
			bool complete = !wc.BaseAddress.StartsWith ("file:///");
			if (!complete) {
				wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
					CheckDefaults (wc);
					Assert.IsFalse (e.Cancelled, "Cancelled");
					Assert.IsTrue (e.Error is WebException, "Error");
					Assert.IsTrue (e.Error.InnerException is NotSupportedException, "Error.InnerException");
					Assert.IsNull (e.UserState, "UserState");
					Assert.Throws<TargetInvocationException, WebException> (delegate {
						Assert.IsNotNull (e.Result, "Result");
					}, "Result");
					complete = true;
				};
				Enqueue (() => { wc.DownloadStringAsync (new Uri ("/myfile", UriKind.Relative)); });
			}
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void DownloadStringAsync ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.IsNull (e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.DownloadStringAsync (new Uri ("http://www.mono-project.com")); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void DownloadStringAsync_UserToken ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.AreEqual (String.Empty, e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.DownloadStringAsync (new Uri ("http://www.mono-project.com"), String.Empty); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("original link is now redirected - need another test case")]
		public void DownloadStringAsync_Gzip ()
		{
			int tid = Thread.CurrentThread.ManagedThreadId;
			int progress = 0;
			bool start = false, end = false, complete = false;
			long received = 0;
			WebClient wc = new WebClient ();
			wc.DownloadProgressChanged += delegate (object sender, DownloadProgressChangedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				// if the "true" (uncompressed) ContentLength is not available then this 
				// event seems called two times - at start time (with data) and at end time (all data)
				if (e.TotalBytesToReceive == -1) {
					start = true;
					Assert.IsTrue (e.BytesReceived >= received, "Start-BytesReceived");
					Assert.AreEqual (0, e.ProgressPercentage, "Start-ProgressPercentage");
					received = e.BytesReceived;
				} else {
					Assert.AreEqual (100, e.ProgressPercentage, "End-ProgressPercentage");
					Assert.AreEqual (e.BytesReceived, e.TotalBytesToReceive, "End-TotalBytesToReceive");
					received = e.TotalBytesToReceive;
					end = true;
				}
				Assert.IsNull (e.UserState, "DownloadProgressChanged-UserState");
				progress++;
			};
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
				complete = true;
				// DownloadProgressChanged is called at least twice (0% and 100%)
				Assert.IsTrue (progress >= 2, "DownloadProgressChanged called (at least) twice: " + progress.ToString ());
				Assert.AreEqual (received, e.Result.Length, "Result");
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "DownloadProgressChanged-ManagedThreadId");
				Assert.IsNull (e.UserState, "DownloadProgressChanged-UserState");
			};
			Enqueue (() => wc.DownloadStringAsync (new Uri ("http://www.ctvolympics.ca/datasources/videoplayer/settings.xml?ignore=db1504e5-ab41-41ed-a8ed-14856e15d283")));
			EnqueueConditional (() => start && end && complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void OpenReadAsync_Null ()
		{
			WebClient wc = new WebClient ();
			wc.OpenReadCompleted += delegate (object sender, OpenReadCompletedEventArgs e) {
				Assert.Fail ("not called");
			};

			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenReadAsync (null);
			}, "null");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenReadAsync (null, String.Empty);
			}, "null,object");
		}

		[TestMethod]
		[Asynchronous]
		public void OpenReadAsync ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.OpenReadCompleted += delegate (object sender, OpenReadCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.IsNull (e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.OpenReadAsync (new Uri ("http://www.mono-project.com")); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenReadAsync_UserToken ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.OpenReadCompleted += delegate (object sender, OpenReadCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.AreEqual (String.Empty, e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.OpenReadAsync (new Uri ("http://www.mono-project.com"), String.Empty); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void OpenWriteAsync_Null ()
		{
			WebClient wc = new WebClient ();
			wc.OpenWriteCompleted += delegate (object sender, OpenWriteCompletedEventArgs e) {
				Assert.Fail ("not called");
			};

			// note: null method == POST
			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenWriteAsync (null, "POST");
			}, "null,POST");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenWriteAsync (null, "POST", String.Empty);
			}, "null,POST,object");
		}

		void CheckStream (Stream s, bool closed, string msg)
		{
			Assert.AreEqual (!closed, s.CanRead, msg + ".CanRead");
			Assert.AreEqual (!closed, s.CanSeek, msg + ".CanWrite");
			Assert.AreEqual (false, s.CanTimeout, msg + ".CanWrite");
			Assert.AreEqual (!closed, s.CanWrite, msg + ".CanWrite");

			if (closed) {
				Assert.Throws<ObjectDisposedException> (delegate {
					Assert.AreEqual (0, s.Position);
				}, msg + ".Position");
				Assert.Throws<ObjectDisposedException> (delegate {
					Assert.AreEqual (0, s.Length);
				}, msg + ".Length");
			} else {
				Assert.AreEqual (0, s.Position, msg + ".Position");
				Assert.AreEqual (0, s.Length, msg + ".Length");
			}

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (-1, s.ReadTimeout);
			}, msg + ".ReadTimeout");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.AreEqual (-1, s.WriteTimeout);
			}, msg + ".WriteTimeout");
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.OpenWriteCompleted += delegate (object sender, OpenWriteCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsNull (e.Error, "Error");
				Assert.IsNull (e.UserState, "UserState");

				CheckStream (e.Result, false, "Result.Before");
				e.Result.Close ();
				CheckStream (e.Result, true, "Result.After");
				complete = true;
			};
			Enqueue (() => { wc.OpenWriteAsync (new Uri ("http://www.mono-project.com"), null); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync_CloseStream ()
		{
			WebClient wc = new WebClient ();
			bool complete_open = false;
			bool complete_close = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.OpenWriteCompleted += delegate (object sender, OpenWriteCompletedEventArgs e) {
				CheckStream (e.Result, false, "Result.Before");
				e.Result.Close ();
				CheckStream (e.Result, true, "Result.After");
				complete_open = true;
			};
			wc.WriteStreamClosed += delegate (object sender, WriteStreamClosedEventArgs e) {
				Assert.AreSame (wc, sender, "Sender");
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsTrue (e.Error is SecurityException, "Error");
				complete_close = true;
			};
			Enqueue (() => { wc.OpenWriteAsync (new Uri ("http://www.mono-project.com"), null); });
			EnqueueConditional (() => complete_open && complete_close);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync_BadMethod ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			wc.OpenWriteCompleted += delegate (object sender, OpenWriteCompletedEventArgs e) {
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is WebException, "Error");
				Assert.IsTrue (e.Error.InnerException is NotSupportedException, "Error.InnerException");
				Assert.IsNull (e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, WebException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.OpenWriteAsync (new Uri ("http://www.mono-project.com"), "ZAP"); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync_UserToken ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.OpenWriteCompleted += delegate (object sender, OpenWriteCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsNull (e.Error, "Error");
				Assert.AreEqual (String.Empty, e.UserState, "UserState");

				CheckStream (e.Result, false, "Result.Before");
				e.Result.Close ();
				CheckStream (e.Result, true, "Result.After");
				complete = true;
			};
			Enqueue (() => { wc.OpenWriteAsync (new Uri ("http://www.mono-project.com"), null, String.Empty); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void UploadStringAsync_Null ()
		{
			WebClient wc = new WebClient ();
			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				Assert.Fail ("not called");
			};

			Uri uri = new Uri ("http://www.mono-project.com");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.UploadStringAsync (null, "data");
			}, "null,data");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.UploadStringAsync (null, "POST", "data");
			}, "null,POST,data");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.UploadStringAsync (uri, null);
			}, "uri,null");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.UploadStringAsync (uri, "POST", null);
			}, "uri,POST,null");
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.IsNull (e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.UploadStringAsync (new Uri ("http://www.mono-project.com"), "data"); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_BadMethod ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is WebException, "Error");
				Assert.IsTrue (e.Error.InnerException is NotSupportedException, "Error.InnerException");
				Assert.IsNull (e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, WebException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.UploadStringAsync (new Uri ("http://www.mono-project.com"), "ZAP", "data"); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_UserToken ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
				CheckDefaults (wc);
				Assert.IsFalse (e.Cancelled, "Cancelled");
				Assert.IsTrue (e.Error is SecurityException, "Error");
				Assert.AreEqual (String.Empty, e.UserState, "UserState");
				Assert.Throws<TargetInvocationException, SecurityException> (delegate {
					Assert.IsNotNull (e.Result, "Result");
				}, "Result");
				complete = true;
			};
			Enqueue (() => { wc.UploadStringAsync (new Uri ("http://www.mono-project.com"), "POST", "data", String.Empty); });
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_ContentType_POST ()
		{
			WebClient wc = new WebClient ();
			UploadStringCompletedEventArgs upload_args = null;

			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				upload_args = e;
			};
			wc.Headers ["Content-Type"] = "foobar";
			Enqueue (() => { wc.UploadStringAsync (PostAspx, "POST", "data", String.Empty); });
			EnqueueConditional (() => upload_args != null);
			Enqueue (delegate () {
				/* We *are* allowed to set Content-Type for POST requests */
				Assert.IsNull (upload_args.Error, "Error");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_ContentType_GET ()
		{
			WebClient wc = new WebClient ();
			UploadStringCompletedEventArgs upload_args = null;

			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadStringCompleted += delegate (object sender, UploadStringCompletedEventArgs e) {
				upload_args = e;
			};
			wc.Headers ["Content-Type"] = "foobar";
			Enqueue (() => { wc.UploadStringAsync (PostAspx, "GET", "data", String.Empty); });
			EnqueueConditional (() => upload_args != null);
			Enqueue (delegate () {
				/* We're not allowed to set Content-Type for GET requests */
				Assert.IsTrue (upload_args.Error is WebException, "Error");
				Assert.IsTrue (upload_args.Error.InnerException is ProtocolViolationException, "Error.InnerException");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		public void Headers_NoValidation ()
		{
			// every value can be added to a collection that is not associated with a WebRequest
			WebHeaderCollection whc = new WebClient ().Headers;
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				HttpRequestHeader hrh = (HttpRequestHeader) i;
				string header = WebHeaderCollectionTest.HttpRequestHeaderToString (hrh);
				string s = i.ToString ();

				whc [hrh] = s;
				Assert.AreEqual (s, whc [hrh], "HttpRequestHeader." + header);
				whc [header] = s;
				Assert.AreEqual (s, whc [header], header);
			}
		}

		[TestMethod]
		[Asynchronous]
		public void DownloadStringAsync_MainThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the DownloadStringAsync events are executed on the main thread when the request was done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in DownloadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.DownloadStringCompleted += new DownloadStringCompletedEventHandler (delegate (object sender, DownloadStringCompletedEventArgs dscea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in DownloadStringCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.DownloadStringAsync (IndexHtml);
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void DownloadStringAsync_UserThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool progress = false;
			bool done = false;

			/* Check that the DownloadStringAsync events are executed on a threadpool thread when the request was done on a user thread */

			WebClient wc = new WebClient ();
			wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
				try {
					progress = true;
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.DownloadStringCompleted += new DownloadStringCompletedEventHandler (delegate (object sender, DownloadStringCompletedEventArgs dscea) {
				try {
					// ResponseHeaders not available from browser stack
					if (RunningFromHttp) {
						// if the response is successful then...
						Assert.IsFalse (dscea.Cancelled, "Cancelled");
						Assert.IsNull (dscea.Error, "Error");
						Assert.Throws<NotImplementedException> (delegate {
							Assert.IsNotNull (wc.ResponseHeaders);
						}, "ResponseHeaders");
					} else {
						// unsuccessful (WebClient cannot be used from file:///)
						Assert.IsFalse (dscea.Cancelled, "Cancelled");
						Assert.IsTrue (dscea.Error is WebException, "Error");
						Assert.IsTrue (dscea.Error.InnerException is NotSupportedException, "Error.InnerException");
						Assert.IsNull (wc.ResponseHeaders, "ResponseHeaders");
					}
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadStringCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			Thread t = new Thread (delegate () {
				wc.DownloadStringAsync (IndexHtml);
			});
			t.Start ();
			EnqueueConditional (() => done);
			Enqueue (() => {
				// DownloadProgressChanged is only called if running from http:// (not file://)
				Assert.AreEqual (RunningFromHttp, progress, "DownloadProgressChanged");
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenReadAsync_MainThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenReadAsync events are executed on the main thread when the request was done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in DownloadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenReadCompleted += new OpenReadCompletedEventHandler (delegate (object sender, OpenReadCompletedEventArgs orcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in OpenReadCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.OpenReadAsync (IndexHtml);
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenReadAsync_UserThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenReadAsync events are executed on a threadpool thread when the request was not done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenReadCompleted += new OpenReadCompletedEventHandler (delegate (object sender, OpenReadCompletedEventArgs orcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in OpenReadCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			Thread t = new Thread (delegate () {
				wc.OpenReadAsync (IndexHtml);
			});
			t.Start ();
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenReadAsync_ThreadPool ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenReadAsync events are executed on a threadpool thread when the request was not done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenReadCompleted += new OpenReadCompletedEventHandler (delegate (object sender, OpenReadCompletedEventArgs orcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in OpenReadCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});

			Enqueue (() => {
				wc.OpenReadAsync (IndexHtml);
			});
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync_MainThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenWriteAsync events are executed on the main thread when the request was done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadProgressChanged += new UploadProgressChangedEventHandler (delegate (object sender, UploadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in UploadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenWriteCompleted += new OpenWriteCompletedEventHandler (delegate (object sender, OpenWriteCompletedEventArgs orcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in OpenWriteCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.WriteStreamClosed += new WriteStreamClosedEventHandler (delegate (object sender, WriteStreamClosedEventArgs wscea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess ());
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in WriteStreamClosed");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenWriteAsync (IndexHtml);
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenWriteAsync_UserThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenWriteAsync events are executed on a threadpool thread when the request was not done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadProgressChanged += new UploadProgressChangedEventHandler (delegate (object sender, UploadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in UploadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.OpenWriteCompleted += new OpenWriteCompletedEventHandler (delegate (object sender, OpenWriteCompletedEventArgs orcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in OpenWriteCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.WriteStreamClosed += new WriteStreamClosedEventHandler (delegate (object sender, WriteStreamClosedEventArgs wscea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in WriteStreamClosed");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			Thread t = new Thread (delegate () {
				wc.OpenWriteAsync (IndexHtml);
			});
			t.Start ();
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_MainThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the UploadStringAsync events are executed on the main thread when the request was done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadProgressChanged += new UploadProgressChangedEventHandler (delegate (object sender, UploadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess (), "CheckAccess UploadProgressChanged");
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in UploadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.UploadStringCompleted += new UploadStringCompletedEventHandler (delegate (object sender, UploadStringCompletedEventArgs upcea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess (), "CheckAccess UploadStringCompleted");
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in UploadStringCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.WriteStreamClosed += new WriteStreamClosedEventHandler (delegate (object sender, WriteStreamClosedEventArgs wscea) {
				try {
					Assert.IsTrue (TestPanel.CheckAccess (), "CheckAccess WriteStreamClosed");
					Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Equal thread ids in WriteStreamClosed");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.UploadStringAsync (IndexHtml, "dummy data");
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UploadStringAsync_UserThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenWriteAsync events are executed on a threadpool thread when the request was not done on the main thread */

			WebClient wc = new WebClient ();
			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			wc.UploadProgressChanged += new UploadProgressChangedEventHandler (delegate (object sender, UploadProgressChangedEventArgs dpcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in UploadProgressChanged");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			wc.UploadStringCompleted += new UploadStringCompletedEventHandler (delegate (object sender, UploadStringCompletedEventArgs upcea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in UploadStringCompleted");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
				finally {
					done = true;
				}
			});
			wc.WriteStreamClosed += new WriteStreamClosedEventHandler (delegate (object sender, WriteStreamClosedEventArgs wscea) {
				try {
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in WriteStreamClosed");
				}
				catch (AssertFailedException e) {
					afe = e;
				}
			});
			Thread t = new Thread (delegate () {
				wc.UploadStringAsync (IndexHtml, "dummy data");
			});
			t.Start ();
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void WebClient_ThreadPool ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			Enqueue (() => {
				WebClient wc = new WebClient ();
				wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
					try {
						Assert.IsTrue (TestPanel.CheckAccess ());
						Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadProgressChanged");
					}
					catch (AssertFailedException e) {
						afe = e;
					}
				});
				wc.OpenReadCompleted += new OpenReadCompletedEventHandler (delegate (object sender, OpenReadCompletedEventArgs orcea) {
					try {
						Assert.IsTrue (TestPanel.CheckAccess ());
						Assert.AreEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in OpenReadCompleted");
					}
					catch (AssertFailedException e) {
						afe = e;
					}
					finally {
						done = true;
					}
				});
				wc.OpenReadAsync (IndexHtml);
			});
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void WebClient_UserThread ()
		{
			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			AssertFailedException afe = null;
			bool done = false;

			/* Check that the OpenReadAsync events are executed on a threadpool thread when the request was not done on the main thread */

			if (!RunningFromHttp) {
				EnqueueTestComplete ();
				return;
			}

			Thread t = new Thread (delegate () {
				WebClient wc = new WebClient ();
				wc.DownloadProgressChanged += new DownloadProgressChangedEventHandler (delegate (object sender, DownloadProgressChangedEventArgs dpcea) {
					try {
						Assert.IsFalse (TestPanel.CheckAccess ());
						Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in DownloadProgressChanged");
					}
					catch (AssertFailedException e) {
						afe = e;
					}
				});
				wc.OpenReadCompleted += new OpenReadCompletedEventHandler (delegate (object sender, OpenReadCompletedEventArgs orcea) {
					try {
						Assert.IsFalse (TestPanel.CheckAccess ());
						Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in OpenReadCompleted");
					}
					catch (AssertFailedException e) {
						afe = e;
					}
					finally {
						done = true;
					}
				});
				wc.OpenReadAsync (IndexHtml);
			});
			t.Start ();
			EnqueueConditional (() => done);
			Enqueue (() => {
				if (afe != null)
					throw afe;
			});
			EnqueueTestComplete ();
		}
	}
}

