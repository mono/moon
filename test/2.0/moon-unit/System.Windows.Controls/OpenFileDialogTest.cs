//
// OpenFileDialog Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class OpenFileDialogTest {

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

			ofd.Filter = "a|b";
			Assert.AreEqual ("a|b", ofd.Filter, "Filter");

			Assert.Throws<ArgumentException> (delegate {
				ofd.Filter = "a|b|";
			}, "Even |");
			Assert.AreEqual ("a|b", ofd.Filter, "Unchanged");
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

		[Ignore ("blocking UI")]
		[TestMethod]
		public void SingleSelection ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();
			bool? result = ofd.ShowDialog ();
			if (result.HasValue && result.Value) {
				CheckFileInfo (ofd.File);
				// there can be only one FileInfo inside Files
				int n = 0;
				foreach (FileInfo fi in ofd.Files) {
					CheckFileInfo (fi);
					n++;
					Assert.AreEqual (ofd.File.Name, fi.Name, "File.Name");
				}
				Assert.AreEqual (1, n, "Files");
			} else {
				Assert.IsNull (ofd.File, "File");
				Assert.IsNull (ofd.Files, "Files");
			}
		}

		[Ignore ("blocking UI")]
		[TestMethod]
		public void MultipleSelection ()
		{
			OpenFileDialog ofd = new OpenFileDialog ();
			ofd.Multiselect = true;
			bool? result = ofd.ShowDialog ();
			if (result.HasValue && result.Value) {
				foreach (FileInfo fi in ofd.Files) {
					CheckFileInfo (fi);
				}
			} else {
				Assert.IsNull (ofd.File, "File");
				Assert.IsNull (ofd.Files, "Files");
			}
		}

		public void CheckFileInfo (FileInfo fi)
		{
			string s = fi.ToString ();
			Assert.IsFalse (s.IndexOf (Path.AltDirectorySeparatorChar) >= 0, "AltDirectorySeparatorChar");
			Assert.IsFalse (s.IndexOf (Path.DirectorySeparatorChar) >= 0, "DirectorySeparatorChar");
			Assert.IsFalse (s.IndexOf (Path.PathSeparator) >= 0, "PathSeparator");
			Assert.IsFalse (s.IndexOf (Path.VolumeSeparatorChar) >= 0, "VolumeSeparatorChar");

			try {
				Assert.AreEqual (String.Empty, fi.Directory, "Directory");
				throw new SecurityException ("MethodAccessException was not thrown by the runtime");
			}
			catch (MethodAccessException) {
				// SecurityCritical
			}
		}
	}
}
