//
// Unit tests for System.Exception
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
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System {

	[TestClass]
	public class ExceptionTest {

		[TestMethod]
		[MoonlightBug ("for easier debugging ML let the filename out, but not the path")]
		public void Exception_FileName_Leak ()
		{
			try {
				throw new Exception ();
			}
			catch (Exception e) {
				Assert.IsFalse (e.StackTrace.Contains ("ExceptionTest.cs"), "StackTrace");
				Assert.IsFalse (e.ToString ().Contains ("ExceptionTest.cs"), "ToString");
			}
		}

		[TestMethod]
		[SilverlightBug ("for easier debugging ML let the filename out, but not the path")]
		public void Exception_Path_Leak ()
		{
			try {
				throw new Exception ();
			}
			catch (Exception e) {
				Assert.IsFalse (e.StackTrace.Contains ("moon-unit"), "StackTrace" + e.StackTrace);
				Assert.IsFalse (e.ToString ().Contains ("moon-unit"), "ToString");
			}
		}
	}
}

