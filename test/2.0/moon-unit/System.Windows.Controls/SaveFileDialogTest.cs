//
// SaveFileDialog Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2010 Novell, Inc.
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
using System.Security;
using System.Windows;
using System.Windows.Controls;
using System.Threading;

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class SaveFileDialogTest : SilverlightTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			SaveFileDialog sfd = new SaveFileDialog ();
			Assert.AreEqual (String.Empty, sfd.DefaultExt, "DefaultExt");
			Assert.AreEqual (String.Empty, sfd.SafeFileName, "SafeFileName");
			Assert.AreEqual (String.Empty, sfd.Filter, "Filter");
			Assert.AreEqual (1, sfd.FilterIndex, "FilterIndex");
		}

		[TestMethod]
		public void DefaultExt ()
		{
			SaveFileDialog sfd = new SaveFileDialog ();
			sfd.DefaultExt = null;
			Assert.AreEqual (String.Empty, sfd.DefaultExt, "Null->Empty");

			sfd.DefaultExt = ".exe";
			// leading dot is removed
			Assert.AreEqual ("exe", sfd.DefaultExt, "exe");
			// if supplied
			sfd.DefaultExt = "html";
			Assert.AreEqual ("html", sfd.DefaultExt, "html");
			// removed if alone
			sfd.DefaultExt = ".";
			Assert.AreEqual (String.Empty, sfd.DefaultExt, ".");
			// kept if multiple
			sfd.DefaultExt = "..";
			Assert.AreEqual (".", sfd.DefaultExt, "..");

			sfd.DefaultExt = ".*";
			// that's likely a problem later...
			Assert.AreEqual ("*", sfd.DefaultExt, "*");
		}

		[TestMethod]
		[MoonlightBug]
		public void Filter ()
		{
			SaveFileDialog sfd = new SaveFileDialog ();

			sfd.Filter = null;
			Assert.AreEqual (String.Empty, sfd.Filter, "Null->Empty");

			Assert.Throws<ArgumentException>(() =>
				sfd.Filter = "a|b"
			, "Invalid 1");

			Assert.Throws<ArgumentException> (delegate {
				sfd.Filter = "a|b|";
			}, "Even |");

			sfd.Filter = "a|*.b";
			Assert.AreEqual("a|*.b", sfd.Filter, "Unchanged");
		}

		[TestMethod]
		public void FilterIndex ()
		{
			SaveFileDialog sfd = new SaveFileDialog ();

			// unlike OpenFileDialog the default index is 1 (instead of 0)
			Assert.AreEqual (1, sfd.FilterIndex, "1");
			// but it makes more sense that having an invalid default ;-)
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				sfd.FilterIndex = 0;
			}, "0");

			sfd.FilterIndex = 1;
			Assert.AreEqual (1, sfd.FilterIndex, "1");

			sfd.FilterIndex = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, sfd.FilterIndex, "MaxValue");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				sfd.FilterIndex = -1;
			}, "negative");
			Assert.AreEqual (Int32.MaxValue, sfd.FilterIndex, "Unchanged");
		}

		[TestMethod]
		public void OpenFile ()
		{
			// incomplete since we can't call (correctly) ShowDialog inside unit tests
			SaveFileDialog sfd = new SaveFileDialog ();
			Assert.Throws<InvalidOperationException> (delegate {
				sfd.OpenFile ();
			}, "OpenFile");
		}

		[TestMethod]
		public void ShowDialog ()
		{
			// we cannot test the "correct" behavior in unit tests since this
			// method is not called from a user action
			SaveFileDialog sfd = new SaveFileDialog ();
			Assert.Throws<SecurityException> (delegate {
				sfd.ShowDialog ();
			}, "SaveDialog");
		}

		[TestMethod]
		[Asynchronous]
		public void UserThread ()
		{
			SaveFileDialog sfd = new SaveFileDialog ();

			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");

					Assert.Throws<InvalidOperationException> (delegate {
						new SaveFileDialog ();
					}, "new");

					Assert.Throws<InvalidOperationException> (delegate {
						sfd.ShowDialog ();
					}, "ShowDialog");
					Assert.Throws<InvalidOperationException> (delegate {
						sfd.OpenFile ();
					}, "OpenFile");

					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (String.Empty, sfd.DefaultExt);
					}, "get_DefaultExt");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (String.Empty, sfd.Filter);
					}, "get_Filter");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (0, sfd.FilterIndex);
					}, "get_FilterIndex");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (String.Empty, sfd.SafeFileName);
					}, "get_SafeFileName");

					Assert.Throws<InvalidOperationException> (delegate {
						sfd.DefaultExt = null;
					}, "set_DefaultExt");
					Assert.Throws<InvalidOperationException> (delegate {
						sfd.Filter = null;
					}, "set_Filter");
					Assert.Throws<InvalidOperationException> (delegate {
						sfd.FilterIndex = 1;
					}, "set_FilterIndex");
				}
				finally {
					complete = true;
				}
			});
			t.Start ();
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}
	}
}

