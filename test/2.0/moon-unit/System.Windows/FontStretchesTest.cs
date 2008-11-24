//
// FontStretches Unit Tests
//
// Author:
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
// Copyright 2008 Novell, Inc. (http://www.novell.com)
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
	public class FontStretchesTest {

		[TestMethod]
		public void HashCode ()
		{
			Assert.AreEqual (3, FontStretches.Condensed.GetHashCode (), "Condensed");
			Assert.AreEqual (7, FontStretches.Expanded.GetHashCode (), "Expanded");
			Assert.AreEqual (2, FontStretches.ExtraCondensed.GetHashCode (), "ExtraCondensed");
			Assert.AreEqual (8, FontStretches.ExtraExpanded.GetHashCode (), "ExtraExpanded");
			Assert.AreEqual (5, FontStretches.Normal.GetHashCode (), "Normal");
			Assert.AreEqual (4, FontStretches.SemiCondensed.GetHashCode (), "SemiCondensed");
			Assert.AreEqual (6, FontStretches.SemiExpanded.GetHashCode (), "SemiExpanded");
			Assert.AreEqual (1, FontStretches.UltraCondensed.GetHashCode (), "UltraCondensed");
			Assert.AreEqual (9, FontStretches.UltraExpanded.GetHashCode (), "UltraExpanded");
		}
	}
}
