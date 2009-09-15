//
// MessageBox Unit Tests
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
using System.Windows;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class MessageBoxTest {

		[TestMethod]
		public void InvalidValues ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				MessageBox.Show (null);
			}, "null");

			Assert.Throws<ArgumentNullException> (delegate {
				MessageBox.Show (null, "caption", MessageBoxButton.OK);
			}, "null,string,MessageBoxButton");
			Assert.Throws<ArgumentNullException> (delegate {
				MessageBox.Show ("messageBoxText", null, MessageBoxButton.OKCancel);
			}, "string,null,MessageBoxButton");
			Assert.Throws<ArgumentException> (delegate {
				MessageBox.Show ("messageBoxText", "caption", (MessageBoxButton)Int32.MinValue);
			}, "string,string,Int32.MinValue");
		}

		[TestMethod]
		[Ignore ("MessageBox does not requires to be called from a user event")]
		public void NoUserEvent ()
		{
			MessageBox.Show ("uho");
		}
	}
}

