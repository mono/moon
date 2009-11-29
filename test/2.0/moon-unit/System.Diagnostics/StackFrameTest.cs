//
// Unit tests for System.Diagnostics.StackFrame
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Diagnostics;
using System.Reflection;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Diagnostics {

	[TestClass]
	public class StackFrameTest {

		[TestMethod]
		public void Default ()
		{
			StackFrame sf = new StackFrame ();
			Assert.AreEqual (0, sf.GetFileLineNumber (), "GetFileLineNumber");
			Assert.AreEqual (0, sf.GetFileColumnNumber (), "GetFileColumnNumber");
			Assert.IsTrue (sf.GetILOffset () >= 0, "GetILOffset");
			Assert.IsTrue (sf.GetNativeOffset () >= 0, "GetNativeOffset");
			Assert.AreEqual ("Default", sf.GetMethod ().Name, "GetMethod");
		}

		// ctors with bool (fNeedFileInfo) are [SecurityCritical]

		[TestMethod]
		public void FileName_LineNumber ()
		{
			StackFrame sf = new StackFrame (null, Int32.MinValue);
			Assert.AreEqual (Int32.MinValue, sf.GetFileLineNumber (), "GetFileLineNumber");
			Assert.AreEqual (0, sf.GetFileColumnNumber (), "GetFileColumnNumber");
			Assert.IsTrue (sf.GetILOffset () >= 0, "GetILOffset");
			Assert.IsTrue (sf.GetNativeOffset () > 0, "GetNativeOffset");
			Assert.AreEqual ("FileName_LineNumber", sf.GetMethod ().Name, "GetMethod");
		}

		[TestMethod]
		public void FileName_LineNumber_ColumnNumber ()
		{
			StackFrame sf = new StackFrame (String.Empty, Int32.MinValue, Int32.MaxValue);
			Assert.AreEqual (Int32.MinValue, sf.GetFileLineNumber (), "GetFileLineNumber");
			Assert.AreEqual (Int32.MaxValue, sf.GetFileColumnNumber (), "GetFileColumnNumber");
			Assert.IsTrue (sf.GetILOffset () >= 0, "GetILOffset");
			Assert.IsTrue (sf.GetNativeOffset () > 0, "GetNativeOffset");
			Assert.AreEqual ("FileName_LineNumber_ColumnNumber", sf.GetMethod ().Name, "GetMethod");
		}

		[TestMethod]
		[MoonlightBug ("for easier debugging ML let the filename out, but not the path")]
		public void StackFrame_FileName_Leak ()
		{
			StackFrame sf = new StackFrame ("/path/to/the/source/file.cs", 1);
			Assert.IsTrue (sf.ToString ().Contains ("<filename unknown>"), "no file name leaked");
		}

		[TestMethod]
		[SilverlightBug ("for easier debugging ML let the filename out, but not the path")]
		public void StackFrame_Path_Leak ()
		{
			StackFrame sf = new StackFrame ("/path/to/the/source/file.cs", 1);
			Assert.IsFalse (sf.ToString ().Contains ("path/to/the/source"), "no path leaked");
			Assert.IsTrue (sf.ToString ().Contains ("file.cs"), "filename");
		}
	}
}

