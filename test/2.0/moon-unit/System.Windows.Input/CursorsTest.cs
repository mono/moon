//
// Unit tests for Cursor[s]
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
using System.Windows.Input;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Input {

	[TestClass]
	public class CursorsTest {

		[TestMethod]
		public void Cursor ()
		{
			Assert.AreEqual ("Arrow", Cursors.Arrow.ToString (), "Arrow");
			Assert.AreEqual ("Eraser", Cursors.Eraser.ToString (), "Eraser");
			Assert.AreEqual ("Hand", Cursors.Hand.ToString (), "Hand");
			Assert.AreEqual ("IBeam", Cursors.IBeam.ToString (), "IBeam");
			Assert.AreEqual ("None", Cursors.None.ToString (), "None");
			Assert.AreEqual ("SizeNS", Cursors.SizeNS.ToString (), "SizeNS");
			Assert.AreEqual ("SizeWE", Cursors.SizeWE.ToString (), "SizeWE");
			Assert.AreEqual ("Stylus", Cursors.Stylus.ToString (), "Stylus");
			Assert.AreEqual ("Wait", Cursors.Wait.ToString (), "Wait");
		}
	}
}
