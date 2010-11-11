//
// Unit tests for System.Windows.DataObjectTest
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Security;
using System.Windows;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class DataObjectTest {

		[TestMethod]
		public void Defaults ()
		{
			DataObject dobj = new DataObject ();

			Assert.Throws<SecurityException> (() => dobj.GetFormats (), "GetFormats ()");
			Assert.Throws<SecurityException> (() => dobj.GetFormats (true), "GetFormats (true)");
			Assert.Throws<SecurityException> (() => dobj.GetFormats (false), "GetFormats (false)");

			Assert.Throws<SecurityException> (() => dobj.GetData (DataFormats.FileDrop), "GetData (string)");
			Assert.Throws<SecurityException> (() => dobj.GetData (DataFormats.FileDrop, true), "GetData (string,true)");
			Assert.Throws<SecurityException> (() => dobj.GetData (DataFormats.FileDrop, false), "GetData (string,false)");
			Assert.Throws<SecurityException> (() => dobj.GetData (typeof(string)), "GetData (Type)");

			Assert.Throws<SecurityException> (() => dobj.GetDataPresent (DataFormats.FileDrop), "GetDataPresent (string)");
			Assert.Throws<SecurityException> (() => dobj.GetDataPresent (DataFormats.FileDrop, true), "GetDataPresent (string,true)");
			Assert.Throws<SecurityException> (() => dobj.GetDataPresent (DataFormats.FileDrop, false), "GetDataPresent (string,false)");
			Assert.Throws<SecurityException> (() => dobj.GetDataPresent (typeof (string)), "GetDataPresent (Type)");

			Assert.Throws<SecurityException> (() => dobj.SetData (DataFormats.FileDrop), "SetData (string)");
			Assert.Throws<SecurityException> (() => dobj.SetData (DataFormats.FileDrop, true), "SetData (string,true)");
			Assert.Throws<SecurityException> (() => dobj.SetData (DataFormats.FileDrop, false), "SetData (string,false)");
			Assert.Throws<SecurityException> (() => dobj.SetData (typeof (string)), "SetData (Type)");
		}

		[TestMethod]
		public void Ctor_String ()
		{
			Assert.Throws<NotImplementedException> (() => new DataObject (null), "null");
			Assert.Throws<NotImplementedException> (() => new DataObject (String.Empty), "Empty");
			Assert.Throws<NotImplementedException> (() => new DataObject (DataFormats.FileDrop), "FileDrop");
		}
	}
}

