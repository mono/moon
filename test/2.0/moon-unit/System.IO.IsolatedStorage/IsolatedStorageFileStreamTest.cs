//
// Unit tests for System.IO.IsolatedStorage.IsolatedStorageFileStream
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
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.IO.IsolatedStorage {

	[TestClass]
	public class IsolatedStorageFileStreamTest {

		[TestMethod]
		public void IsolatedStorageFileStream_BadValues ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.Throws (delegate { new IsolatedStorageFileStream (null, FileMode.Create, isf); },
				typeof (ArgumentNullException), "ctor(null,FileMode,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream (null, FileMode.Create, FileAccess.Read, isf); },
				typeof (ArgumentNullException), "ctor(null,FileMode,FileAccess,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream (null, FileMode.Create, FileAccess.Read, FileShare.None, isf); },
				typeof (ArgumentNullException), "ctor(null,FileMode,FileAccess,FileShare,isf)");

			Assert.Throws (delegate { new IsolatedStorageFileStream (String.Empty, FileMode.Create, isf); },
				typeof (ArgumentException), "ctor(empty,FileMode,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream (String.Empty, FileMode.Create, FileAccess.Read, isf); },
				typeof (ArgumentException), "ctor(empty,FileMode,FileAccess,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream (String.Empty, FileMode.Create, FileAccess.Read, FileShare.None, isf); },
				typeof (ArgumentException), "ctor(empty,FileMode,FileAccess,FileShare,isf)");

			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, null); },
				typeof (ArgumentNullException), "ctor(string,FileMode,null)");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, FileAccess.Read, null); },
				typeof (ArgumentNullException), "ctor(string,FileMode,FileAccess,null)");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, FileAccess.Read, FileShare.None, null); },
				typeof (ArgumentNullException), "ctor(string,FileMode,FileAccess,FileShare,null)");

			FileMode mode = (FileMode) Int32.MinValue;
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", mode, isf); },
				typeof (ArgumentException), "ctor(string,bad,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", mode, FileAccess.Read, isf); },
				typeof (ArgumentException), "ctor(string,bad,FileAccess,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", mode, FileAccess.Read, FileShare.None, isf); },
				typeof (ArgumentException), "ctor(string,bad,FileAccess,FileShare,isf)");

			FileAccess access = (FileAccess) Int32.MinValue;
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, access, isf); },
				typeof (IsolatedStorageException), "ctor(string,FileMode,bad,isf)");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, access, FileShare.None, isf); },
				typeof (IsolatedStorageException), "ctor(string,FileMode,bad,FileShare,isf)");

			FileShare share = (FileShare) Int32.MinValue;
			Assert.Throws (delegate { new IsolatedStorageFileStream ("moon", FileMode.Create, FileAccess.Read, share, isf); },
				typeof (IsolatedStorageException), "ctor(string,FileMode,FileAccess,bad,isf)");
		}

		[TestMethod]
		public void Create ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				Assert.IsTrue (fs.CanRead, "CanRead");
				Assert.IsTrue (fs.CanSeek, "CanSeek");
				Assert.IsTrue (fs.CanWrite, "CanWrite");
				Assert.AreEqual (0, fs.Length, "Length");
				Assert.AreEqual (0, fs.Position, "Position");

				fs.Dispose ();
				Assert.IsFalse (fs.CanRead, "Dispose/CanRead");
				Assert.IsFalse (fs.CanSeek, "Dispose/CanSeek");
				Assert.IsFalse (fs.CanWrite, "Dispose/CanWrite");
				Assert.Throws (delegate { Console.WriteLine (fs.Length); }, typeof (ObjectDisposedException), "Dispose/Length");
				Assert.Throws (delegate { Console.WriteLine (fs.Position); }, typeof (ObjectDisposedException), "Dispose/Position");
			}

			isf.Remove ();
			Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, isf); }, typeof (IsolatedStorageException), "Remove/new");

			isf.Dispose ();
			Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, isf); }, typeof (ObjectDisposedException), "Dispose/new");
		}

		[TestMethod]
		public void PlayingWithQuota ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			try {
				using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
					// funny (in a strange way) but this works (up to 1024 bytes)
					fs.SetLength (isf.AvailableFreeSpace + 1024);
					fs.SetLength (0); // reset
					Assert.Throws (delegate { fs.SetLength (isf.AvailableFreeSpace + 1025); }, typeof (IsolatedStorageException), ">1024");

					// this does not since AvailableFreeSpace < Quota (overhead?)
					Assert.Throws (delegate { fs.SetLength (isf.Quota); }, typeof (IsolatedStorageException), "OverQuota");
				}
			}
			finally {
				// ensure other tests can run properly
				isf.Remove ();
			}
		}
	}
}
