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

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class WebClientTest : SilverlightTest {

		private void CheckDefaults (WebClient wc)
		{
			Assert.IsTrue (wc.AllowReadStreamBuffering, "AllowReadStreamBuffering");
//			Assert.IsTrue (wc.BaseAddress.EndsWith ("/moon-unit.xap"), "BaseAddress");
Console.WriteLine ("wc.BaseAddress: {0}", wc.BaseAddress);
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
		[Asynchronous]
		[MoonlightBug]
		public void DownloadStringAsync ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
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
		[MoonlightBug]
		public void DownloadStringAsync_UserToken ()
		{
			WebClient wc = new WebClient ();
			bool complete = false;
			wc.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
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
	}
}

