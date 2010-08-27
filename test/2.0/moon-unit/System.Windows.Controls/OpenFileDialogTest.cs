//
// OpenFileDialog Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008, 2010 Novell, Inc.
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
	public class OpenFileDialogTest : SilverlightTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();
			Assert.IsNull (ofd.File, "File");
			Assert.IsNull (ofd.Files, "Files");
			Assert.AreEqual (String.Empty, ofd.Filter, "Filter");
			Assert.AreEqual (0, ofd.FilterIndex, "FilterIndex");
			Assert.IsFalse (ofd.Multiselect, "Multiselect");
		}

		[TestMethod]
		public void Filter ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();
			ofd.Filter = null;
			Assert.AreEqual (String.Empty, ofd.Filter, "Null->Empty");

			ofd.Filter = "All Files (*.*)|*.*";

			Assert.Throws<ArgumentException> (delegate {
				ofd.Filter = "a|b|";
			}, "Even |");

			ofd.Filter = "a|*.b";
			Assert.AreEqual ("a|*.b", ofd.Filter, "Unchanged");
		}

		[TestMethod]
		public void FilterIndex ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();

			Assert.AreEqual (0, ofd.FilterIndex, "0");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				// it's default value is unacceptable ;-)
				ofd.FilterIndex = ofd.FilterIndex;
			}, "0");

			ofd.FilterIndex = 1;
			Assert.AreEqual (1, ofd.FilterIndex, "1");

			ofd.FilterIndex = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, ofd.FilterIndex, "MaxValue");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				ofd.FilterIndex = -1;
			}, "negative");
			Assert.AreEqual (Int32.MaxValue, ofd.FilterIndex, "Unchanged");
		}

		[TestMethod]
		public void ShowDialog ()
		{
			// note: earlier SL (before 4) did not have this check
			OpenFileDialog ofd = new OpenFileDialog ();
			Assert.Throws<SecurityException> (delegate {
				ofd.ShowDialog ();
			}, "user initiated check");
		}

		[TestMethod]
		[Asynchronous]
		public void UserThread ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();

			bool complete = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");

					Assert.Throws<InvalidOperationException> (delegate {
						new OpenFileDialog ();
					}, "new");

					Assert.Throws<InvalidOperationException> (delegate {
						ofd.ShowDialog ();
					}, "ShowDialog");

					Assert.Throws<InvalidOperationException> (delegate {
						Assert.IsNull (ofd.File);
					}, "get_File");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.IsNull (ofd.Files);
					}, "get_Files");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (String.Empty, ofd.Filter);
					}, "get_Filter");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.AreEqual (0, ofd.FilterIndex);
					}, "get_FilterIndex");
					Assert.Throws<InvalidOperationException> (delegate {
						Assert.IsFalse (ofd.Multiselect);
					}, "get_Multiselect");

					Assert.Throws<InvalidOperationException> (delegate {
						ofd.Filter = null;
					}, "set_Filter");
					Assert.Throws<InvalidOperationException> (delegate {
						ofd.FilterIndex = 1;
					}, "set_FilterIndex");
					Assert.Throws<InvalidOperationException> (delegate {
						ofd.Multiselect = true;
					}, "set_Multiselect");
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
