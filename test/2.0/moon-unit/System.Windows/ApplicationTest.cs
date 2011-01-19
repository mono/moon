//
// Unit tests for System.Windows.Application
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008-2011 Novell, Inc (http://www.novell.com)
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
using System.Threading;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Resources;
using System.Windows.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class ApplicationTest : SilverlightTest {

		static private Uri uri = new Uri ("http://www.mono-project.com");

		void Check (Application app)
		{
			Assert.IsNotNull (app.ApplicationLifetimeObjects, "ApplicationLifetimeObjects");
			Assert.IsNotNull (app.Host, "Host");
			Assert.AreEqual (InstallState.NotInstalled, app.InstallState, "InstallState");
			Assert.IsFalse (app.IsRunningOutOfBrowser, "IsRunningOutOfBrowser");
			Assert.IsNotNull (app.Resources, "Resources");
			Assert.IsNotNull (app.RootVisual, "RootVisual");
		}

		[TestMethod]
		public void New ()
		{
			Application app = new Application ();
			// AFAIK this is *nearly* identical to Application.Current
			Check (app);
			// funny thing is that creating a SilverlightHost defaults to true :|
			Assert.IsTrue (app.Host.IsLoaded, "Host.IsLoaded");
		}

		[TestMethod]
		public void Current ()
		{
			Application app = Application.Current;
			Check (app);
			Assert.IsTrue (app.Host.IsLoaded, "Host.IsLoaded");
		}

		[TestMethod]
		public void GetResourceStream_MemoryStream ()
		{
			StreamResourceInfo sri = Application.GetResourceStream (new Uri ("AppManifest.xaml", UriKind.Relative));
			Assert.IsNull (sri.ContentType, "ContentType");
			Stream s = sri.Stream;
			Assert.IsNotNull (s, "Stream");

			Assert.IsTrue (s.GetType ().FullName.Contains ("MemoryStream"), "MemoryStream");
			Assert.IsTrue (s.CanRead, "CanRead");
			Assert.IsTrue (s.CanSeek, "CanSeek");
			Assert.IsFalse (s.CanTimeout, "CanTimeout");
			Assert.IsFalse (s.CanWrite, "CanWrite");
			Assert.IsTrue (s.Length > 0, "Length");
			Assert.AreEqual (0, s.Position, "Position");
		}

		[TestMethod]
		[Asynchronous]
		public void GetResourceStream_Dispatched ()
		{
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Application.Current.RootVisual.Dispatcher.BeginInvoke (() => {
				try {
					Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					GetResourceStream_MemoryStream ();
					status = true;
				}
				finally {
					complete = true;
					Assert.IsTrue (status, "Success");
				}
			});
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void GetResourceStream_UserThread ()
		{
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					GetResourceStream_MemoryStream ();
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

		[TestMethod]
		public void InvalidGetResourceStream ()
		{
			Assert.Throws<ArgumentNullException> (() => Application.GetResourceStream (null), "GetResourceStream(null)");
			Assert.Throws<ArgumentNullException> (() => Application.GetResourceStream (null, uri), "GetResourceStream(null,uri)");

			Assert.IsNull (Application.GetResourceStream (new Uri ("/moon_unit;component/App-does-not-exists.xaml", UriKind.Relative)), "GetResourceStream(does-not-exists-uri)");

			Assert.Throws<ArgumentException> (() => Application.GetResourceStream (new Uri ("http://mono-project.com/", UriKind.Absolute)), "GetResourceStream absolute uri");

			StreamResourceInfo sri = new StreamResourceInfo (new MemoryStream (), String.Empty);

			Assert.Throws<ArgumentNullException> (() => Application.GetResourceStream (sri, null), "GetResourceStream(sri,null)");

			Assert.Throws<IndexOutOfRangeException> (() => Application.GetResourceStream (new Uri ("", UriKind.Relative)));

			Uri docuri = HtmlPage.Document.DocumentUri;
			Uri relative = new Uri (docuri.LocalPath, UriKind.RelativeOrAbsolute);
			Assert.IsNull (Application.GetResourceStream (relative), "GetResourceStream(out-of-xap)");
		}

		[TestMethod]
		public void InvalidLoadComponent ()
		{
			Assert.Throws<ArgumentNullException> (() => Application.LoadComponent (null, uri), "LoadComponent(null,uri)");
		}

		[TestMethod]
		public void LoadComponent_Application ()
		{
			Uri docuri = HtmlPage.Document.DocumentUri;

			if (docuri.Scheme != "http" && docuri.Scheme != "https")
				return;
			
			// Note: Exception message can be misleading
			Assert.Throws<ArgumentNullException> (() => Application.LoadComponent (Application.Current, null), "LoadComponent(app,null)");
			Assert.Throws<ArgumentException> (() => Application.LoadComponent (Application.Current, uri), "LoadComponent(app,uri)");
			Assert.Throws<ArgumentException> (() => Application.LoadComponent (Application.Current, Application.Current.Host.Source), "LoadComponent(app,xap)");

			// try to load an unexisting uri
			Application.LoadComponent (Application.Current, new Uri ("/moon_unit;component/App-does-not-exists.xaml", UriKind.Relative));
		}

		[TestMethod]
		public void LoadComponent_Application_OutsideXap ()
		{
			Uri docuri = HtmlPage.Document.DocumentUri;
			Uri relative = new Uri (docuri.LocalPath, UriKind.RelativeOrAbsolute);
			Assert.Throws<XamlParseException> (() => Application.LoadComponent (Application.Current, relative), "LoadComponent(app,out-of-xap)");
		}

		[TestMethod]
		public void ResetRootVisual ()
		{
			UIElement root = Application.Current.RootVisual;
			Canvas new_root = new Canvas ();
			Application.Current.RootVisual = new_root;
			// we can't change the root
			Assert.IsFalse (Object.ReferenceEquals (new_root, Application.Current.RootVisual), "new root");
			Assert.IsTrue (Object.ReferenceEquals (root, Application.Current.RootVisual), "old root");
			// and we can't clear it
			Assert.Throws (delegate { Application.Current.RootVisual = null; }, typeof (InvalidOperationException), "null");
		}

		[TestMethod]
		public void NonUserInitiated ()
		{
			Assert.IsFalse (Application.Current.Install (), "Install");
		}

		[TestMethod]
		[Asynchronous]
		[MinRuntimeVersion (3)]
		public void Properties_UserThread ()
		{
			Application app = Application.Current;
			if (app.IsRunningOutOfBrowser)
				return;

			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					// testing instance properties (not Application.Current)
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (app.ApplicationLifetimeObjects);
					}, "ApplicationLifetimeObjects");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (app.Host);
					}, "Host");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.AreEqual (InstallState.NotInstalled, app.InstallState);
					}, "InstallState");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsFalse (app.IsRunningOutOfBrowser);
					}, "IsRunningOutOfBrowser");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (app.Resources);
					}, "Resources");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (app.RootVisual);
					}, "RootVisual");
					status = true;
				}
				finally {
					complete = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => complete && status);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[MinRuntimeVersion (4)]
		public void MainWindow ()
		{
			Application app = Application.Current;
			if (app.IsRunningOutOfBrowser)
				return;

			Assert.Throws<NotSupportedException> (delegate {
				Assert.IsNotNull (app.MainWindow);
			}, "MainWindow");
		}

		[TestMethod]
		[Asynchronous]
		[MinRuntimeVersion (4)]
		public void MainWindow_Dispatched ()
		{
			Application app = Application.Current;
			if (app.IsRunningOutOfBrowser)
				return;

			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Application.Current.RootVisual.Dispatcher.BeginInvoke (() => {
				try {
					Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					Assert.Throws<NotSupportedException> (delegate {
						Assert.IsNotNull (app.MainWindow);
					}, "MainWindow");
					status = true;
				}
				finally {
					complete = true;
				}
			});
			EnqueueConditional (() => complete && status);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MinRuntimeVersion (4)]
		public void MainWindow_UserThread ()
		{
			Application app = Application.Current;
			if (app.IsRunningOutOfBrowser)
				return;

			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (app.MainWindow);
					}, "MainWindow");
					status = true;
				}
				finally {
					complete = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => complete && status);
			EnqueueTestComplete ();
		}
	}
}
