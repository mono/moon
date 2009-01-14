//
// Unit tests for LinearGradientBrush
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
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public class LinearGradientBrushTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			LinearGradientBrush lgb = new LinearGradientBrush ();

			Assert.AreEqual (1.0d, lgb.EndPoint.X, "EndPoint.X");
			Assert.AreEqual (1.0d, lgb.EndPoint.Y, "EndPoint.Y");

			CheckDefaults (lgb);
		}

		[TestMethod]
		public void AngleCtor ()
		{
			for (int i=0; i <= 360; i++) {
				LinearGradientBrush lgb = new LinearGradientBrush (null, i);
				Assert.IsTrue (Math.Abs (Math.Cos (i * Math.PI / 180) - lgb.EndPoint.X) < 0.0001, i.ToString () + "-EndPoint.X");
				Assert.IsTrue (Math.Abs (Math.Sin (i * Math.PI / 180) - lgb.EndPoint.Y) < 0.0001, i.ToString () + "-EndPoint.Y");
				CheckDefaults (lgb);
			}
		}

		static public void CheckDefaults (LinearGradientBrush lgb)
		{
			Assert.AreEqual (0.0d, lgb.StartPoint.X, "Start.X");
			Assert.AreEqual (0.0d, lgb.StartPoint.Y, "Start.Y");
			GradientBrushTest.CheckDefaults (lgb);
		}
	}
}
