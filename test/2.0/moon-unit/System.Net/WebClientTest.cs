//
// Unit tests for System.Net.WebClient
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Net;
using System.Reflection;
using System.Security;
using System.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class WebClientTest : SilverlightTest {

		private void CheckDefaults (WebClient wc)
		{
			Assert.IsTrue (wc.AllowReadStreamBuffering, "AllowReadStreamBuffering");
			Assert.IsTrue (wc.BaseAddress.EndsWith ("/moon-unit.xap"), "BaseAddress");
			Assert.AreEqual ("utf-8", wc.Encoding.WebName, "Encoding");
			Assert.AreEqual (0, wc.Headers.Count, "Headers");
			Assert.IsFalse (wc.IsBusy, "IsBusy");
#if SL3
			Assert.IsNull (wc.Credentials, "Credentials");
			Assert.IsNull (wc.ResponseHeaders, "ResponseHeaders");
#endif
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
				Assert.Throws<TargetInvocationException,SecurityException> (delegate {
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
				Assert.Throws<TargetInvocationException,SecurityException> (delegate {
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
// XXX requires a fix in mcs/class - will be committed (and uncommented) later
//				Assert.IsNull (e.UserState, "DownloadProgressChanged-UserState");
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

			Uri uri = new Uri ("http://www.mono-project.com");
			// note: null method == POST
			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenWriteAsync (null, "POST");
			}, "null,POST");
			Assert.Throws<ArgumentNullException> (delegate {
				wc.OpenWriteAsync (null, "POST", String.Empty);
			}, "null,POST,object");
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
				Assert.IsNull (e.Error, "Error"); // weird
				Assert.IsNull (e.UserState, "UserState");
				Assert.IsNotNull (e.Result, "Result"); // weirder
				complete = true;
			};
			Enqueue (() => { wc.OpenWriteAsync (new Uri ("http://www.mono-project.com"), null); });
			EnqueueConditional (() => complete);
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
				Assert.IsNull (e.Error, "Error"); // weird
				Assert.AreEqual (String.Empty, e.UserState, "UserState");
				Assert.IsNotNull (e.Result, "Result"); // weirder
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
	}
}

