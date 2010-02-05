//
// Unit tests for System.Windows.Browser.HtmlPopupWindowOptions
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
//
//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Windows.Browser;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Browser {

	[TestClass]
	public class HtmlPopupWindowOptionsTest {

		[TestMethod]
		public void Defaults ()
		{
			HtmlPopupWindowOptions options = new HtmlPopupWindowOptions ();
			Assert.IsTrue (options.Directories, "Directories");
			Assert.AreEqual (450, options.Height, "Height");
			Assert.AreEqual (100, options.Left, "Left");
			Assert.IsTrue (options.Location, "Location");
			Assert.IsTrue (options.Menubar, "Menubar");
			Assert.IsTrue (options.Resizeable, "Resizeable");
			Assert.IsTrue (options.Scrollbars, "Scrollbars");
			Assert.IsTrue (options.Status, "Status");
			Assert.IsTrue (options.Toolbar, "Toolbar");
			Assert.AreEqual (100, options.Top, "Top");
			Assert.AreEqual (600, options.Width, "Width");
		}

		[TestMethod]
		public void Limits ()
		{
			HtmlPopupWindowOptions options = new HtmlPopupWindowOptions ();

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Height = Int32.MinValue;
			}, "Height-MinValue");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Height = -1;
			}, "Height-Negative");
			options.Height = 0;
			Assert.AreEqual (0, options.Height, "Height-Zero");
			options.Height = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, options.Height, "Height-MaxValue");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Left = Int32.MinValue;
			}, "Left-MinValue");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Left = -1;
			}, "Left-Negative");
			options.Left = 0;
			Assert.AreEqual (0, options.Left, "Left-Zero");
			options.Left = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, options.Left, "Left-MaxValue");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Top = Int32.MinValue;
			}, "Top-MinValue");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Top = -1;
			}, "Top-Negative");
			options.Top = 0;
			Assert.AreEqual (0, options.Top, "Top-Zero");
			options.Top = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, options.Top, "Top-MaxValue");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Width = Int32.MinValue;
			}, "Width-MinValue");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				options.Width = -1;
			}, "Width-Negative");
			options.Width = 0;
			Assert.AreEqual (0, options.Width, "Width-Zero");
			options.Width = Int32.MaxValue;
			Assert.AreEqual (Int32.MaxValue, options.Width, "Width-MaxValue");
		}
	}
}

