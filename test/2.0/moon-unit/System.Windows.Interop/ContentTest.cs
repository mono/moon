//
// Unit tests for System.Windows.Interop.Content
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.Windows;
using System.Windows.Interop;
using System.Threading;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Interop {

	[TestClass]
	public class ContentTest {

		void Check (Content content)
		{
			Assert.IsTrue (content.ActualHeight >= 0, "ActualHeight");
			Assert.IsTrue (content.ActualWidth >= 0, "ActualWidth");
			Assert.IsFalse (content.IsFullScreen, "IsFullScreen");
		}

		[TestMethod]
		public void New ()
		{
			Content content = new Content ();
			Check (content);
		}

		[TestMethod]
		public void Current ()
		{
			Content content = Application.Current.Host.Content;
			Check (content);
		}

		[TestMethod]
		public void IsFullScreen ()
		{
			Content content = new Content ();
			try {
				content.IsFullScreen = true;
				// only works from a user keyboard / mouse event
				Assert.IsFalse (content.IsFullScreen, "IsFullScreen");
			}
			finally {
				content.IsFullScreen = false;
			}
		}
	}
}
