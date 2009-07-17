//
// Unit tests for Glyphs
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
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Documents {

	[TestClass]
	public partial class GlyphsTest {

		static string GetXaml (string uri)
		{
			return @"<Glyphs xmlns=""http://schemas.microsoft.com/client/2007"" FontUri=""" + uri + @""" />";
		}

		// unexpected, but documented, the use of backslash is not accepted, at parse time, for FontUri

		[TestMethod]
		[MoonlightBug]
		public void FontUri_Backslash_Absolute ()
		{
			string uri = @"\\server\dir\font.ttf";
			Assert.AreEqual ("file://server/dir/font.ttf", new Uri (uri).ToString (), "Uri-UNC");
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (GetXaml (uri));
			}, "UNC");
		}

		[TestMethod]
		[MoonlightBug]
		public void FontUri_Backslash_Relative ()
		{
			string uri = @"\font.ttf";
			Assert.AreEqual (uri, new Uri (uri, UriKind.Relative).ToString (), "Uri-single-backslash");
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (GetXaml (uri));
			}, "single-backslash");
		}
	}
}

