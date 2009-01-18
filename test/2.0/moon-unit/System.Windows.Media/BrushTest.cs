//
// Unit tests for Brush
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

	//[TestClass]
	public class BrushTest {

		static public void CheckDefaults (Brush b)
		{
			Assert.AreEqual (1.0d, b.Opacity, "Opacity");
			MatrixTest.CheckIdentity ((b.RelativeTransform as MatrixTransform).Matrix, "RelativeTransform");
			MatrixTest.CheckIdentity ((b.Transform as MatrixTransform).Matrix, "Transform");
		}

		public class ConcreteBrush : Brush {
		}

		[TestMethod]
		[MoonlightBug ("Looks like an SL2 limitation that we don't have")]
		public void CannotInheritFromBrush ()
		{
			Assert.Throws<Exception> (delegate {
				new ConcreteBrush ();
			}, "we can't inherit from Brush");
		}
	}
}
