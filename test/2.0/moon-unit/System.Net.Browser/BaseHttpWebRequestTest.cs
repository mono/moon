//
// Common unit tests for System.Net.Browser.[Browser|Client]HttpWebRequest
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Browser;
using System.Security;
using System.Threading;
using System.Windows;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Browser {

	// Here are the test cases that are identical between the Browser and the Client HTTP stack provided by Silverlight 3
	// Test cases where results differs are located in [Browser|Client]HttpWebRequestTest - which both inherits this type
	public abstract class BaseHttpWebRequestTest : SilverlightTest  {

		public static Uri localhost = new Uri ("http://localhost");

		protected abstract WebRequest GetWebRequest (Uri uri);

		[TestMethod]
		public void Buffering_Get ()
		{
			HttpWebRequest wr = (HttpWebRequest) GetWebRequest (new Uri ("http://localhost"));
			Assert.IsTrue (wr.AllowReadStreamBuffering, "AllowReadStreamBuffering");
			Assert.IsTrue (wr.AllowWriteStreamBuffering, "AllowWriteStreamBuffering");
		}

		bool IsValidHeader (HttpRequestHeader header)
		{
			switch (header) {
			case HttpRequestHeader.Connection:
			case HttpRequestHeader.Date:
			case HttpRequestHeader.KeepAlive:
			case HttpRequestHeader.Trailer:
			case HttpRequestHeader.TransferEncoding:
			case HttpRequestHeader.Upgrade:
			case HttpRequestHeader.Via:
			case HttpRequestHeader.Warning:
			case HttpRequestHeader.Allow:
			case HttpRequestHeader.ContentLength:
			case HttpRequestHeader.ContentType:
			case HttpRequestHeader.ContentLocation:
			case HttpRequestHeader.ContentRange:
			case HttpRequestHeader.LastModified:
			case HttpRequestHeader.Accept:
			case HttpRequestHeader.AcceptCharset:
			case HttpRequestHeader.AcceptEncoding:
			case HttpRequestHeader.AcceptLanguage:
			// Authorization was not valid before SL4
			// case HttpRequestHeader.Authorization:
			case HttpRequestHeader.Cookie:
			case HttpRequestHeader.Expect:
			case HttpRequestHeader.Host:
			case HttpRequestHeader.IfModifiedSince:
			case HttpRequestHeader.MaxForwards:
			// ProxyAuthorization was not valid before SL4
			// case HttpRequestHeader.ProxyAuthorization:
			case HttpRequestHeader.Referer:
			case HttpRequestHeader.Te:
			// Range was not valid in SL2 but is accepted in SL3
			// case HttpRequestHeader.Range:
			case HttpRequestHeader.UserAgent:
				return false;
			default:
				return true;
			}
		}

		void Headers_HttpRequestHeader (string method)
		{
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
				wr.Method = method;
				HttpRequestHeader header = (HttpRequestHeader) i;
				string s = i.ToString ();

				if (IsValidHeader (header)) {
					wr.Headers [header] = s;
					Assert.AreEqual (s, wr.Headers [header], header.ToString ());
				} else {
					Assert.Throws<ArgumentException> (delegate {
						wr.Headers [header] = s;
					}, header.ToString ());
				}
			}
		}

		[TestMethod]
		public void Headers_HttpRequestHeader_GET ()
		{
			Headers_HttpRequestHeader ("GET");
		}

		[TestMethod]
		public void Headers_HttpRequestHeader_POST ()
		{
			Headers_HttpRequestHeader ("POST");
		}

		void Headers_String (string method)
		{
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
				wr.Method = method;
				HttpRequestHeader hrh = (HttpRequestHeader) i;
				string header = WebHeaderCollectionTest.HttpRequestHeaderToString (hrh);
				string s = i.ToString ();

				if (IsValidHeader (hrh)) {
					// not case sensitive
					wr.Headers [header.ToUpper ()] = s;
					Assert.AreEqual (s, wr.Headers [header], header);
				} else {
					if (hrh == HttpRequestHeader.ContentType) {
						// we can set ContentType using its property, but not the collection
						Assert.IsNull (wr.ContentType, "ContentType/default");
						wr.ContentType = s;
						Assert.AreEqual (s, wr.ContentType, "ContentType");
						Assert.AreEqual (s, wr.Headers [header], "ContentType/Collection");
					}

					Assert.Throws<ArgumentException> (delegate {
						wr.Headers [header] = s;
					}, header);
				}
			}
		}

		[TestMethod]
		public void Headers_String_GET ()
		{
			Headers_String ("GET");
		}

		[TestMethod]
		public void Headers_String_POST ()
		{
			Headers_String ("POST");
		}

		[TestMethod]
		public void Methods_GetPost ()
		{
			WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
			Assert.AreEqual ("GET", wr.Method, "GET");

			wr.Method = "get";
			Assert.AreEqual ("get", wr.Method, "get");

			wr.Method = "poSt";
			Assert.AreEqual ("poSt", wr.Method, "poSt");
		}


		internal class TestState {
			public bool Complete;
			public WebRequest Request;
			public string Header;
			public Type Exception;

			public TestState (WebRequest wr, string header, Type exception)
			{
				Complete = false;
				Request = wr;
				Header = header;
				Exception = exception;
			}

			public override string ToString ()
			{
				return String.Format ("{0} {1} {2}", Request.Method, Header, Exception);
			}
		}

		protected void TryWebRequest (string method, string header, Type exception)
		{
			WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
			wr.Method = method;
			wr.Headers [header] = "value";

			TestState state = new TestState (wr, header, exception);
			Enqueue (() => { wr.BeginGetResponse (new AsyncCallback (TryWebRequestGetResponse), state); });
			EnqueueConditional (() => state.Complete);
		}

		protected void TryWebRequestGetResponse (IAsyncResult ar)
		{
			TestState state = (ar.AsyncState as TestState);
			try {
				WebResponse wr = state.Request.EndGetResponse (ar);
				Assert.IsNotNull (wr, "WebResponse");
				if (state.Exception != null)
					Assert.Fail (state.ToString () + " missing exception");
			}
			catch (Exception e) {
				Type type = e.GetType ();
				if (state.Exception != e.GetType ())
					Assert.Fail (state.ToString () + " got: " + type.ToString ());
			}
			finally {
				state.Complete = true;
			}
		}

		[TestMethod]
		[Asynchronous]
		public void HeadersPostThrowingInGetResponse ()
		{
			Type se = typeof (SecurityException);
			TryWebRequest ("POST", "Cache-Control", se);
			TryWebRequest ("POST", "Content-Encoding", se);
			TryWebRequest ("POST", "Content-Language", se);
			TryWebRequest ("POST", "Content-MD5", se);
			TryWebRequest ("POST", "Expires", se);

			EnqueueTestComplete ();
		}

		[TestMethod]
		public void BeginGetResponse_NullCallback ()
		{
			WebRequest wr = GetWebRequest (new Uri ("http://localhost"));
			Assert.Throws<NotSupportedException> (delegate {
				wr.BeginGetResponse (null, String.Empty);
			}, "null callback");
		}

		[TestMethod]
		[Asynchronous]
		public void ContentType_GET ()
		{
			WebClient wc = new WebClient ();
			if (new Uri (wc.BaseAddress).Scheme != "http") {
				EnqueueTestComplete ();
				return;
			}

			WebRequest request = WebRequest.Create (WebClientTest.GetSitePage ("POST.aspx"));
			IAsyncResult async_result = null;

			/* We can't set Headers ["Content-Type"] directly, neither with POST nor GET requests */
			request.Method = "GET";
			Assert.Throws<ArgumentException> (() => request.Headers ["Content-Type"] = "foobar");
			/* Set the ContentType property, assert that EndGetResponse fails */
			request.ContentType = "foobar";
			request.BeginGetResponse ((IAsyncResult result) => async_result = result, null);
			EnqueueConditional (() => async_result != null);
			Enqueue (() => Assert.Throws<ProtocolViolationException> (() => request.EndGetResponse (async_result)));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ContentType_POST ()
		{
			WebClient wc = new WebClient ();
			if (new Uri (wc.BaseAddress).Scheme != "http") {
				EnqueueTestComplete ();
				return;
			}

			WebRequest wr = GetWebRequest (new Uri (WebClientTest.GetSitePage ("POST.aspx")));
			IAsyncResult async_result = null;

			/* We can't set Headers ["Content-Type"] directly, neither with POST nor GET requests */
			wr.Method = "POST";
			Assert.Throws<ArgumentException> (() => wr.Headers ["Content-Type"] = "foobar");
			/* Set the ContentType property, assert that EndGetResponse does not fail */
			wr.ContentType = "foobar";
			wr.BeginGetResponse ((IAsyncResult result) => async_result = result, null);
			EnqueueConditional (() => async_result != null);
			Enqueue (() => wr.EndGetResponse (async_result));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void GetRequestStream_ThreadPool ()
		{
			WebClient wc = new WebClient ();
			if (new Uri (wc.BaseAddress).Scheme != "http") {
				EnqueueTestComplete ();
				return;
			}

			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			List<Stream> streams_to_close = new List<Stream> ();
			bool get_request_stream_called = false;
			
			/* Check that the GetRequestStream callback is executed on a thread-pool thread */

			HttpWebRequest request = (HttpWebRequest) GetWebRequest (new Uri (WebClientTest.GetSitePage ("timecode-long.wmv")));
			request.Method = "POST";
			Enqueue (() =>
			{
				request.BeginGetRequestStream (delegate (IAsyncResult ar)
				{
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in BeginGetRequestStream");
					streams_to_close.Add (request.EndGetRequestStream (ar));
					get_request_stream_called = true;
				}, request);
			});
			EnqueueConditional (() => get_request_stream_called);
			Enqueue (() => { streams_to_close.ForEach ((v) => v.Close ()); });
			EnqueueTestComplete ();
			
		}

		[TestMethod]
		[Asynchronous]
		public void GetResponse_ThreadPool ()
		{
			WebClient wc = new WebClient ();
			if (new Uri (wc.BaseAddress).Scheme != "http") {
				EnqueueTestComplete ();
				return;
			}

			DependencyObject TestPanel = this.TestPanel;
			Thread main_thread = Thread.CurrentThread;
			bool get_response_called = false;

			/* Check that the GetResponse callback is executed on a thread-pool thread */

			HttpWebRequest request = (HttpWebRequest) GetWebRequest (new Uri (WebClientTest.GetSitePage ("timecode-long.wmv")));
			
			Enqueue (() =>
			{
				request.BeginGetResponse (delegate (IAsyncResult ar)
				{
					Assert.IsFalse (TestPanel.CheckAccess ());
					Assert.AreNotEqual (main_thread.ManagedThreadId, Thread.CurrentThread.ManagedThreadId, "Different thread ids in BeginGetResponse");
					get_response_called = true;
					try {
						request.EndGetResponse (ar);
					} catch (Exception ex) {
						Console.WriteLine (ex);
					}
				}, request);
			});
			EnqueueConditional (() => get_response_called);
			EnqueueTestComplete ();
		}

		void NonHttpProtocol (Uri uri)
		{
			IAsyncResult async_result = null;
			WebRequest request = GetWebRequest (uri);
			request.BeginGetResponse ((IAsyncResult result) => async_result = result, null);
			EnqueueConditional (() => async_result != null);
			Enqueue (() => Assert.Throws<SecurityException> (() => request.EndGetResponse (async_result)));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void File ()
		{
			NonHttpProtocol (new Uri ("file:///test"));
		}

		[TestMethod]
		[Asynchronous]
		public void Ftp ()
		{
			NonHttpProtocol (new Uri ("ftp://ftp.novell.com"));
		}

		[TestMethod]
		[Asynchronous]
		public void Mailto ()
		{
			NonHttpProtocol (new Uri ("mailto:mono@novell.com"));
		}
	}
}

