//
// FontWeights Unit Tests
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
	public class FontWeightsTest {

		[TestMethod]
		public void HashCode ()
		{
			Assert.AreEqual (900, FontWeights.Black.GetHashCode (), "Black");
			Assert.AreEqual (700, FontWeights.Bold.GetHashCode (), "Bold");
			Assert.AreEqual (950, FontWeights.ExtraBlack.GetHashCode (), "ExtraBlack");
			Assert.AreEqual (800, FontWeights.ExtraBold.GetHashCode (), "ExtraBold");
			Assert.AreEqual (200, FontWeights.ExtraLight.GetHashCode (), "ExtraLight");
			Assert.AreEqual (300, FontWeights.Light.GetHashCode (), "Light");
			Assert.AreEqual (500, FontWeights.Medium.GetHashCode (), "Medium");
			Assert.AreEqual (400, FontWeights.Normal.GetHashCode (), "Normal");
			Assert.AreEqual (600, FontWeights.SemiBold.GetHashCode (), "SemiBold");
			Assert.AreEqual (100, FontWeights.Thin.GetHashCode (), "Thin");
		}
	}
}
