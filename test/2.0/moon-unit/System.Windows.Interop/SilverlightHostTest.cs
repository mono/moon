//
// Unit tests for System.Windows.Interop.SilverlightHost
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008, 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows;
using System.Windows.Browser;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Resources;
using System.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class SilverlightHostTest : SilverlightTest {

		static private Uri uri = new Uri ("http://www.mono-project.com");

		void Check (SilverlightHost host)
		{
			Assert.AreEqual (Colors.White, host.Background, "Background");
			Assert.IsNotNull (host.Content, "Content");
			Assert.IsTrue (host.IsLoaded, "IsLoaded");
			Assert.AreEqual (String.Empty, host.NavigationState, "NavigationState");
			Assert.IsNotNull (host.Settings, "Settings");
			Assert.IsTrue (host.Source.AbsoluteUri.EndsWith ("moon-unit.xap"), "Source");
		}

		[TestMethod]
		public void New ()
		{
			SilverlightHost host = new SilverlightHost ();
			Check (host);
		}

		[TestMethod]
		public void Current ()
		{
			SilverlightHost host = Application.Current.Host;
			Check (host);
		}

		[TestMethod]
		[SilverlightBug(PlatformID.MacOSX)]
		public void IsVersionSupported ()
		{
			SilverlightHost host = new SilverlightHost ();

			Assert.Throws (delegate { host.IsVersionSupported (null); }, typeof (NullReferenceException), "IsVersionSupported(null)");

			Assert.IsFalse (host.IsVersionSupported (String.Empty), "Empty");
			Assert.IsFalse (host.IsVersionSupported ("1"), "1");
			Assert.IsFalse (host.IsVersionSupported ("1."), "1.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0"), "1.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0."), "1.0.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0.0"), "1.0.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0."), "1.0.0.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0.0.0"), "1.0.0.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0.0."), "1.0.0.0.");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0.0.0"), "1.0.0.0.0");
		}

		[TestMethod]
		public void InitParams ()
		{
			SilverlightHost host = Application.Current.Host;
			// what's added at Application.Startup is available later (see App.xaml.cs)
			Assert.IsTrue (host.InitParams.ContainsKey ("Moon-y-Test"), "Application_Startup");
			int n = host.InitParams.Count;
			host.InitParams.Add ("a", "b"); // not read-only
			// a single copy is kept
			Assert.AreEqual (n + 1, Application.Current.Host.InitParams.Count, "Count");
			host.InitParams.Remove ("a");
			Assert.AreEqual (n, Application.Current.Host.InitParams.Count, "Count-2");
		}

		// Fragment is Empty by default but is easy to change to a single '#' on Silverlight - not that it should change things...
		static bool IsFragmentEmptyOrSharp (string s)
		{
			return ((s.Length == 0) || (s == "#"));
		}

		[TestMethod]
		public void StateChange ()
		{
			SilverlightHost host = Application.Current.Host;
			Assert.AreEqual (String.Empty, host.NavigationState, "NavigationState-original");
			Assert.AreEqual (String.Empty, HtmlPage.Window.CurrentBookmark, "CurrentBookmark-original");
			// Empty by default - but some other (previous) test may have already changed it to '#'
			Assert.IsTrue (IsFragmentEmptyOrSharp (HtmlPage.Document.DocumentUri.Fragment), "Fragment-original");

			int n = 0;
			host.NavigationStateChanged += delegate (object sender, NavigationStateChangedEventArgs e) {
				Assert.AreSame (host, sender, "sender");
				Assert.AreEqual (n == 0 ? "a" : String.Empty, e.NewNavigationState, "NewNavigationState");
				Assert.AreEqual (n == 0 ? String.Empty : "a", e.PreviousNavigationState, "PreviousNavigationState");
				n++;
			};
		
			host.NavigationState = host.NavigationState;
			Assert.AreEqual (n, 0, "same state");
			Assert.AreEqual (String.Empty, HtmlPage.Window.CurrentBookmark, "CurrentBookmark-same");
			// resetting NavigationState to same (even empty) value introduced a '#' for fragment
			Assert.IsTrue (IsFragmentEmptyOrSharp (HtmlPage.Document.DocumentUri.Fragment), "Fragment-same");
			
			host.NavigationState = "a";
			Assert.AreEqual (n, 1, "different state");
			Assert.AreEqual ("a", HtmlPage.Window.CurrentBookmark, "CurrentBookmark-a");
			Assert.AreEqual ("#a", HtmlPage.Document.DocumentUri.Fragment, "Fragment-a");

			host.NavigationState = String.Empty;
			Assert.AreEqual (n, 2, "back to original");
			Assert.AreEqual (String.Empty, HtmlPage.Window.CurrentBookmark, "CurrentBookmark-b");
			Assert.IsTrue (IsFragmentEmptyOrSharp (HtmlPage.Document.DocumentUri.Fragment), "Fragment-b");

			// using CurrentBookmark will change NavigationState but won't fire the event
			HtmlPage.Window.CurrentBookmark = "a";
			Assert.AreEqual (n, 2, "different state using CurrentBookmark");
			Assert.AreEqual ("a", host.NavigationState, "Document-a2");
			Assert.AreEqual ("#a", HtmlPage.Document.DocumentUri.Fragment, "Fragment-a2");

			HtmlPage.Window.CurrentBookmark = String.Empty;
			Assert.AreEqual (n, 2, "back to original using CurrentBookmark");
			Assert.AreEqual (String.Empty, host.NavigationState, "Document-b2");
			Assert.IsTrue (IsFragmentEmptyOrSharp (HtmlPage.Document.DocumentUri.Fragment), "Fragment-b2");
		}

		void host_NavigationStateChanged (object sender, NavigationStateChangedEventArgs e)
		{
		}

		[Asynchronous]
		[TestMethod]
		public void AsyncStateChange_Dispatch ()
		{
			SilverlightHost host = Application.Current.Host;
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Application.Current.RootVisual.Dispatcher.BeginInvoke (() => {
				try {
					Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					host.NavigationStateChanged += host_NavigationStateChanged;
					host.NavigationState = "dispatch"; // set
					Assert.AreEqual ("dispatch", host.NavigationState, "get");
					host.NavigationStateChanged -= host_NavigationStateChanged;
					status = true;
				}
				finally {
					complete = true;
					host.NavigationState = String.Empty;
					Assert.IsTrue (status, "Success");
				}
			});
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[Asynchronous]
		[TestMethod]
		public void AsyncStateChange_UserThread ()
		{
			SilverlightHost host = Application.Current.Host;
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						host.NavigationStateChanged += host_NavigationStateChanged;
					}, "add_NavigationStateChanged");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (host.NavigationState, "get");
					}, "get_NavigationState");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						host.NavigationState = "user thread";
					}, "set_NavigationState");
					host.NavigationStateChanged -= host_NavigationStateChanged;
					status = true;
				}
				finally {
					complete = true;
					Assert.IsTrue (status, "Success");
				}
			});
			t.Start ();
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}
	}
}
