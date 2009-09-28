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
using System.Collections.Generic;
using System.IO;
using System.IO.IsolatedStorage;
using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

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
			// Fails in Silverlight 3
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

				fs.Dispose (); // multiple times (like using do to)

				isf.Remove ();
				Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, isf); }, typeof (IsolatedStorageException), "Remove/new");
				Assert.IsFalse (fs.CanRead, "Dispose/CanRead");
				Assert.IsFalse (fs.CanSeek, "Dispose/CanSeek");
				Assert.IsFalse (fs.CanWrite, "Dispose/CanWrite");
				Assert.Throws (delegate { Console.WriteLine (fs.Length); }, typeof (IsolatedStorageException), "Dispose/Length");
				Assert.Throws (delegate { Console.WriteLine (fs.Position); }, typeof (IsolatedStorageException), "Dispose/Position");

				isf.Dispose ();
				Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, isf); }, typeof (ObjectDisposedException), "Dispose/new");
				Assert.IsFalse (fs.CanRead, "Dispose/CanRead");
				Assert.IsFalse (fs.CanSeek, "Dispose/CanSeek");
				Assert.IsFalse (fs.CanWrite, "Dispose/CanWrite");
				Assert.Throws (delegate { Console.WriteLine (fs.Length); }, typeof (ObjectDisposedException), "Dispose/Length");
				Assert.Throws (delegate { Console.WriteLine (fs.Position); }, typeof (ObjectDisposedException), "Dispose/Position");
			}
		}

		[TestMethod]
		public void Create_RemovedIsolatedStorageFile ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			isf.Remove ();
			Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, isf); }, typeof (IsolatedStorageException), "Remove/new1");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, FileAccess.ReadWrite, isf); }, typeof (IsolatedStorageException), "Remove/new2");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("removed", FileMode.Create, FileAccess.ReadWrite, FileShare.Read, isf); }, typeof (IsolatedStorageException), "Remove/new3");
		}

		[TestMethod]
		public void Create_DisposedIsolatedStorageFile ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			isf.Dispose ();
			Assert.Throws (delegate { new IsolatedStorageFileStream ("disposed", FileMode.Create, isf); }, typeof (ObjectDisposedException), "Dispose/new1");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("disposed", FileMode.Create, FileAccess.ReadWrite, isf); }, typeof (ObjectDisposedException), "Dispose/new2");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("disposed", FileMode.Create, FileAccess.ReadWrite, FileShare.Read, isf); }, typeof (ObjectDisposedException), "Dispose/new3");
		}

		[TestMethod]
		public void Create_NotFound ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			// LAMESPEC: documented to be a FileNotFoundException
			Assert.Throws (delegate { new IsolatedStorageFileStream ("does-not-exist", FileMode.Open, isf); }, typeof (IsolatedStorageException), "FileNotFound1");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("does-not-exist", FileMode.Open, FileAccess.Read, isf); }, typeof (IsolatedStorageException), "FileNotFound2");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("does-not-exist", FileMode.Open, FileAccess.Read, FileShare.Read, isf); }, typeof (IsolatedStorageException), "FileNotFound3");
			// LAMESPEC: documented to be a DirectoryNotFoundException
			Assert.Throws (delegate { new IsolatedStorageFileStream ("dir-does-not-exist/new-file", FileMode.Create, isf); }, typeof (IsolatedStorageException), "DirectoryNotFound1");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("dir-does-not-exist/new-file", FileMode.Create, FileAccess.ReadWrite, isf); }, typeof (IsolatedStorageException), "DirectoryNotFound2");
			Assert.Throws (delegate { new IsolatedStorageFileStream ("dir-does-not-exist/new-file", FileMode.Create, FileAccess.ReadWrite, FileShare.Read, isf); }, typeof (IsolatedStorageException), "DirectoryNotFound3");
		}

		void EndRead (IAsyncResult result)
		{
			IsolatedStorageFileStream fs = (IsolatedStorageFileStream) result.AsyncState;
			Assert.AreEqual (0, fs.EndRead (result), "EndRead");
		}

		[TestMethod]
		public void AsyncRead ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2];
				IAsyncResult result = fs.BeginRead (data, 0, 2, new AsyncCallback (EndRead), fs);
				result.AsyncWaitHandle.WaitOne ();

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.BeginRead (data, 0, 2, new AsyncCallback (EndRead), fs); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				isf.Dispose ();
				Assert.Throws (delegate { fs.BeginRead (data, 0, 2, new AsyncCallback (EndRead), fs); }, typeof (ObjectDisposedException), "Dispose/Write");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		void EndReadRemoved (IAsyncResult result)
		{
			KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = (KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream>) result.AsyncState;
			kvp.Key.Remove ();
			Assert.Throws (delegate { kvp.Value.EndRead (result); }, typeof (IsolatedStorageException), "Remove/EndRead"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void AsyncEndRead_Removed ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2];
				KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = new KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> (isf, fs);
				IAsyncResult result = fs.BeginRead (data, 0, 2, new AsyncCallback (EndReadRemoved), kvp);
				result.AsyncWaitHandle.WaitOne ();
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		void EndReadDisposed (IAsyncResult result)
		{
			KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = (KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream>) result.AsyncState;
			kvp.Key.Dispose ();
			Assert.Throws (delegate { kvp.Value.EndRead (result); }, typeof (ObjectDisposedException), "Dispose/EndRead"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void AsyncEndRead_Disposed ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2];
				KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = new KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> (isf, fs);
				IAsyncResult result = fs.BeginRead (data, 0, 2, new AsyncCallback (EndReadDisposed), kvp);
				result.AsyncWaitHandle.WaitOne ();
			}
		}

		void EndWrite (IAsyncResult result)
		{
			IsolatedStorageFileStream fs = (IsolatedStorageFileStream) result.AsyncState;
			fs.EndWrite (result);
		}

		[TestMethod]
		public void AsyncWrite ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte[] data = new byte [2];
				IAsyncResult result = fs.BeginWrite (data, 0, 2, new AsyncCallback (EndWrite), fs);
				result.AsyncWaitHandle.WaitOne ();

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.BeginWrite (data, 0, 2, new AsyncCallback (EndWrite), fs); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				isf.Dispose ();
				Assert.Throws (delegate { fs.BeginWrite (data, 0, 2, new AsyncCallback (EndWrite), fs); }, typeof (ObjectDisposedException), "Dispose/Write");
			}

			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		void EndWriteRemoved (IAsyncResult result)
		{
			KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = (KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream>) result.AsyncState;
			kvp.Key.Remove ();
			Assert.Throws (delegate { kvp.Value.EndWrite (result); }, typeof (IsolatedStorageException), "Remove/EndWrite"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void AsyncEndWrite_Removed ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			isf.Remove ();

			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "pre-check");

			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2];
				KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = new KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> (isf, fs);
				IAsyncResult result = fs.BeginWrite (data, 0, 2, new AsyncCallback (EndWriteRemoved), kvp);
				result.AsyncWaitHandle.WaitOne ();
			}
		}

		void EndWriteDisposed (IAsyncResult result)
		{
			KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = (KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream>) result.AsyncState;
			kvp.Key.Dispose ();
			Assert.Throws (delegate { kvp.Value.EndWrite (result); }, typeof (ObjectDisposedException), "Dispose/EndWrite"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void AsyncEndWrite_Disposed ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2];
				KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> kvp = new KeyValuePair<IsolatedStorageFile, IsolatedStorageFileStream> (isf, fs);
				IAsyncResult result = fs.BeginWrite (data, 0, 2, new AsyncCallback (EndWriteDisposed), kvp);
				result.AsyncWaitHandle.WaitOne ();
			}
		}

		[TestMethod]
		public void Flush ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				fs.Flush ();

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.Flush (); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				isf.Dispose ();
				Assert.Throws (delegate { fs.Flush (); }, typeof (ObjectDisposedException), "Dispose/Write");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		[TestMethod]
		public void Seek ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				fs.Seek (0, SeekOrigin.Begin);

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.Seek (0, SeekOrigin.Begin); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				isf.Dispose ();
				Assert.Throws (delegate { fs.Seek (0, SeekOrigin.Begin); }, typeof (ObjectDisposedException), "Dispose/Write");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		[TestMethod]
		public void SetLength ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				fs.SetLength (1);

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.SetLength (1); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				isf.Dispose ();
				Assert.Throws (delegate { fs.SetLength (1); }, typeof (ObjectDisposedException), "Dispose/Write");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		[TestMethod]
		public void Write ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2] { 0x00, 0x01 };
				fs.Write (data, 0, 1);
				fs.WriteByte (0x01);
				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.Write (data, 1, 1); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				Assert.Throws (delegate { fs.WriteByte (0x0f); }, typeof (IsolatedStorageException), "Remove/WriteByte");
				isf.Dispose ();
				Assert.Throws (delegate { fs.Write (data, 1, 1); }, typeof (ObjectDisposedException), "Dispose/Write");
				Assert.Throws (delegate { fs.WriteByte (0xf0); }, typeof (ObjectDisposedException), "Dispose/WriteByte");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}


		[TestMethod]
		public void WriteThenRead ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("moon", FileMode.Create, isf)) {
				byte [] data = new byte [2] { 0x00, 0x01 };
				fs.Write (data, 0, 1);
				fs.WriteByte (0xff);
			}
			using (IsolatedStorageFileStream fs = isf.OpenFile ("moon", FileMode.Open)) {
				byte [] data = new byte [1];
				Assert.AreEqual (1, fs.Read (data, 0, 1), "1");
				Assert.AreEqual (0x00, data[0], "0x00");
				Assert.AreEqual (0xff, fs.ReadByte (), "0xff");

				isf.Remove (); // this removed everything
				Assert.Throws (delegate { fs.Read (data, 1, 1); }, typeof (IsolatedStorageException), "Remove/Write"); // Fails in Silverlight 3
				Assert.Throws (delegate { fs.ReadByte (); }, typeof (IsolatedStorageException), "Remove/WriteByte");
				isf.Dispose ();
				Assert.Throws (delegate { fs.Read (data, 1, 1); }, typeof (ObjectDisposedException), "Dispose/Write");
				Assert.Throws (delegate { fs.ReadByte (); }, typeof (ObjectDisposedException), "Dispose/WriteByte");
			}
			isf = IsolatedStorageFile.GetUserStoreForApplication ();
			Assert.AreEqual (0, isf.GetFileNames ().Length, "Empty");
		}

		[TestMethod]
		public void ReadOnly ()
		{
			IsolatedStorageFile isf = IsolatedStorageFile.GetUserStoreForApplication ();
			try {
				using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("read-only", FileMode.Create, FileAccess.Write, isf)) {
					fs.WriteByte (0);
				}
				// now we open it read-only
				using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream ("read-only", FileMode.Open, FileAccess.Read, isf)) {
					Assert.Throws (delegate { fs.WriteByte (0); }, typeof (NotSupportedException), "WriteByte");
					Assert.Throws (delegate { fs.Write (new byte [0], 0, 0); }, typeof (NotSupportedException), "Write");
					Assert.Throws (delegate { fs.BeginWrite (new byte [0], 0, 0, null, null); }, typeof (NotSupportedException), "BeginWrite");
				}
			}
			finally {
				isf.DeleteFile ("read-only");
			}
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
					// KnownFailure because Moonlight, right now, has AvailableFreeSpace = Quota - 1024 (Safety) while SL has some "extra" stuff (less space)
					// Assert.Throws (delegate { fs.SetLength (isf.Quota); }, typeof (IsolatedStorageException), "OverQuota");
				}
			}
			finally {
				// ensure other tests can run properly
				isf.Remove ();
			}
		}
	}
}
