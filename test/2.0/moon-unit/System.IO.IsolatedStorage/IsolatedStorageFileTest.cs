//
// Unit tests for System.IO.IsolatedStorage.IsolatedStorageFile
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.IO.IsolatedStorage;
using System.Security;
using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.IO.IsolatedStorage {

	[TestClass]
	public class IsolatedStorageFileTest {

		[TestMethod]
		public void AvailableFreeSpace ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.IsTrue (isf.AvailableFreeSpace > 0, "AvailableFreeSpace");

			isf.Remove ();
			Assert.Throws (delegate { Console.WriteLine (isf.AvailableFreeSpace); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { Console.WriteLine (isf.AvailableFreeSpace); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void CreateDirectory ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.Throws (delegate { isf.CreateDirectory (null); }, typeof (ArgumentNullException), "null");

			string baddir = "\x00"; // <- the only invalid char, on linux, that is invalid in a directory
			Assert.Throws (delegate { isf.CreateDirectory (baddir); }, typeof (ArgumentException), "null char");

			baddir = Path.Combine ("..", "gotcha");
			Assert.Throws (delegate { isf.CreateDirectory (baddir); }, typeof (IsolatedStorageException), "../gotcha");

			isf.CreateDirectory (String.Empty);

			isf.CreateDirectory ("dir-1");
			Assert.IsTrue (isf.DirectoryExists ("dir-1"), "Exists(dir-1)");

			string codir = Path.Combine ("dir-1", "..");
			isf.CreateDirectory (codir);
			Assert.IsTrue (isf.DirectoryExists (codir), "Exists(dir-1/..)");

			codir = Path.Combine (codir, "dir-3");
			isf.CreateDirectory (codir);
			Assert.IsTrue (isf.DirectoryExists (codir), "Exists(dir-1/../dir-3)");

			string subdir = Path.Combine ("dir-1", "dir-2");
			isf.CreateDirectory (subdir);
			Assert.IsTrue (isf.DirectoryExists (subdir), "Exists(dir-1/dir2)");

			isf.DeleteDirectory (subdir);
			Assert.IsFalse (isf.DirectoryExists (subdir), "Delete-Exists(dir-1/dir2)");
			Assert.IsTrue (isf.DirectoryExists ("dir-1"), "Delete-Exists(dir-1)");

			isf.DeleteDirectory ("dir-1");
			Assert.IsFalse (isf.DirectoryExists ("dir-1"), "Delete2-Exists(dir-1)");

			isf.Remove ();
			Assert.Throws (delegate { isf.CreateDirectory ("a"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.CreateDirectory ("a"); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void CreateFile ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			Assert.Throws<ArgumentNullException> (delegate {
				isf.CreateFile (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				isf.CreateFile (String.Empty);
			}, "Empty");

			try {
				isf.DeleteFile ("create-file");
			} catch (IsolatedStorageException) {
				// ignore this exception here since
				// it's generated when the file
				// doesn't exist.
			}

			// now test the above behavior just to make sure
			Assert.Throws (delegate { isf.DeleteFile ("create-file"); }, typeof (IsolatedStorageException));

			Assert.IsFalse (isf.FileExists ("create-file"), "before");
			IsolatedStorageFileStream fs = isf.CreateFile ("create-file");
			try {
				Assert.IsTrue (isf.FileExists ("create-file"), "after");

				Assert.Throws (delegate { isf.CreateFile (null); }, typeof (ArgumentNullException), "null");
				Assert.Throws (delegate { isf.CreateFile (String.Empty); }, typeof (ArgumentException), "empty");
				Assert.Throws (delegate { isf.CreateFile ("does-not-exist/new"); }, typeof (IsolatedStorageException), "subdir does not exist");
			}
			finally {
				fs.Close ();
				isf.DeleteFile ("create-file");
				Assert.IsFalse (isf.FileExists ("create-file"), "deleted");
			}
			isf.Remove ();
			Assert.Throws (delegate { isf.CreateFile (null); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.CreateFile (null); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void Dispose ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			isf.Dispose ();
			isf.Dispose (); // can be disposed multiple times
		}

		[TestMethod]
		public void DeleteDirectory ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			Assert.Throws<ArgumentNullException> (delegate { 
				isf.DeleteDirectory (null);
			}, "null");
			isf.DeleteDirectory (String.Empty);

			Assert.IsFalse (isf.DirectoryExists ("dir-exist"), "Before/Exists(dir-exist)");
			isf.CreateDirectory ("dir-exist");
			Assert.IsTrue (isf.DirectoryExists ("dir-exist"), "After/Exists(dir-exist)");
			isf.DeleteDirectory ("dir-exist");
			Assert.IsFalse (isf.DirectoryExists ("dir-exist"), "Delete/Exists(dir-exist)");

			isf.Remove ();
			Assert.Throws (delegate { isf.DeleteDirectory ("does not exists"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.DeleteDirectory ("does not exists"); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void DeleteFile ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			Assert.Throws<ArgumentNullException> (delegate {
				isf.DeleteFile (null);
			}, "null");
			Assert.Throws<IsolatedStorageException> (delegate {
				isf.DeleteFile (String.Empty);
			}, "Empty");

			Assert.IsFalse (isf.FileExists ("file-exist"), "Before/Exists(file-exist)");
			using (IsolatedStorageFileStream s = isf.CreateFile ("file-exist")) {
				Assert.IsTrue (isf.FileExists ("file-exist"), "After/Exists(file-exist)");
			}
			isf.DeleteFile ("file-exist");
			Assert.IsFalse (isf.FileExists ("file-exist"), "Delete/Exists(file-exist)");

			isf.Remove ();
			Assert.Throws (delegate { isf.DeleteFile ("does not exists"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.DeleteFile ("does not exists"); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void DirectoryExists ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			Assert.Throws<NullReferenceException> (delegate {
				isf.DirectoryExists (null);
			}, "null");
			Assert.IsTrue (isf.DirectoryExists (String.Empty), "Empty");

			Assert.IsFalse (isf.DirectoryExists ("does not exists"), "DirectoryExists(doesnotexists)");
			Assert.IsFalse (isf.DirectoryExists ("dir-exist"), "DirectoryExists(dir-exist)");
			string[] dirs = isf.GetDirectoryNames ();
			int ndir = dirs.Length;
			try {
				isf.CreateDirectory ("dir-exist");
				Assert.IsTrue (isf.DirectoryExists ("dir-exist"), "DirectoryExists(dir-exist)");
				Assert.AreEqual (ndir + 1, isf.GetDirectoryNames ().Length, "GetDirectoryNames");
				dirs = isf.GetDirectoryNames ("dir-exist");
				Assert.AreEqual (1, dirs.Length, "Length");
				// make sure we're not leaking the full path to the directory
				Assert.AreEqual ("dir-exist", dirs [0], "dir-exist");
			}
			finally {
				isf.DeleteDirectory ("dir-exist");
				Assert.IsFalse (isf.DirectoryExists ("dir-exist"), "Delete/Exists(dir-exist)");
				Assert.AreEqual (ndir, isf.GetDirectoryNames ().Length, "Delete/GetDirectoryNames");
				dirs = isf.GetDirectoryNames ("dir-exist");
				Assert.AreEqual (0, dirs.Length, "Delete/Length");
			}

			isf.Remove ();
			Assert.Throws (delegate { isf.DirectoryExists ("does not exists"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.DirectoryExists ("does not exists"); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void FileExists ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			Assert.Throws<NullReferenceException> (delegate {
				isf.FileExists (null);
			}, "null");
			Assert.IsFalse (isf.FileExists (String.Empty), "Empty");

			Assert.IsFalse (isf.FileExists ("does not exists"), "FileExists(doesnotexists)");

			isf.Remove ();
			Assert.Throws (delegate { isf.FileExists ("does not exists"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.FileExists ("does not exists"); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void GetDirectoryNames ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.IsNotNull (isf.GetDirectoryNames (), "GetDirectoryNames");
			Assert.Throws<ArgumentNullException> (delegate {
				isf.GetDirectoryNames (null);
			}, "GetDirectoryNames(null)");
			Assert.Throws<IsolatedStorageException> (delegate {
				isf.GetDirectoryNames (String.Empty);
			}, "GetDirectoryNames(String.Empty)");
			
			foreach (char c in Path.GetInvalidPathChars ()) {
				string s = c.ToString ();
				Assert.Throws<ArgumentException> (delegate {
					isf.GetDirectoryNames (s);
				}, s);
			}
		}

		[TestMethod]
		public void GetFileNames ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.IsNotNull (isf.GetFileNames (), "GetFileNames");
			Assert.Throws<ArgumentNullException> (delegate {
				isf.GetFileNames (null);
			}, "GetFileNames(null)");
			Assert.Throws<IsolatedStorageException> (delegate {
				isf.GetFileNames (String.Empty);
			}, "GetFileNames(String.Empty)");

			foreach (char c in Path.GetInvalidPathChars ()) {
				string s = c.ToString ();
				Assert.Throws<ArgumentException> (delegate {
					isf.GetFileNames (s);
				}, s);
			}
		}

		[TestMethod]
		public void IncreaseQuotaTo ()
		{
			// Fails in Silverlight 3
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			// LAMESPEC: documented as ArgumentOutOfRangeException on MSDN (reported)
			Assert.Throws (delegate { isf.IncreaseQuotaTo (-1); }, typeof (ArgumentException), "negative");
			Assert.Throws (delegate { isf.IncreaseQuotaTo (isf.Quota); }, typeof (ArgumentException), "current quota");

			isf.Remove ();
			Assert.Throws (delegate { isf.IncreaseQuotaTo (1); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.IncreaseQuotaTo (1); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void Quota ()
		{
			// Fails in Silverlight 3
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.IsTrue (isf.Quota > 0, "Quota");

			isf.Remove ();
			Assert.Throws (delegate { Console.WriteLine (isf.Quota); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { Console.WriteLine (isf.Quota); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void Remove ()
		{
			// Fails in Silverlight 3
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();

			isf.Remove ();
			Assert.Throws (delegate { isf.Remove (); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.Remove (); }, typeof (ObjectDisposedException), "Dispose");
		}

		[TestMethod]
		public void GetFilesInSubdirs ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			string pattern = Path.Combine ("..", "*");
			Assert.Throws<IsolatedStorageException> (() => isf.GetFileNames (pattern));

			isf.CreateDirectory ("test");
			isf.CreateFile ("test/file.txt");

			string [] names = isf.GetFileNames ("test/*");

			Assert.AreEqual (1, names.Length, "#a1");
			Assert.AreEqual ("file.txt", names [0], "#a2");

			isf.Remove ();
			Assert.Throws (delegate { isf.DirectoryExists ("does not exists"); }, typeof (IsolatedStorageException), "Remove");

			isf.Dispose ();
			Assert.Throws (delegate { isf.DirectoryExists ("does not exists"); }, typeof (ObjectDisposedException), "Dispose");
		}
	}
}
