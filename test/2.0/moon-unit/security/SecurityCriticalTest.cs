//
// SecurityCritical Unit Tests on visible API
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Diagnostics;
using System.IO;
using System.IO.IsolatedStorage;
using System.Reflection;
using System.Reflection.Emit;
using System.Resources;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Markup;
using System.Text;
using System.Threading;

using Microsoft.Win32.SafeHandles;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class SecurityCriticalTest {

		// mscorlib.dll

		// note: There's no need to have unit tests for [SecurityCritical] code
		// when parameters using pointers are used since application code won't 
		// be able to call it because it lack support for unsafe blocks.

		class ConcreteCriticalHandleMinusOneIsInvalid : CriticalHandleMinusOneIsInvalid {

			public ConcreteCriticalHandleMinusOneIsInvalid ()
			{
			}

			protected override bool ReleaseHandle ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void Microsoft_Win32_SafeHandles_CriticalHandleMinusOneIsInvalid ()
		{
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteCriticalHandleMinusOneIsInvalid ();
			}, "ctor");
		}

		class ConcreteSafeHandleMinusOneIsInvalid : SafeHandleMinusOneIsInvalid {

			public ConcreteSafeHandleMinusOneIsInvalid ()
				: base (true)
			{
			}

			protected override bool ReleaseHandle ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void Microsoft_Win32_SafeHandles_SafeHandleMinusOneIsInvalid ()
		{
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteSafeHandleMinusOneIsInvalid ();
			}, "ctor");
		}

		class ConcreteSafeHandleZeroOrMinusOneIsInvalid : SafeHandleZeroOrMinusOneIsInvalid {

			public ConcreteSafeHandleZeroOrMinusOneIsInvalid ()
				: base (true)
			{
			}

			protected override bool ReleaseHandle ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void Microsoft_Win32_SafeHandles_SafeHandleZeroOrMinusOneIsInvalid ()
		{
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteSafeHandleZeroOrMinusOneIsInvalid ();
			}, "ctor");
		}

		[TestMethod]
		public void Microsoft_Win32_SafeHandles_SafeWaitHandle ()
		{
			// whole type is decorated with [SecurityCritical]
			Assert.Throws<MethodAccessException> (delegate {
				new SafeWaitHandle (IntPtr.Zero, true);
			}, "ctor");
			// note: SafeWaitHandle seems only (directly) exposed by
			// System.Threading.WaitHandle.SafeWaitHandle property where
			// both getter and setter are [SecurityCritical]
		}

		[TestMethod]
		public void System_AppDomain_AssemblyResolve ()
		{
			// ResolveEventHandler is [SecurityCritical] too so we avoid to use it here
			// since it would throw before AssemblyResolve was used
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.AssemblyResolve += null;
			}, "add_AssemblyResolve");
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.AssemblyResolve -= null;
			}, "remove_AssemblyResolve");
		}

		[TestMethod]
		public void System_AppDomain_DomainManager ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (AppDomain.CurrentDomain.DomainManager);
			}, "get_DomainManager");
		}

		[TestMethod]
		public void System_AppDomain_ExecuteAssemblyByName ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.ExecuteAssemblyByName (String.Empty);
			}, "ExecuteAssemblyByName");
		}

		[TestMethod]
		public void System_AppDomain_GetData ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.GetData (String.Empty);
			}, "GetData");
		}

		[TestMethod]
		public void System_AppDomain_SetData ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.SetData (String.Empty, null);
			}, "SetData");
		}

		[TestMethod]
		public void System_AppDomain_UnhandledException ()
		{
			// UnhandledExceptionEventHandler is [SecurityCritical] too so we avoid to use it here
			// since it would throw before AssemblyResolve was used
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.UnhandledException += null;
			}, "add_UnhandledException");
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.UnhandledException -= null;
			}, "remove_UnhandledException");
		}

		[TestMethod]
		public void System_AppDomainManager ()
		{
			// whole type is decorated with [SecurityCritical]
			Assert.Throws<MethodAccessException> (delegate {
				new AppDomainManager ();
			}, "ctor");
			// note: AppDomainManager seems only (directly) exposed by
			// System.AppDomain.DomainManager() property where
			// the getter (no setter exists) is [SecurityCritical]
		}

		[TestMethod]
		public void System_AppDomainSetup_ApplicationBase ()
		{
			AppDomainSetup ads = new AppDomainSetup ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (ads.ApplicationBase);
			}, "get_ApplicationBase");
			Assert.Throws<MethodAccessException> (delegate {
				ads.ApplicationBase = String.Empty;
			}, "set_ApplicationBase");
		}

		[TestMethod]
		public void System_Console_SetError ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Console.SetError (null);
			}, "SetError");
		}

		[TestMethod]
		public void System_Console_SetIn ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Console.SetIn (null);
			}, "SetIn");
		}

		[TestMethod]
		public void System_Console_SetOut ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Console.SetOut (null);
			}, "SetOut");
		}

		[TestMethod]
		public void System_Diagnostics_StackFrame_ctor ()
		{
			Assert.IsNotNull (new StackFrame (), ".ctor()");
			Assert.IsNotNull (new StackFrame (1), ".ctor(int)");
			Assert.IsNotNull (new StackFrame (String.Empty, 1), ".ctor(string,int)");
			Assert.IsNotNull (new StackFrame (String.Empty, 1, 2), ".ctor(string,int,int)");

			Assert.Throws<MethodAccessException> (delegate {
				new StackFrame (false);
			}, ".ctor(bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackFrame (1, false);
			}, ".ctor(int,bool)");
		}

		[TestMethod]
		public void System_Diagnostics_StackFrame_GetFileName ()
		{
			StackFrame sf = new StackFrame ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (sf.GetFileName ());
			}, "GetFileName");
		}

		[TestMethod]
		public void System_Diagnostics_StackTrace_ctor ()
		{
			Assert.IsNotNull (new StackTrace (), ".ctor()");
			Assert.IsNotNull (new StackTrace (new Exception ()), ".ctor(Exception)");
			Assert.IsNotNull (new StackTrace (new StackFrame ()), ".ctor(StackFrame)");

			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (false);
			}, ".ctor(bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (new Exception (), false);
			}, ".ctor(Exception,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (new Exception (), -1);
			}, ".ctor(Exception,int)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (new Exception (), 1, false);
			}, ".ctor(Exception,int,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (1);
			}, ".ctor(int)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (1, false);
			}, ".ctor(int,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StackTrace (Thread.CurrentThread, false);
			}, ".ctor(Thread,bool)");
		}

		[TestMethod]
		public void System_Environment_CurrentDirectory ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Environment.CurrentDirectory);
			}, "get_CurrentDirectory");
			Assert.Throws<MethodAccessException> (delegate {
				Environment.CurrentDirectory = String.Empty;
			}, "set_CurrentDirectory");
		}

		[TestMethod]
		public void System_Environment_GetFolderPath ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Environment.GetFolderPath (Environment.SpecialFolder.ApplicationData));
			}, "GetFolderPath");
		}

		[TestMethod]
		public void System_Environment_ProcessorCount ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Environment.ProcessorCount);
			}, "get_ProcessorCount");
		}

		[TestMethod]
		public void System_IO_Directory_CreateDirectory ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.CreateDirectory (String.Empty));
			}, "CreateDirectory");
		}

		[TestMethod]
		public void System_IO_Directory_Delete ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Directory.Delete (String.Empty);
			}, "Delete(string)");
			Assert.Throws<MethodAccessException> (delegate {
				Directory.Delete (String.Empty, true);
			}, "Delete(string,bool)");
		}

		[TestMethod]
		public void System_IO_Directory_Exists ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsFalse (Directory.Exists ("/"));
			}, "Exists");
		}

		[TestMethod]
		public void System_IO_Directory_GetCurrentDirectory ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetCurrentDirectory ());
			}, "GetCurrentDirectory");
		}

		[TestMethod]
		public void System_IO_Directory_GetCreationTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetCreationTime ("/"));
			}, "GetCreationTime");
		}

		[TestMethod]
		public void System_IO_Directory_GetDirectoryRoot ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetDirectoryRoot (String.Empty));
			}, "GetDirectoryRoot");
		}

		[TestMethod]
		public void System_IO_Directory_GetDirectories ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetDirectories (String.Empty));
			}, "GetDirectories(string)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetDirectories (String.Empty, String.Empty));
			}, "GetDirectories(string,string)");
		}

		[TestMethod]
		public void System_IO_Directory_GetFiles ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetFiles (String.Empty));
			}, "GetFiles(string)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetFiles (String.Empty, String.Empty));
			}, "GetFiles(string,string)");
		}

		[TestMethod]
		public void System_IO_Directory_GetFileSystemEntries ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetFileSystemEntries (String.Empty));
			}, "GetFileSystemEntries(string)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetFileSystemEntries (String.Empty, String.Empty));
			}, "GetFileSystemEntries(string,string)");
		}

		[TestMethod]
		public void System_IO_Directory_GetLastAccessTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetLastAccessTime ("/"));
			}, "GetLastAccessTime");
		}

		[TestMethod]
		public void System_IO_Directory_GetLastWriteTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Directory.GetLastWriteTime ("/"));
			}, "GetLastWriteTime");
		}

		[TestMethod]
		public void System_IO_Directory_Move ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Directory.Move (String.Empty, String.Empty);
			}, "Move");
		}

		[TestMethod]
		public void System_IO_Directory_SetCurrentDirectory ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Directory.SetCurrentDirectory (String.Empty);
			}, "SetCurrentDirectory");
		}

		// note: I could not find an API that returns a usable DirectoryInfo instance
		// so it's unclear why the DirectoryInfo type itself was not made [SecurityCritical]
		// Every visible API is [SecurityCritical] except for:
		// - get_Name property
		// - get_Exists property
		// - ToString()

		[TestMethod]
		public void System_IO_DirectoryInfo_ctor ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				new DirectoryInfo (String.Empty);
			}, "ctor");
		}

		[TestMethod]
		public void System_IO_File_AppendText ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.AppendText (String.Empty));
			}, "AppendText(string)");
		}

		[TestMethod]
		public void System_IO_File_Copy ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				File.Copy (String.Empty, String.Empty);
			}, "Copy(string,string)");
			Assert.Throws<MethodAccessException> (delegate {
				File.Copy (String.Empty, String.Empty, false);
			}, "Copy(string,string,bool)");
		}

		[TestMethod]
		public void System_IO_File_Create ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.Create (String.Empty));
			}, "Create(string)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.Create (String.Empty, 0));
			}, "Create(string,int)");
		}

		[TestMethod]
		public void System_IO_File_CreateText ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.CreateText (String.Empty));
			}, "CreateText(string)");
		}

		[TestMethod]
		public void System_IO_File_Delete ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				File.Delete (String.Empty);
			}, "Delete(string)");
		}

		[TestMethod]
		public void System_IO_File_Exists ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsFalse (File.Exists (String.Empty));
			}, "Exists");
		}

		[TestMethod]
		public void System_IO_File_GetCreationTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.GetCreationTime (String.Empty));
			}, "GetCreationTime");
		}

		[TestMethod]
		public void System_IO_File_GetLastAccessTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.GetLastAccessTime (String.Empty));
			}, "GetLastAccessTime");
		}

		[TestMethod]
		public void System_IO_File_GetLastWriteTime ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.GetLastWriteTime (String.Empty));
			}, "GetLastWriteTime");
		}

		[TestMethod]
		public void System_IO_File_Move ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				File.Move (String.Empty, String.Empty);
			}, "Move(string,string)");
		}

		[TestMethod]
		public void System_IO_File_Open ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open));
			}, "Open(string,FileMode)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open, FileAccess.Read));
			}, "Open(string,FileMode,FileAccess)");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.Open (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None));
			}, "Open(string,FileMode,FileAccess,FileShare)");
		}

		[TestMethod]
		public void System_IO_File_OpenRead ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.OpenRead (String.Empty));
			}, "OpenRead(string)");
		}

		[TestMethod]
		public void System_IO_File_OpenText ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.OpenText (String.Empty));
			}, "OpenText(string)");
		}

		[TestMethod]
		public void System_IO_File_OpenWrite ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (File.OpenWrite (String.Empty));
			}, "OpenWrite(string)");
		}

		[TestMethod]
		public void System_IO_File_SetAttributes ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				File.SetAttributes (String.Empty, FileAttributes.Archive);
			}, "SetAttributes");
		}

		[TestMethod]
		public void System_IO_FileStream_ctor ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				new FileStream (String.Empty, FileMode.Open);
			}, ".ctor(string,FileMode)");
			Assert.Throws<MethodAccessException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read);
			}, ".ctor(string,FileMode,FileAccess)");
			Assert.Throws<MethodAccessException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None);
			}, ".ctor(string,FileMode,FileAccess,FileShare)");
			Assert.Throws<MethodAccessException> (delegate {
				new FileStream (String.Empty, FileMode.Open, FileAccess.Read, FileShare.None, -1);
			}, ".ctor(string,FileMode,FileAccess,FileShare,int)");
		}

		[TestMethod]
		public void System_IO_FileStream_Name ()
		{
			using (FileStream fs = new IsolatedStorageFileStream ("a", FileMode.OpenOrCreate, IsolatedStorageFile.GetUserStoreForApplication ())) {
				Assert.Throws<MethodAccessException> (delegate {
					Assert.IsNotNull (fs.Name);
				}, "get_Name");
			}
		}

		// note: the only way to get FileInfo instances are from OpenFileDialog.
		// Sadly it's not easy to test since it requires an GUI intervention

		[TestMethod]
		public void System_IO_FileInfo_ctor ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				new FileInfo (String.Empty);
			}, "ctor");
		}

		// don't ask the user many times for a FileInfo
		static FileInfo fi_cache;

		private FileInfo GetFileInfo ()
		{
			if (fi_cache == null) {
				OpenFileDialog ofd = new OpenFileDialog ();
				ofd.ShowDialog ();
				fi_cache = ofd.File;
			}
			return fi_cache;
		}

		[Ignore ("blocking UI")] // comment to test locally
		[TestMethod]
		public void System_IO_FileInfo_UI ()
		{
			FileInfo fi = GetFileInfo ();

			// Transparent (or Safe Critical)

			using (FileStream fs = fi.OpenRead ()) {
				Assert.IsNotNull (fs, "OpenRead");
			}

			using (StreamReader sr = fi.OpenText ()) {
				Assert.IsNotNull (sr, "OpenText");
			}

			// ToString() returns Name and includes no path information
			Assert.AreEqual (String.Empty, Path.GetDirectoryName (fi.ToString ()), "ToString");

			// [SecurityCritical]

			Assert.Throws<MethodAccessException> (delegate {
				fi.AppendText ();
			}, "AppendText");

			Assert.Throws<MethodAccessException> (delegate {
				fi.CopyTo (String.Empty);
			}, "CopyTo(string)");
			Assert.Throws<MethodAccessException> (delegate {
				fi.CopyTo (String.Empty, false);
			}, "CopyTo(string,bool)");

			Assert.Throws<MethodAccessException> (delegate {
				fi.Create ();
			}, "Create");

			Assert.Throws<MethodAccessException> (delegate {
				fi.CreateText ();
			}, "CreateText");

			Assert.Throws<MethodAccessException> (delegate {
				fi.Delete ();
			}, "Delete");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fi.Directory);
			}, "get_Directory");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fi.DirectoryName);
			}, "get_DirectoryName");

			Assert.Throws<MethodAccessException> (delegate {
				fi.MoveTo (String.Empty);
			}, "MoveTo");

			Assert.Throws<MethodAccessException> (delegate {
				fi.Open (FileMode.Append);
			}, "Open(FileMode)");
			Assert.Throws<MethodAccessException> (delegate {
				fi.Open (FileMode.Truncate, FileAccess.Write);
			}, "Open(FileMode,FileAccess)");
			Assert.Throws<MethodAccessException> (delegate {
				fi.Open (FileMode.CreateNew, FileAccess.ReadWrite, FileShare.ReadWrite);
			}, "Open(FileMode,FileAccess,FileShare)");

			Assert.Throws<MethodAccessException> (delegate {
				fi.OpenWrite ();
			}, "OpenWrite");
		}

		class ConcreteFileSystemInfo : FileSystemInfo {

			public ConcreteFileSystemInfo ()
			{
			}

			public override void Delete ()
			{
				throw new NotImplementedException ();
			}

			public override bool Exists {
				get { throw new NotImplementedException (); }
			}

			public override string Name {
				get { throw new NotImplementedException (); }
			}
		}

		// note: the only way to get FileSystemInfo instances are from OpenFileDialog
		// (which will return a FileInfo)
		// Sadly it's not easy to test since it requires an GUI intervention

		[TestMethod]
		public void System_IO_FileSystemInfo_ctor ()
		{
			Assert.Throws<TypeLoadException> (delegate {
				// this transparent ctor calls into the base [SecurityCritical] ctor
				new ConcreteFileSystemInfo ();
			}, "ctor");
		}

		[Ignore ("blocking UI")] // comment to test locally
		[TestMethod]
		public void System_IO_FileSystemInfo_UI ()
		{
			FileSystemInfo fsi = GetFileInfo ();

			// Transparent (or Safe Critical)

			Assert.IsTrue (fsi.Exists, "Exists");
			Assert.IsNotNull (fsi.Extension, "Extension");
			Assert.IsNotNull (fsi.Name, "Name");
			
			fsi.Refresh ();

			// [SecurityCritical]

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fsi.Attributes);
			}, "get_Attributes");
			Assert.Throws<MethodAccessException> (delegate {
				fsi.Attributes = FileAttributes.Archive;
			}, "set_Attributes");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fsi.CreationTime);
			}, "get_CreationTime");

			Assert.Throws<MethodAccessException> (delegate {
				fsi.Delete ();
			}, "Delete");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fsi.FullName);
			}, "get_FullName");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fsi.LastAccessTime);
			}, "get_LastAccessTime");

			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (fsi.LastWriteTime);
			}, "get_LastWriteTime");
		}

		[TestMethod]
		public void System_IO_Path_GetFullPath ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Path.GetFullPath (String.Empty));
			}, "GetFullPath");
		}

		[TestMethod]
		public void System_IO_Path_GetTempFileName ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (Path.GetTempFileName ());
			}, "GetTempFileName");
		}

		[TestMethod]
		public void System_IO_Path_GetTempPath ()
		{
			Assert.Throws<MethodAccessException> (delegate {
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

			Assert.Throws<MethodAccessException> (delegate {
				new StreamReader (String.Empty);
			}, ".ctor(string)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamReader (String.Empty, false);
			}, ".ctor(string,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamReader (String.Empty, Encoding.Unicode);
			}, ".ctor(string,Encoding)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamReader (String.Empty, Encoding.UTF8, true);
			}, ".ctor(string,Encoding,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamReader (String.Empty, Encoding.BigEndianUnicode, true, -1);
			}, ".ctor(string,Encoding,bool)");
		}

		[TestMethod]
		public void System_IO_StreamWriter_ctor ()
		{
			Assert.IsNotNull (new StreamWriter (Stream.Null));
			Assert.IsNotNull (new StreamWriter (Stream.Null, Encoding.UTF8));
			Assert.IsNotNull (new StreamWriter (Stream.Null, Encoding.Unicode, 1024));

			Assert.Throws<MethodAccessException> (delegate {
				new StreamWriter (String.Empty);
			}, ".ctor(string)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamWriter (String.Empty, false);
			}, ".ctor(string,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamWriter (String.Empty, true, Encoding.Unicode);
			}, ".ctor(string,bool,Encoding)");
			Assert.Throws<MethodAccessException> (delegate {
				new StreamWriter (String.Empty, false, Encoding.UTF8, -1);
			}, ".ctor(string,bool,Encoding,int)");
		}

		[TestMethod]
		public void System_Reflection_Assembly_CodeBase ()
		{
			Assembly a = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (a.CodeBase);
			}, "get_CodeBase");
		}

		[TestMethod]
		public void System_Reflection_Assembly_GetFile ()
		{
			Assembly a = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				a.GetFile (String.Empty);
			}, "GetFile");
		}

		[TestMethod]
		public void System_Reflection_Assembly_GetFiles ()
		{
			Assembly a = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				a.GetFiles (true);
			}, "GetFiles");
		}

		[TestMethod]
		public void System_Reflection_Assembly_GetName ()
		{
			Assembly a = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (a.GetName ());
			}, "GetName");
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (a.GetName (false));
			}, "GetName(bool)");
		}

		[TestMethod]
		public void System_Reflection_Assembly_Load ()
		{
			Assert.IsNotNull (Assembly.Load (typeof (int).Assembly.FullName), "Load(string)");

			byte [] empty = new byte [0];
			Assert.Throws<MethodAccessException> (delegate {
				Assembly.Load (empty);
			}, "Load(byte[])");
			Assert.Throws<MethodAccessException> (delegate {
				Assembly.Load (empty, empty);
			}, "Load(byte[],byte[])");
			Assert.Throws<MethodAccessException> (delegate {
				Assembly.Load (new AssemblyName ());
			}, "Load(AssemblyName)");
		}

		[TestMethod]
		public void System_Reflection_Assembly_LoadFrom ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Assembly.LoadFrom (String.Empty);
			}, "LoadFrom(string)");
		}

		[TestMethod]
		public void System_Reflection_Assembly_Location ()
		{
			Assembly a = typeof (int).Assembly;
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (a.Location);
			}, "get_Location");
		}

		[TestMethod]
		public void System_Reflection_AssemblyName_CodeBase ()
		{
			AssemblyName an = new AssemblyName ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (an.CodeBase);
			}, "get_CodeBase");
			Assert.Throws<MethodAccessException> (delegate {
				an.CodeBase = String.Empty;
			}, "set_CodeBase");
		}

		[TestMethod]
		public void System_Reflection_Module_FullyQualifiedName ()
		{
			Module m = typeof (int).Assembly.GetModules () [0];
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (m.FullyQualifiedName);
			}, "get_FullyQualifiedName");
		}

		private AssemblyBuilder GetAssemblyBuilder ()
		{
			AssemblyName an = new AssemblyName ("Test");
			return AppDomain.CurrentDomain.DefineDynamicAssembly (an, AssemblyBuilderAccess.Run);
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_AddResourceFile ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				ab.AddResourceFile (String.Empty, String.Empty);
			}, "AddResourceFile(string,string)");
			Assert.Throws<MethodAccessException> (delegate {
				ab.AddResourceFile (String.Empty, String.Empty, ResourceAttributes.Private);
			}, "AddResourceFile(string,string,ResourceAttributes)");
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_CodeBase ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (ab.CodeBase);
			}, "get_CodeBase");
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_GetFile ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				ab.GetFile (String.Empty);
			}, "GetFile");
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_GetFiles ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				ab.GetFiles (true);
			}, "GetFiles");
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_Location ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (ab.Location);
			}, "get_Location");
		}

		private CustomAttributeBuilder GetCustomAttributeBuilder ()
		{
			ConstructorInfo ci = typeof (ObsoleteAttribute).GetConstructor (Type.EmptyTypes);
			return new CustomAttributeBuilder (ci, new object [0]);
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_SetCustomAttribute ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			ab.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				ab.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_AssemblyBuilder_SetEntryPoint ()
		{
			AssemblyBuilder ab = GetAssemblyBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				ab.SetEntryPoint (null);
			}, "SetEntryPoint(MethodInfo)");
			Assert.Throws<MethodAccessException> (delegate {
				ab.SetEntryPoint (null, PEFileKinds.Dll);
			}, "SetEntryPoint(MethodInfo,PEFileKinds)");
		}

		private ModuleBuilder GetModuleBuilder ()
		{
			return GetAssemblyBuilder ().DefineDynamicModule ("Test");
		}

		private TypeBuilder GetTypeBuilder ()
		{
			return GetModuleBuilder ().DefineType ("Type");
		}

		[TestMethod]
		public void System_Reflection_Emit_ConstructorBuilder_SetCustomAttribute ()
		{
			ConstructorBuilder cb = GetTypeBuilder ().DefineConstructor (MethodAttributes.Public, CallingConventions.Standard, Type.EmptyTypes);
			cb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				cb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_DynamicMethod_ctor ()
		{
			Assert.IsNotNull (new DynamicMethod ("DM1", typeof (void), Type.EmptyTypes), "ctor(string,Type,Type[])");

			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM2", typeof (void), Type.EmptyTypes, (Module) null);
			}, "ctor(string,Type,Type[],Module)");
			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM3", typeof (void), Type.EmptyTypes, (Module) null, false);
			}, "ctor(string,Type,Type[],Module,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM4", typeof (void), Type.EmptyTypes, (Type) null);
			}, "ctor(string,Type,Type[],Type)");
			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM5", typeof (void), Type.EmptyTypes, (Type) null, false);
			}, "ctor(string,Type,Type[],Type,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM6", MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, (Module) null, true);
			}, "ctor(string,MethodAttributes,CallingConventions,Type,Type[],Module,bool)");
			Assert.Throws<MethodAccessException> (delegate {
				new DynamicMethod ("DM7", MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, (Type) null, true);
			}, "ctor(string,MethodAttributes,CallingConventions,Type,Type[],Type,bool)");
		}

		[TestMethod]
		public void System_Reflection_Emit_EnumBuilder_SetCustomAttribute ()
		{
			EnumBuilder eb = GetModuleBuilder ().DefineEnum ("DE", TypeAttributes.Public, typeof (int));
			eb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				eb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_EventBuilder_SetCustomAttribute ()
		{
			EventBuilder eb = GetTypeBuilder ().DefineEvent ("EV", EventAttributes.None, typeof (EventArgs));
			eb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				eb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		private FieldBuilder GetFieldBuilder ()
		{
			return GetTypeBuilder ().DefineField ("F", typeof (int), FieldAttributes.Public);
		}

		[TestMethod]
		public void System_Reflection_Emit_FieldBuilder_SetCustomAttribute ()
		{
			FieldBuilder fb = GetFieldBuilder ();
			fb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				fb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_FieldBuilder_SetOffset ()
		{
			FieldBuilder fb = GetFieldBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				fb.SetOffset (0);
			}, "SetOffset");
		}

		[TestMethod]
		public void System_Reflection_Emit_GenericTypeParameterBuilder_SetCustomAttribute ()
		{
			GenericTypeParameterBuilder[] gtpb = GetTypeBuilder ().DefineGenericParameters (new string [1] { "GTP" });
			gtpb [0].SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				gtpb [0].SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		private MethodBuilder GetMethodBuilder ()
		{
			return GetTypeBuilder ().DefineMethod ("M", MethodAttributes.Public, CallingConventions.Standard, 
				typeof (void), new Type[1] { typeof (string) });
		}

		[TestMethod]
		public void System_Reflection_Emit_MethodBuilder_CreateMethodBody ()
		{
			MethodBuilder mb = GetMethodBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.CreateMethodBody (null, 0);
			}, "CreateMethodBody");
		}

		[TestMethod]
		public void System_Reflection_Emit_MethodBuilder_SetCustomAttribute ()
		{
			MethodBuilder mb = GetMethodBuilder ();
			mb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				mb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefineInitializedData ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineInitializedData (String.Empty, null, FieldAttributes.Public);
			}, "DefineInitializedData");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefineManifestResource ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineManifestResource (String.Empty, Stream.Null, ResourceAttributes.Public);
			}, "DefineManifestResource");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefinePInvokeMethod ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefinePInvokeMethod (String.Empty, String.Empty, MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, CallingConvention.StdCall, CharSet.Ansi);
			}, "DefinePInvokeMethod-1");
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefinePInvokeMethod (String.Empty, String.Empty, String.Empty, MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, CallingConvention.StdCall, CharSet.Ansi);
			}, "DefinePInvokeMethod-2");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefineType ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.IsNotNull (mb.DefineType ("T1"), "DefineType(string)");
			Assert.IsNotNull (mb.DefineType ("T2", TypeAttributes.Public), "DefineType(string,TypeAttributes)");
			Assert.IsNotNull (mb.DefineType ("T3", TypeAttributes.Public, typeof (object)), "DefineType(string,TypeAttributes,Type)");
			Assert.IsNotNull (mb.DefineType ("T4", TypeAttributes.Public, typeof (object),Type.EmptyTypes), "DefineType(string,TypeAttributes,Type,Type[])");

			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineType ("T5", TypeAttributes.Public, typeof (object), 1);
			}, "DefineType(string,TypeAttributes,Type,int)");
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineType ("T6", TypeAttributes.Public, typeof (object), PackingSize.Size1);
			}, "DefineType(string,TypeAttributes,Type,PackingSize)");
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineType ("T7", TypeAttributes.Public, typeof (object), PackingSize.Size1, 1);
			}, "DefineType(string,TypeAttributes,Type,PackingSize,int)");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefineUninitializedData ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineUninitializedData (String.Empty, 0, FieldAttributes.Public);
			}, "DefineUninitializedData");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_DefineUnmanagedResource ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineUnmanagedResource ((byte []) null);
			}, "DefineUnmanagedResource(byte[])");
			Assert.Throws<MethodAccessException> (delegate {
				mb.DefineUnmanagedResource (String.Empty);
			}, "DefineUnmanagedResource(string)");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_FullyQualifiedName ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (mb.FullyQualifiedName);
			}, "get_FullyQualifiedName");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_SetCustomAttribute ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			mb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				mb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_ModuleBuilder_SetUserEntryPoint ()
		{
			ModuleBuilder mb = GetModuleBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				mb.SetUserEntryPoint (null);
			}, "SetUserEntryPoint");
		}

		[TestMethod]
		public void System_Reflection_Emit_ParameterBuilder_SetCustomAttribute ()
		{
			ParameterBuilder pb = GetMethodBuilder ().DefineParameter (0, ParameterAttributes.None, "a");
			pb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				pb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_PropertyBuilder_SetCustomAttribute ()
		{
			PropertyBuilder pb = GetTypeBuilder ().DefineProperty ("P", PropertyAttributes.None, typeof (int), Type.EmptyTypes);
			pb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				pb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		[TestMethod]
		public void System_Reflection_Emit_TypeBuilder_DefineInitializedData ()
		{
			TypeBuilder tb = GetTypeBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefineInitializedData (String.Empty, null, FieldAttributes.Public);
			}, "DefineInitializedData");
		}

		[TestMethod]
		public void System_Reflection_Emit_TypeBuilder_DefineNestedType ()
		{
			TypeBuilder tb = GetTypeBuilder ();
			Assert.IsNotNull (tb.DefineNestedType ("T1"), "DefineNestedType(string)");
			Assert.IsNotNull (tb.DefineNestedType ("T2", TypeAttributes.NestedPublic), "DefineNestedType(string,TypeAttributes)");
			Assert.IsNotNull (tb.DefineNestedType ("T3", TypeAttributes.NestedPublic, typeof (object)), "DefineNestedType(string,TypeAttributes,Type)");
			Assert.IsNotNull (tb.DefineNestedType ("T4", TypeAttributes.NestedPublic, typeof (object), Type.EmptyTypes), "DefineNestedType(string,TypeAttributes,Type,Type[])");

			Assert.Throws<MethodAccessException> (delegate {
				tb.DefineNestedType ("T5", TypeAttributes.NestedPublic, typeof (object), 0);
			}, "DefineNestedType(string,TypeAttributes,Type,int)");
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefineNestedType ("T6", TypeAttributes.NestedPublic, typeof (object), PackingSize.Size1);
			}, "DefineNestedType(string,TypeAttributes,Type,PackingSize)");
		}

		[TestMethod]
		public void System_Reflection_Emit_TypeBuilder_DefinePInvokeMethod ()
		{
			TypeBuilder tb = GetTypeBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefinePInvokeMethod (String.Empty, String.Empty, MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, CallingConvention.StdCall, CharSet.Ansi);
			}, "DefinePInvokeMethod-1");
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefinePInvokeMethod (String.Empty, String.Empty, String.Empty, MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, CallingConvention.StdCall, CharSet.Ansi);
			}, "DefinePInvokeMethod-2");
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefinePInvokeMethod (String.Empty, String.Empty, String.Empty, MethodAttributes.Public, CallingConventions.Standard, typeof (void), Type.EmptyTypes, Type.EmptyTypes, Type.EmptyTypes, (Type [] []) null, (Type [] []) null, CallingConvention.StdCall, CharSet.Unicode);
			}, "DefinePInvokeMethod-3");
		}

		[TestMethod]
		public void System_Reflection_Emit_TypeBuilder_DefineUninitializedData ()
		{
			TypeBuilder tb = GetTypeBuilder ();
			Assert.Throws<MethodAccessException> (delegate {
				tb.DefineUninitializedData (String.Empty, 0, FieldAttributes.Public);
			}, "DefineUninitializedData");
		}

		[TestMethod]
		public void System_Reflection_Emit_TypeBuilder_SetCustomAttribute ()
		{
			TypeBuilder tb = GetTypeBuilder ();
			tb.SetCustomAttribute (GetCustomAttributeBuilder ());

			Assert.Throws<MethodAccessException> (delegate {
				tb.SetCustomAttribute (null, null);
			}, "SetCustomAttribute(ConstructorInfo,byte[])");
		}

		Assembly CurrentDomain_AssemblyResolve (object sender, ResolveEventArgs args)
		{
			throw new NotImplementedException ();
		}

		[TestMethod]
		public void System_ResolveEventHandler ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler (CurrentDomain_AssemblyResolve);
			}, "add_AssemblyResolve");
		}

		[TestMethod]
		public void System_Resources_ResourceSet_ctor ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new ResourceSet ((IResourceReader) null);
			}, ".ctor(IResourceReader)");

			Assert.Throws<MethodAccessException> (delegate {
				new ResourceSet (String.Empty);
			}, ".ctor(string)");
			Assert.Throws<MethodAccessException> (delegate {
				new ResourceSet (Stream.Null);
			}, ".ctor(Stream)");
		}

		class ConcreteCriticalHandle : CriticalHandle {

			public ConcreteCriticalHandle ()
				: base (IntPtr.Zero)
			{
			}

			public override bool IsInvalid {
				get { throw new NotImplementedException (); }
			}

			protected override bool ReleaseHandle ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void System_Runtime_InteropServices_CriticalHandle ()
		{
			// whole type is decorated with [SecurityCritical]
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteCriticalHandle ();
			}, "ctor");
			// note: nothing seems to (directly) expose CriticalHandle in SL2 visible API
		}

		[TestMethod]
		public void System_Runtime_InteropServices_GCHandle_AddrOfPinnedObject ()
		{
			GCHandle handle = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (handle.AddrOfPinnedObject ());
			}, "AddrOfPinnedObject");
		}

		[TestMethod]
		public void System_Runtime_InteropServices_GCHandle_Alloc ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				GCHandle.Alloc (null);
			}, "Alloc(object)");
			Assert.Throws<MethodAccessException> (delegate {
				GCHandle.Alloc (null, GCHandleType.Normal);
			}, "Alloc(object,GCHandleType)");
		}

		[TestMethod]
		public void System_Runtime_InteropServices_GCHandle_Target ()
		{
			GCHandle handle = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (handle.Target);
			}, "get_Target");
			Assert.Throws<MethodAccessException> (delegate {
				handle.Target = handle;
			}, "set_Target");
		}

		[TestMethod]
		public void System_Runtime_InteropServices_GCHandle_Free ()
		{
			GCHandle handle = new GCHandle ();
			Assert.Throws<MethodAccessException> (delegate {
				handle.Free ();
			}, "Free");
		}

		[TestMethod]
		public void System_Runtime_InteropServices_GCHandle_opExplicit ()
		{
			GCHandle handle = new GCHandle ();
			IntPtr ptr = (IntPtr) handle;
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull ((GCHandle) ptr);
			}, "op_Explicit");
		}

		[TestMethod]
		public void System_Runtime_InteropServices_Marshal ()
		{
			// note: [SecurityCritical] is on the type

			Assert.Throws<FieldAccessException> (delegate {
				// note: an uncommon case, this field is critical
				Assert.IsNotNull (Marshal.SystemDefaultCharSize);
			}, "SystemDefaultCharSize");

			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy ((byte []) null, 0, IntPtr.Zero, 0);
			}, "Copy(byte[],int,IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy ((char []) null, 0, IntPtr.Zero, 0);
			}, "Copy(char[],int,IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy ((double []) null, 0, IntPtr.Zero, 0);
			}, "Copy(double[],int,IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy ((float []) null, 0, IntPtr.Zero, 0);
			}, "Copy(float[],int,IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy ((int []) null, 0, IntPtr.Zero, 0);
			}, "Copy(int[],int,IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (byte []) null, 0, 0);
			}, "Copy(IntPtr,long[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (short []) null, 0, 0);
			}, "Copy(IntPtr,short[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (byte []) null, 0, 0);
			}, "Copy(IntPtr,byte[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (char []) null, 0, 0);
			}, "Copy(IntPtr,char[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (double []) null, 0, 0);
			}, "Copy(IntPtr,double[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (float []) null, 0, 0);
			}, "Copy(IntPtr,float[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (int []) null, 0, 0);
			}, "Copy(IntPtr,int[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (long []) null, 0, 0);
			}, "Copy(IntPtr,long[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Copy (IntPtr.Zero, (short []) null, 0, 0);
			}, "Copy(IntPtr,short[],int,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.GetDelegateForFunctionPointer (IntPtr.Zero, null);
			}, "GetDelegateForFunctionPointer");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.GetExceptionForHR (0);
			}, "GetExceptionForHR");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.GetFunctionPointerForDelegate (null);
			}, "GetFunctionPointerForDelegate");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.GetHRForException (null);
			}, "GetHRForException");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.GetLastWin32Error ();
			}, "GetLastWin32Error");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.Prelink (null);
			}, "Prelink");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PrelinkAll (typeof (int));
			}, "PrelinkAll");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStringAnsi (IntPtr.Zero);
			}, "PtrToStringAnsi(IntPtr)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStringAnsi (IntPtr.Zero, -1);
			}, "PtrToStringAnsi(IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStringUni (IntPtr.Zero);
			}, "PtrToStringUni(IntPtr)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStringUni (IntPtr.Zero, -1);
			}, "PtrToStringUni(IntPtr,int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStructure (IntPtr.Zero, this);
			}, "PtrToStructure(IntPtr,object)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.PtrToStructure (IntPtr.Zero, typeof(int));
			}, "PtrToStructure(IntPtr,Type)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ReadByte (IntPtr.Zero);
			}, "ReadByte");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ReadInt16 (IntPtr.Zero);
			}, "ReadInt16");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ReadInt32 (IntPtr.Zero);
			}, "ReadInt32");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ReadInt64 (IntPtr.Zero);
			}, "ReadInt64");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ReadIntPtr (IntPtr.Zero);
			}, "ReadIntPtr");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.SizeOf (this);
			}, "SizeOf(object)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.SizeOf (typeof(int));
			}, "SizeOf(Type)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.StructureToPtr (this, IntPtr.Zero, false);
			}, "StructureToPtr");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ThrowExceptionForHR (0);
			}, "ThrowExceptionForHR(int)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.ThrowExceptionForHR (0, IntPtr.Zero);
			}, "ThrowExceptionForHR(int,IntPtr)");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.WriteByte (IntPtr.Zero, 0);
			}, "WriteByte");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.WriteInt16 (IntPtr.Zero, 0);
			}, "WriteInt16");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.WriteInt32 (IntPtr.Zero, 0);
			}, "WriteInt32");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.WriteInt64 (IntPtr.Zero, 0);
			}, "WriteInt64");
			Assert.Throws<MethodAccessException> (delegate {
				Marshal.WriteIntPtr (IntPtr.Zero, IntPtr.Zero);
			}, "WriteIntPtr");
		}

		class ConcreteSafeHandle : SafeHandle {
			public override bool IsInvalid {
				get { throw new NotImplementedException (); }
			}

			protected override bool ReleaseHandle ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void System_Runtime_InteropServices_SafeHandle ()
		{
			// whole type is decorated with [SecurityCritical]
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteSafeHandle ();
			}, "ctor");
			// note: SafeHandle seems only (directly) exposed by
			// System.Threading.ThreadPool::BindHandle method 
			// which takes it as a parameter and is decorated with [SecurityCritical]
		}

		class ConcreteSecurityState : SecurityState {

			public ConcreteSecurityState ()
			{
			}

			public override void EnsureState ()
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void System_Security_SecurityState ()
		{
			Assert.Throws<TypeLoadException> (delegate {
				// SecurityState type is [SecurityCritical] so we can't inherit from it
				// inside application (transparent) code
				new ConcreteSecurityState ();
			}, "ctor");
		}

		static byte [] raw_cert = { 0x30,0x82,0x01,0xFF,0x30,0x82,0x01,0x6C,0x02,0x05,0x02,0x72,0x00,0x06,0xE8,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x02,0x05,0x00,0x30,0x5F,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x20,0x30,0x1E,0x06,0x03,0x55,0x04,0x0A,0x13,0x17,0x52,0x53,0x41,0x20,0x44,0x61,0x74,0x61,0x20,0x53,0x65,0x63,0x75,0x72,0x69,0x74,0x79,0x2C,0x20,0x49,0x6E,0x63,0x2E,0x31,0x2E,0x30,0x2C,0x06,0x03,0x55,0x04,0x0B,0x13,0x25,0x53,0x65,0x63,0x75,0x72,0x65,0x20,0x53,0x65,0x72,0x76,
			0x65,0x72,0x20,0x43,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x20,0x41,0x75,0x74,0x68,0x6F,0x72,0x69,0x74,0x79,0x30,0x1E,0x17,0x0D,0x39,0x36,0x30,0x33,0x31,0x32,0x31,0x38,0x33,0x38,0x34,0x37,0x5A,0x17,0x0D,0x39,0x37,0x30,0x33,0x31,0x32,0x31,0x38,0x33,0x38,0x34,0x36,0x5A,0x30,0x61,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0A,0x43,0x61,0x6C,0x69,0x66,0x6F,0x72,0x6E,0x69,0x61,0x31,0x14,0x30,0x12,0x06,0x03,
			0x55,0x04,0x0A,0x13,0x0B,0x43,0x6F,0x6D,0x6D,0x65,0x72,0x63,0x65,0x4E,0x65,0x74,0x31,0x27,0x30,0x25,0x06,0x03,0x55,0x04,0x0B,0x13,0x1E,0x53,0x65,0x72,0x76,0x65,0x72,0x20,0x43,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x20,0x41,0x75,0x74,0x68,0x6F,0x72,0x69,0x74,0x79,0x30,0x70,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x5F,0x00,0x30,0x5C,0x02,0x55,0x2D,0x58,0xE9,0xBF,0xF0,0x31,0xCD,0x79,0x06,0x50,0x5A,0xD5,0x9E,0x0E,0x2C,0xE6,0xC2,0xF7,0xF9,
			0xD2,0xCE,0x55,0x64,0x85,0xB1,0x90,0x9A,0x92,0xB3,0x36,0xC1,0xBC,0xEA,0xC8,0x23,0xB7,0xAB,0x3A,0xA7,0x64,0x63,0x77,0x5F,0x84,0x22,0x8E,0xE5,0xB6,0x45,0xDD,0x46,0xAE,0x0A,0xDD,0x00,0xC2,0x1F,0xBA,0xD9,0xAD,0xC0,0x75,0x62,0xF8,0x95,0x82,0xA2,0x80,0xB1,0x82,0x69,0xFA,0xE1,0xAF,0x7F,0xBC,0x7D,0xE2,0x7C,0x76,0xD5,0xBC,0x2A,0x80,0xFB,0x02,0x03,0x01,0x00,0x01,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x02,0x05,0x00,0x03,0x7E,0x00,0x54,0x20,0x67,0x12,0xBB,0x66,0x14,0xC3,0x26,0x6B,0x7F,
			0xDA,0x4A,0x25,0x4D,0x8B,0xE0,0xFD,0x1E,0x53,0x6D,0xAC,0xA2,0xD0,0x89,0xB8,0x2E,0x90,0xA0,0x27,0x43,0xA4,0xEE,0x4A,0x26,0x86,0x40,0xFF,0xB8,0x72,0x8D,0x1E,0xE7,0xB7,0x77,0xDC,0x7D,0xD8,0x3F,0x3A,0x6E,0x55,0x10,0xA6,0x1D,0xB5,0x58,0xF2,0xF9,0x0F,0x2E,0xB4,0x10,0x55,0x48,0xDC,0x13,0x5F,0x0D,0x08,0x26,0x88,0xC9,0xAF,0x66,0xF2,0x2C,0x9C,0x6F,0x3D,0xC3,0x2B,0x69,0x28,0x89,0x40,0x6F,0x8F,0x35,0x3B,0x9E,0xF6,0x8E,0xF1,0x11,0x17,0xFB,0x0C,0x98,0x95,0xA1,0xC2,0xBA,0x89,0x48,0xEB,0xB4,0x06,0x6A,0x22,0x54,
			0xD7,0xBA,0x18,0x3A,0x48,0xA6,0xCB,0xC2,0xFD,0x20,0x57,0xBC,0x63,0x1C };

		[TestMethod]
		public void System_Security_Cryptography_X509Certificates_X509Certificate_ctor ()
		{
			Assert.IsNotNull (new X509Certificate (), ".ctor()");
			Assert.IsNotNull (new X509Certificate (raw_cert), ".ctor(byte[])");
			Assert.IsNotNull (new X509Certificate (raw_cert, String.Empty), ".ctor(byte[],string)");
			Assert.IsNotNull (new X509Certificate (raw_cert, String.Empty, X509KeyStorageFlags.DefaultKeySet), ".ctor(byte[],string,X509KeyStorageFlags)");
			Assert.IsNotNull (new X509Certificate (new X509Certificate (raw_cert)), ".ctor(X509Certificate)");

			Assert.Throws<MethodAccessException> (delegate {
				new X509Certificate (IntPtr.Zero);
			}, ".ctor(IntPtr)");
			Assert.Throws<MethodAccessException> (delegate {
				new X509Certificate (String.Empty);
			}, ".ctor(string)");
			Assert.Throws<MethodAccessException> (delegate {
				new X509Certificate (String.Empty, String.Empty);
			}, ".ctor(string,string)");
			Assert.Throws<MethodAccessException> (delegate {
				new X509Certificate (String.Empty, String.Empty, X509KeyStorageFlags.DefaultKeySet);
			}, ".ctor(string,string,X509KeyStorageFlags)");
		}

		[TestMethod]
		public void System_Security_Cryptography_X509Certificates_X509Certificate_CreateFromCertFile ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				X509Certificate.CreateFromCertFile (String.Empty);
			}, "CreateFromCertFile");
		}

		[TestMethod]
		public void System_Security_Cryptography_X509Certificates_X509Certificate_Handle ()
		{
			X509Certificate cert = new X509Certificate ();
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (cert.Handle);
			}, "get_Handle");
		}

		[TestMethod]
		public void System_Security_Cryptography_X509Certificates_X509Certificate_Import ()
		{
			X509Certificate cert = new X509Certificate ();

			cert.Import (raw_cert);
			cert.Import (raw_cert, String.Empty, X509KeyStorageFlags.DefaultKeySet);

			Assert.Throws<MethodAccessException> (delegate {
				cert.Import (String.Empty);
			}, "Import(string)");
			Assert.Throws<MethodAccessException> (delegate {
				cert.Import (String.Empty, String.Empty, X509KeyStorageFlags.DefaultKeySet);
			}, "Import(string,string,X509KeyStorageFlags)");
		}

		[TestMethod]
		public void System_Threading_SynchronizationContext_SetSynchronizationContext ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				SynchronizationContext.SetSynchronizationContext (null);
			}, "SetSynchronizationContext");
		}

		[TestMethod]
		public void System_Threading_SynchronizationContext_SetThreadStaticContext ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				SynchronizationContext.SetThreadStaticContext (null);
			}, "SetThreadStaticContext");
		}

		[TestMethod]
		[Ignore ("this will freeze the browser until coreclr is enabled")]
		public void System_Threading_Thread_Abort ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Thread.CurrentThread.Abort ();
			}, "Abort");
		}

		[TestMethod]
		public void System_Threading_ThreadPool_BindHandle ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				ThreadPool.BindHandle (null);
			}, "BindHandle");
		}

		[TestMethod]
		public void System_Threading_ThreadPool_SetMaxThreads ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				ThreadPool.SetMaxThreads (0, 0);
			}, "SetMaxThreads");
		}

		[TestMethod]
		public void System_Threading_ThreadPool_SetMinThreads ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				ThreadPool.SetMinThreads (0, 0);
			}, "SetMinThreads");
		}

		[TestMethod]
		public void System_Threading_WaitHandle_SafeWaitHandle ()
		{
			WaitHandle wh = new AutoResetEvent (false);
			Assert.Throws<MethodAccessException> (delegate {
				Assert.IsNotNull (wh.SafeWaitHandle);
			}, "get_SafeWaitHandle");
			Assert.Throws<MethodAccessException> (delegate {
				wh.SafeWaitHandle = null;
			}, "set_SafeWaitHandle");
		}

		// System.dll

		[TestMethod]
		public void System_ComponentModel_AsyncOperationManager_SynchronizationContext ()
		{
			Assert.IsNotNull (AsyncOperationManager.SynchronizationContext, "get");
			Assert.Throws<MethodAccessException> (delegate {
				AsyncOperationManager.SynchronizationContext = null;
			}, "set_SynchronizationContext");
		}

		// System.Windows.dll

		// note: Microsoft.Internal.IManagedFrameworkInternalHelper::SetContextEx
		// cannot be unit tested since we CANNOT inherit from an internal with
		// [SecurityCritical] members (at least not in application code)

		[TestMethod]
		public void System_Windows_Deployment_RegisterAssembly ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Deployment.RegisterAssembly (null);
			}, "RegisterAssembly");
		}

		[TestMethod]
		public void System_Windows_Deployment_SetCurrentApplication ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				Deployment.SetCurrentApplication (null);
			}, "SetCurrentApplication");
		}

		[TestMethod]
		public void System_Windows_Interop_HostingRenderTargetBitmap_ctor ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				new HostingRenderTargetBitmap (0, 0, IntPtr.Zero);
			}, ".ctor");
		}

		[TestMethod]
		public void System_Windows_Markup_XamlReader_LoadWithInitialTemplateValidation ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				XamlReader.LoadWithInitialTemplateValidation (null);
			}, "LoadWithInitialTemplateValidation");
		}

		void CurrentDomain_UnhandledException (object sender, UnhandledExceptionEventArgs e)
		{
			throw new NotImplementedException ();
		}

		[TestMethod]
		public void System_UnhandledExceptionEventHandler ()
		{
			Assert.Throws<MethodAccessException> (delegate {
				AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler (CurrentDomain_UnhandledException);
			}, "UnhandledExceptionEventHandler");
		}
	}
}
