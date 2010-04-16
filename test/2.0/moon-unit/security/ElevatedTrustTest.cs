//
// Elevated Trust Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009-2010 Novell, Inc.
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
using System.Text;
using System.Windows;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class ElevatedTrustTest {

		[TestMethod]
		public void Application_HasElevatedPermissions ()
		{
			Assert.IsFalse (Application.Current.HasElevatedPermissions, "HasElevatedPermissions");
		}

		[TestMethod]
		public void System_Environment_CurrentDirectory ()
		{
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Environment.CurrentDirectory);
			}, "get_CurrentDirectory");
		}

		[TestMethod]
		public void System_Environment_GetFolderPath ()
		{
			// some folders are accessible using elevated trust
			Assert.Throws<SecurityException> (delegate {
				Environment.GetFolderPath (Environment.SpecialFolder.MyMusic);
			}, "GetFolderPath");
			// but not all of them - but it all looks the same without elevated trust
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Environment.GetFolderPath (Environment.SpecialFolder.ApplicationData));
			}, "GetFolderPath");
		}

		[TestMethod]
		public void System_IO_Directory_CreateDirectory ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Directory.CreateDirectory (String.Empty));
			}, "CreateDirectory-empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.CreateDirectory ("a"));
			}, "CreateDirectory");
		}

		[TestMethod]
		public void System_IO_Directory_Delete ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Directory.Delete (String.Empty);
			}, "Delete(string)-empty");
			Assert.Throws<SecurityException> (delegate {
				Directory.Delete ("a");
			}, "Delete(string)");

			Assert.Throws<ArgumentException> (delegate {
				Directory.Delete (String.Empty, true);
			}, "Delete(string,bool)-empty");
			Assert.Throws<SecurityException> (delegate {
				Directory.Delete ("a", true);
			}, "Delete(string,bool)");
		}

		[TestMethod]
		public void System_IO_Directory_EnumerateDirectories ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateDirectories (String.Empty);
			}, "EnumerateDirectories(string)-empty");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateDirectories (String.Empty, String.Empty), "EnumerateDirectories(string,string)-empty");
			Assert.IsNotNull (Directory.EnumerateDirectories (String.Empty, String.Empty, SearchOption.AllDirectories), "EnumerateDirectories(string,string,SearchOption)-empty");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateDirectories (String.Empty, "*.dll");
			}, "EnumerateDirectories(string,string)-empty/*.dll");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateDirectories (String.Empty, "*.dll", SearchOption.AllDirectories);
			}, "EnumerateDirectories(string,string,SearchOption)-empty/*.dll");

			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateDirectories ("a");
			}, "EnumerateDirectories(string)");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateDirectories ("a", String.Empty), "EnumerateDirectories(string,string)");
			Assert.IsNotNull (Directory.EnumerateDirectories ("a", String.Empty, SearchOption.AllDirectories), "EnumerateDirectories(string,string,SearchOption)");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateDirectories ("a", "*.dll");
			}, "EnumerateDirectories(string,string)-*.dll");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateDirectories ("a", "*.dll", SearchOption.AllDirectories);
			}, "EnumerateDirectories(string,string,SearchOption)-*.dll");
		}

		[TestMethod]
		public void System_IO_Directory_EnumerateFiles ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFiles (String.Empty);
			}, "EnumerateFiles(string)-empty");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateFiles (String.Empty, String.Empty), "EnumerateFiles(string,string)-empty");
			Assert.IsNotNull (Directory.EnumerateFiles (String.Empty, String.Empty, SearchOption.AllDirectories), "EnumerateFiles(string,string,SearchOption)-empty");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFiles (String.Empty, "*.dll");
			}, "EnumerateFiles(string,string)-empty/*.dll");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFiles (String.Empty, "*.dll", SearchOption.AllDirectories);
			}, "EnumerateFiles(string,string,SearchOption)-empty/*.dll");

			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFiles ("a");
			}, "EnumerateFiles(string)");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateFiles ("a", String.Empty), "EnumerateFiles(string,string)");
			Assert.IsNotNull (Directory.EnumerateFiles ("a", String.Empty, SearchOption.AllDirectories), "EnumerateFiles(string,string,SearchOption)");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFiles ("a", "*.dll");
			}, "EnumerateFiles(string,string)-*.dll");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFiles ("a", "*.dll", SearchOption.AllDirectories);
			}, "EnumerateFiles(string,string,SearchOption)-*.dll");
		}

		[TestMethod]
		public void System_IO_Directory_EnumerateFileSystemEntries ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFileSystemEntries (String.Empty);
			}, "EnumerateFiles(string)-empty");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateFileSystemEntries (String.Empty, String.Empty), "EnumerateFiles(string,string)-empty");
			Assert.IsNotNull (Directory.EnumerateFileSystemEntries (String.Empty, String.Empty, SearchOption.AllDirectories), "EnumerateFiles(string,string,SearchOption)-empty");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFileSystemEntries (String.Empty, "*.dll");
			}, "EnumerateFiles(string,string)-empty/*.dll");
			Assert.Throws<ArgumentException> (delegate {
				Directory.EnumerateFileSystemEntries (String.Empty, "*.dll", SearchOption.AllDirectories);
			}, "EnumerateFiles(string,string,SearchOption)-empty/*.dll");

			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFileSystemEntries ("a");
			}, "EnumerateFiles(string)");
			// no exception here - empty enumeration
			Assert.IsNotNull (Directory.EnumerateFileSystemEntries ("a", String.Empty), "EnumerateFiles(string,string)");
			Assert.IsNotNull (Directory.EnumerateFileSystemEntries ("a", String.Empty, SearchOption.AllDirectories), "EnumerateFiles(string,string,SearchOption)");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFileSystemEntries ("a", "*.dll");
			}, "EnumerateFiles(string,string)-*.dll");
			Assert.Throws<SecurityException> (delegate {
				Directory.EnumerateFileSystemEntries ("a", "*.dll", SearchOption.AllDirectories);
			}, "EnumerateFiles(string,string,SearchOption)-*.dll");
		}

		[TestMethod]
		public void System_IO_Directory_Exists ()
		{
			// looks like nothing exists without elevated trust ;-)
			Assert.IsFalse (Directory.Exists (String.Empty), "empty");
			Assert.IsFalse (Directory.Exists ("/"), "unix");
			Assert.IsFalse (Directory.Exists ("c:\\"), "windows");
		}

		[TestMethod]
		public void System_IO_Directory_GetCreationTime ()
		{
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Directory.GetCreationTime (String.Empty));
			}, "GetCreationTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.GetCreationTime ("/"));
			}, "GetCreationTime");
		}

		[TestMethod]
		public void System_IO_Directory_GetCurrentDirectory ()
		{
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.GetCurrentDirectory ());
			}, "GetCurrentDirectory");
		}

		[TestMethod]
		public void System_IO_Directory_GetDirectoryRoot ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Directory.GetDirectoryRoot (String.Empty));
			}, "GetDirectoryRoot");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.GetDirectoryRoot ("a/b"));
			}, "GetDirectoryRoot");
		}

		[TestMethod]
		public void System_IO_Directory_GetLastAccessTime ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Directory.GetLastAccessTime (String.Empty));
			}, "GetLastAccessTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.GetLastAccessTime ("/"));
			}, "GetLastAccessTime");
		}

		[TestMethod]
		public void System_IO_Directory_GetLastWriteTime ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Directory.GetLastWriteTime (String.Empty));
			}, "GetLastWriteTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Directory.GetLastWriteTime ("/"));
			}, "GetLastWriteTime");
		}

		[TestMethod]
		public void System_IO_Directory_Move ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Directory.Move (String.Empty, String.Empty);
			}, "Move-empty");
			Assert.Throws<SecurityException> (delegate {
				Directory.Move ("a", "b");
			}, "Move");
		}

		// System_IO_Directory_SetCurrentDirectory is in SecurityCriticalTest

		// note: I could not find an API that returns a usable DirectoryInfo instance
		// so we cannot test the members to see how they deal without elevated trust.
		// For safety we'll assume that all members marked as [SecuritySafeCritical]
		// are in fact only available from elevated trust. That should not affect
		// application code unless something returns a DirectoryInfo from non-elevated
		// trust

		[TestMethod]
		public void System_IO_DirectoryInfo_ctor ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				new DirectoryInfo (String.Empty);
			}, "ctor-empty");
			Assert.Throws<SecurityException> (delegate {
				new DirectoryInfo ("a");
			}, "ctor");
		}

		[TestMethod]
		public void System_IO_File_AppendAllLines ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.AppendAllLines (String.Empty, new string[0]);
			}, "AppendText(string,IEnumerable)-empty");
			Assert.Throws<ArgumentException> (delegate {
				File.AppendAllLines (String.Empty, new string [0], Encoding.UTF8);
			}, "AppendText(string,IEnumerable,Encoding)-empty");

			Assert.Throws<SecurityException> (delegate {
				File.AppendAllLines ("a", new string [0]);
			}, "AppendText(string,IEnumerable)");
			Assert.Throws<SecurityException> (delegate {
				File.AppendAllLines ("a", new string [0], Encoding.UTF8);
			}, "AppendText(string,IEnumerable,Encoding)");
		}

		[TestMethod]
		public void System_IO_File_AppendAllText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.AppendAllText (String.Empty, String.Empty);
			}, "AppendText(string,IEnumerable)-empty");
			Assert.Throws<ArgumentException> (delegate {
				File.AppendAllText (String.Empty, String.Empty, Encoding.UTF8);
			}, "AppendText(string,IEnumerable,Encoding)-empty");

			Assert.Throws<SecurityException> (delegate {
				File.AppendAllText ("a", String.Empty);
			}, "AppendText(string,IEnumerable)");
			Assert.Throws<SecurityException> (delegate {
				File.AppendAllText ("a", String.Empty, Encoding.UTF8);
			}, "AppendText(string,IEnumerable,Encoding)");
		}

		[TestMethod]
		public void System_IO_File_AppendText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.AppendText (String.Empty));
			}, "AppendText(string)-empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.AppendText ("a"));
			}, "AppendText(string)");
		}

		[TestMethod]
		public void System_IO_File_Copy ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.Copy (String.Empty, String.Empty);
			}, "Copy(string,string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				File.Copy (String.Empty, String.Empty, false);
			}, "Copy(string,string,bool)-Empty");

			Assert.Throws<SecurityException> (delegate {
				File.Copy ("a", "b");
			}, "Copy(string,string)");
			Assert.Throws<SecurityException> (delegate {
				File.Copy ("a", "b", false);
			}, "Copy(string,string,bool)");
		}

		[TestMethod]
		public void System_IO_File_Create ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.Create (String.Empty));
			}, "Create(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.Create (String.Empty, 0));
			}, "Create(string,int)-Empty");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				Assert.IsNotNull (File.Create ("a", 0));
			}, "Create(string,int)-0");

			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.Create ("a"));
			}, "Create(string)");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.Create ("a", 1024));
			}, "Create(string,int)");
		}

		[TestMethod]
		public void System_IO_File_CreateText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.CreateText (String.Empty));
			}, "CreateText(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.CreateText ("a"));
			}, "CreateText(string)");
		}

		[TestMethod]
		public void System_IO_File_Delete ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.Delete (String.Empty);
			}, "Delete(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				File.Delete ("a");
			}, "Delete(string)");
		}

		[TestMethod]
		public void System_IO_File_Exists ()
		{
			// looks like nothing exists without elevated trust ;-)
			Assert.IsFalse (File.Exists (String.Empty));
			Assert.IsFalse (File.Exists ("a"));
		}

		[TestMethod]
		public void System_IO_File_GetAttributes ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.GetAttributes (String.Empty));
			}, "GetAttributes-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.GetAttributes ("a"));
			}, "GetAttributes");
		}

		[TestMethod]
		public void System_IO_File_GetCreationTime ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.GetCreationTime (String.Empty));
			}, "GetCreationTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.GetCreationTime ("a"));
			}, "GetCreationTime");
		}

		[TestMethod]
		public void System_IO_File_GetLastAccessTime ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.GetLastAccessTime (String.Empty));
			}, "GetLastAccessTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.GetLastAccessTime ("a"));
			}, "GetLastAccessTime");
		}

		[TestMethod]
		public void System_IO_File_GetLastWriteTime ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.GetLastWriteTime (String.Empty));
			}, "GetLastWriteTime-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.GetLastWriteTime ("a"));
			}, "GetLastWriteTime");
		}

		[TestMethod]
		public void System_IO_File_Move ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.Move (String.Empty, String.Empty);
			}, "Move(string,string-Empty)");
			Assert.Throws<SecurityException> (delegate {
				File.Move ("a", "b");
			}, "Move(string,string)");
		}

		[TestMethod]
		public void System_IO_File_Open ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open));
			}, "Open(string,FileMode)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open, FileAccess.Read));
			}, "Open(string,FileMode,FileAccess)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None));
			}, "Open(string,FileMode,FileAccess,FileShare)-Empty");

			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.Open ("a", FileMode.Open));
			}, "Open(string,FileMode)");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.Open ("a", FileMode.Open, FileAccess.Read));
			}, "Open(string,FileMode,FileAccess)");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.Open ("a", FileMode.Open, FileAccess.Read, FileShare.None));
			}, "Open(string,FileMode,FileAccess,FileShare)");
		}

		[TestMethod]
		public void System_IO_File_OpenRead ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.OpenRead (String.Empty));
			}, "OpenRead(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.OpenRead ("a"));
			}, "OpenRead(string)");
		}

		[TestMethod]
		public void System_IO_File_OpenText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.OpenText (String.Empty));
			}, "OpenText(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.OpenText ("a"));
			}, "OpenText(string)");
		}

		[TestMethod]
		public void System_IO_File_OpenWrite ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.OpenWrite (String.Empty));
			}, "OpenWrite(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.OpenWrite ("a"));
			}, "OpenWrite(string)");
		}

		[TestMethod]
		public void System_IO_File_ReadAllBytes ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.ReadAllBytes (String.Empty));
			}, "ReadAllBytes(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.ReadAllBytes ("a"));
			}, "ReadAllBytes(string)");
		}

		[TestMethod]
		public void System_IO_File_ReadAllText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.ReadAllText (String.Empty));
			}, "ReadAllText(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.ReadAllText (String.Empty, Encoding.UTF8));
			}, "ReadAllText(string,Encoding)-Empty");

			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.ReadAllText ("a"));
			}, "ReadAllText(string)");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.ReadAllText ("a", Encoding.UTF8));
			}, "ReadAllText(string,Encoding)");
		}

		[TestMethod]
		public void System_IO_File_ReadLines ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.ReadLines (String.Empty));
			}, "ReadLines(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (File.ReadLines (String.Empty, Encoding.UTF8));
			}, "ReadLines(string,Encoding)-Empty");

			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.ReadLines ("a"));
			}, "ReadLines(string)");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (File.ReadLines ("a", Encoding.UTF8));
			}, "ReadLines(string,Encoding)");
		}

		[TestMethod]
		public void System_IO_File_WriteAllBytes ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.WriteAllBytes (String.Empty, new byte[0]);
			}, "WriteAllBytes(string)-Empty");
			Assert.Throws<SecurityException> (delegate {
				File.WriteAllBytes ("a", new byte [0]);
			}, "WriteAllBytes(string)");
		}

		[TestMethod]
		public void System_IO_File_WriteAllLines ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.WriteAllLines (String.Empty, new string [0]);
			}, "WriteAllLines(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				File.WriteAllLines (String.Empty, new string [0], Encoding.UTF8);
			}, "WriteAllLines(string,Encoding)-Empty");

			Assert.Throws<SecurityException> (delegate {
				File.WriteAllLines ("a", new string [0]);
			}, "WriteAllLines(string)");
			Assert.Throws<SecurityException> (delegate {
				File.WriteAllLines ("a", new string [0], Encoding.UTF8);
			}, "WriteAllLines(string,Encoding)");
		}

		[TestMethod]
		public void System_IO_File_WriteAllText ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				File.WriteAllText (String.Empty, String.Empty);
			}, "WriteAllText(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				File.WriteAllText (String.Empty, String.Empty, Encoding.UTF8);
			}, "WriteAllText(string,Encoding)-Empty");

			Assert.Throws<SecurityException> (delegate {
				File.WriteAllText ("a", String.Empty);
			}, "WriteAllText(string)");
			Assert.Throws<SecurityException> (delegate {
				File.WriteAllText ("a", String.Empty, Encoding.UTF8);
			}, "WriteAllText(string,Encoding)");
		}

		[TestMethod]
		public void System_IO_FileInfo_ctor ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				new FileInfo (String.Empty);
			}, "ctor-Empty");
			Assert.Throws<SecurityException> (delegate {
				new FileInfo ("a");
			}, "ctor");
		}

		[TestMethod]
		public void System_IO_FileStream_ctor ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				new FileStream (String.Empty, FileMode.Open);
			}, ".ctor(string,FileMode)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read);
			}, ".ctor(string,FileMode,FileAccess)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None);
			}, ".ctor(string,FileMode,FileAccess,FileShare)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None, 1024);
			}, ".ctor(string,FileMode,FileAccess,FileShare,int)-Empty");

			Assert.Throws<SecurityException> (delegate {
				new FileStream ("a", FileMode.Open);
			}, ".ctor(string,FileMode)");
			Assert.Throws<SecurityException> (delegate {
				new FileStream ("a", FileMode.Open, FileAccess.Read);
			}, ".ctor(string,FileMode,FileAccess)");
			Assert.Throws<SecurityException> (delegate {
				new FileStream ("a", FileMode.Open, FileAccess.Read, FileShare.None);
			}, ".ctor(string,FileMode,FileAccess,FileShare)");
			Assert.Throws<SecurityException> (delegate {
				new FileStream ("a", FileMode.Open, FileAccess.Read, FileShare.None, 1024);
			}, ".ctor(string,FileMode,FileAccess,FileShare,int)");
		}

		[TestMethod]
		public void System_IO_FileStream_Name ()
		{
			using (FileStream fs = new IsolatedStorageFileStream ("a", FileMode.OpenOrCreate, IsolatedStorageFile.GetUserStoreForApplication ())) {
				Assert.AreEqual ("[Unknown]", fs.Name, "Name");
			}
		}

		[TestMethod]
		public void System_IO_Path_GetFullPath ()
		{
			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				Assert.IsNotNull (Path.GetFullPath (String.Empty));
			}, "GetFullPath-Empty");
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Path.GetFullPath ("a"));
			}, "GetFullPath");
		}

		[TestMethod]
		public void System_IO_Path_GetTempFileName ()
		{
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Path.GetTempFileName ());
			}, "GetTempFileName");
		}

		[TestMethod]
		public void System_IO_Path_GetTempPath ()
		{
			Assert.Throws<SecurityException> (delegate {
				Assert.IsNotNull (Path.GetTempPath ());
			}, "GetTempPath");
		}

		[TestMethod]
		public void System_IO_StreamReader_ctor ()
		{
			Assert.IsNotNull (new StreamReader (Stream.Null));
			Assert.IsNotNull (new StreamReader (Stream.Null, true));
			Assert.IsNotNull (new StreamReader (Stream.Null, Encoding.UTF8));
			Assert.IsNotNull (new StreamReader (Stream.Null, Encoding.BigEndianUnicode, false));
			Assert.IsNotNull (new StreamReader (Stream.Null, Encoding.Unicode, false, 1024));

			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				new StreamReader (String.Empty);
			}, ".ctor(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamReader (String.Empty, false);
			}, ".ctor(string,bool)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamReader (String.Empty, Encoding.Unicode);
			}, ".ctor(string,Encoding)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamReader (String.Empty, Encoding.UTF8, true);
			}, ".ctor(string,Encoding,bool)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamReader (String.Empty, Encoding.BigEndianUnicode, true, -1);
			}, ".ctor(string,Encoding,bool)-Empty");

			Assert.Throws<SecurityException> (delegate {
				new StreamReader ("a");
			}, ".ctor(string)");
			Assert.Throws<SecurityException> (delegate {
				new StreamReader ("a", false);
			}, ".ctor(string,bool)");
			Assert.Throws<SecurityException> (delegate {
				new StreamReader ("a", Encoding.Unicode);
			}, ".ctor(string,Encoding)");
			Assert.Throws<SecurityException> (delegate {
				new StreamReader ("a", Encoding.UTF8, true);
			}, ".ctor(string,Encoding,bool)");
			Assert.Throws<SecurityException> (delegate {
				new StreamReader ("a", Encoding.BigEndianUnicode, true, 1024);
			}, ".ctor(string,Encoding,bool)");
		}

		[TestMethod]
		public void System_IO_StreamWriter_ctor ()
		{
			Assert.IsNotNull (new StreamWriter (Stream.Null));
			Assert.IsNotNull (new StreamWriter (Stream.Null, Encoding.UTF8));
			Assert.IsNotNull (new StreamWriter (Stream.Null, Encoding.Unicode, 1024));

			// note: security check is not the first one
			Assert.Throws<ArgumentException> (delegate {
				new StreamWriter (String.Empty);
			}, ".ctor(string)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamWriter (String.Empty, false);
			}, ".ctor(string,bool)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamWriter (String.Empty, true, Encoding.Unicode);
			}, ".ctor(string,bool,Encoding)-Empty");
			Assert.Throws<ArgumentException> (delegate {
				new StreamWriter (String.Empty, false, Encoding.UTF8, 1024);
			}, ".ctor(string,bool,Encoding,int)-Empty");

			Assert.Throws<SecurityException> (delegate {
				new StreamWriter ("a");
			}, ".ctor(string)");
			Assert.Throws<SecurityException> (delegate {
				new StreamWriter ("a", false);
			}, ".ctor(string,bool)");
			Assert.Throws<SecurityException> (delegate {
				new StreamWriter ("a", true, Encoding.Unicode);
			}, ".ctor(string,bool,Encoding)");
			Assert.Throws<SecurityException> (delegate {
				new StreamWriter ("a", false, Encoding.UTF8, 1024);
			}, ".ctor(string,bool,Encoding,int)");
		}
	}
}

