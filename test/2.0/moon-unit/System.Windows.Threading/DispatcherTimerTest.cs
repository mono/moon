//
// Unit tests for DispatcherTimer
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
using System.ComponentModel;
using System.Threading;
using System.Windows;
using System.Windows.Threading;

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Threading {

	[TestClass]
	public class DispatcherTimerTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		public void MainThread ()
		{
			int tid = Thread.CurrentThread.ManagedThreadId;
			bool called = false;
			DispatcherTimer dt = new DispatcherTimer ();
			dt.Tick += delegate (object sender, EventArgs e) {
				try {
					Assert.AreSame (dt, sender, "sender");
					Assert.IsNotNull (e, "e");
					Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");

					Assert.IsTrue (dt.IsEnabled, "IsEnabled");
					dt.Stop ();
					Assert.IsFalse (dt.IsEnabled, "Stop");
				}
				finally {
					called = true;
				}
			};
			dt.Start ();
			EnqueueConditional (() => called);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ThreadPool ()
		{
			int tid = Thread.CurrentThread.ManagedThreadId;
			bool called = false;
			Enqueue (() => {
				DispatcherTimer dt = new DispatcherTimer ();
				dt.Tick += delegate (object sender, EventArgs e) {
					try {
						Assert.AreSame (dt, sender, "sender");
						Assert.IsNotNull (e, "e");
						Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");

						Assert.IsTrue (dt.IsEnabled, "IsEnabled");
						dt.Stop ();
						Assert.IsFalse (dt.IsEnabled, "Stop");
					}
					finally {
						called = true;
					}
				};
				dt.Start ();
			});
			EnqueueConditional (() => called);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UserThread_Ctor ()
		{
			bool called = false;
			Thread t = new Thread (delegate () {
				try {
					Assert.Throws<UnauthorizedAccessException> (delegate {
						new DispatcherTimer ();
					}, "UnauthorizedAccessException");
				}
				finally {
					called = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => called);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void UserThread_Tick ()
		{
			bool called = false;
			DispatcherTimer dt = new DispatcherTimer ();
			dt.Tick += TickCallback;
			Thread t = new Thread (delegate () {
				try {
					Assert.Throws<UnauthorizedAccessException> (delegate {
						dt.Tick += delegate (object sender, EventArgs e) { };
					}, "add/UnauthorizedAccessException");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						dt.Tick -= TickCallback;
					}, "remove/UnauthorizedAccessException");
					Assert.IsFalse (dt.IsEnabled, "IsEnabled");
				}
				finally {
					called = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => called);
			EnqueueTestComplete ();
		}

		void TickCallback (object sender, EventArgs e)
		{
		}

		[TestMethod]
		[Asynchronous]
		public void UserThread_StartStop ()
		{
			bool called = false;
			DispatcherTimer dt = new DispatcherTimer ();
			Thread t = new Thread (delegate () {
				try {
					Assert.IsFalse (dt.IsEnabled, "IsEnabled");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						dt.Start ();
					}, "Start/UnauthorizedAccessException");
					// Start throws but IsEnabled will report true
					Assert.IsTrue (dt.IsEnabled, "Start");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						dt.Stop ();
					}, "Stop/UnauthorizedAccessException");
					// Stop throws but IsEnabled will report false
					Assert.IsFalse (dt.IsEnabled, "Stop");
				}
				finally {
					called = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => called);
			EnqueueTestComplete ();
		}
	}
}

