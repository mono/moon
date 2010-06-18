//
// Unit tests for PrintDocument
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
using System.Windows.Printing;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Printing {

	[TestClass]
	public partial class PrintDocumentTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		public void NonUserInitiated ()
		{
			bool begin = false;
			bool end = false;
			bool print = false;

			PrintDocument pd = new PrintDocument ();
			pd.BeginPrint += delegate {
				begin = true;
			};
			pd.EndPrint += delegate {
				end = true;
			};
			pd.PrintPage += delegate {
				print = true;
			};

			Enqueue (() => {
				Assert.Throws<SecurityException> (delegate {
					pd.Print (null);
				}, "Print");
			});
			// no event is firedPrintDocument if call to Print was not user initiated
			EnqueueConditional (() => begin == end == print == false);
			EnqueueTestComplete ();
		}
	}
}

