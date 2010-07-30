//
// Unit tests for TextPointer
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Documents {

	[TestClass]
	public partial class TextPointerTest {

		RichTextBox rtb;

		[TestInitialize]
		public void Setup ()
		{
			rtb = new RichTextBox ();
		}

		TextPointer GetTextPointer ()
		{
			return rtb.ContentStart;
		}

		[TestMethod]
		public void CompareTo ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				GetTextPointer ().CompareTo (null);
			}, "null");

			Assert.AreEqual (0, GetTextPointer ().CompareTo (GetTextPointer ()), "self");
		}

		[TestMethod]
		[MoonlightBug]
		public void GetCharacterRect ()
		{
			Rect r = GetTextPointer ().GetCharacterRect ((LogicalDirection) Int32.MinValue);
			Assert.IsTrue (r.IsEmpty, "invalid enum / Empty");
		}

		[TestMethod]
		public void GetNextInsertionPosition ()
		{
			Assert.IsNull (GetTextPointer ().GetNextInsertionPosition ((LogicalDirection) Int32.MinValue), "invalid enum");
		}

		[TestMethod]
		[MoonlightBug]
		public void GetPositionAtOffset ()
		{
			Assert.IsNull (GetTextPointer ().GetPositionAtOffset (Int32.MinValue, LogicalDirection.Forward), "invalid offset");

			// invalid enum
			TextPointer p = GetTextPointer ().GetPositionAtOffset (0, (LogicalDirection) Int32.MinValue); 
			Assert.IsNotNull (p, "invalid enum");
			Assert.IsTrue (p.IsAtInsertionPosition, "IsAtInsertionPosition");
			Assert.AreEqual (LogicalDirection.Forward, p.LogicalDirection, "LogicalDirection");
			Assert.AreEqual (rtb, p.Parent, "Parent");
		}
	}
}

