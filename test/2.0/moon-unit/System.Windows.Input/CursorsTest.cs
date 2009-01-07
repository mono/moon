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
using System.Windows;
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

		public class ConcreteFrameworkElement : FrameworkElement {
		}

		[TestMethod]
		public void Set ()
		{
			ConcreteFrameworkElement cfe = new ConcreteFrameworkElement ();
			Assert.IsNull (cfe.Cursor, "default");

			cfe.Cursor = Cursors.Arrow;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.Arrow), "Arrow");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "Arrow-null");

			cfe.Cursor = Cursors.Eraser;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.Eraser), "Eraser");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "Eraser-null");

			cfe.Cursor = Cursors.Hand;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.Hand), "Hand");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "Hand-null");

			cfe.Cursor = Cursors.IBeam;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.IBeam), "IBeam");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "IBeam-null");

			cfe.Cursor = Cursors.None;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.None), "None");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "None-null");

			cfe.Cursor = Cursors.SizeNS;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.SizeNS), "SizeNS");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "SizeNS-null");

			cfe.Cursor = Cursors.SizeWE;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.SizeWE), "SizeWE");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "SizeWE-null");

			cfe.Cursor = Cursors.Stylus;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.Stylus), "Stylus");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "Stylus-null");

			cfe.Cursor = Cursors.Wait;
			Assert.IsTrue (Object.ReferenceEquals (cfe.Cursor, Cursors.Wait), "Wait");
			cfe.Cursor = null;
			Assert.IsNull (cfe.Cursor, "Wait-null");
		}
	}
}
