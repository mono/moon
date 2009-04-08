//
// Unit tests for SolidColorBrush
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
	public partial class SolidColorBrushTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			Assert.AreEqual ("#00000000", scb.Color.ToString (), "Color");
			// SolidColorBrush's Transforms are non-null by default (false)
			BrushTest.CheckDefaults (scb, false);
		}

		[TestMethod]
		[MoonlightBug ("ML has the property Matrix frozen")]
		public void EnsureNotFrozen ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (scb.RelativeTransform as MatrixTransform), "RelativeTransform");
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (scb.Transform as MatrixTransform), "Transform");
		}

		[TestMethod]
		[MoonlightBug ("Looks like a bad SL2 bug")]
		public void Destructive ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			// from this instance we can change all default values
			BrushTest.DestructiveRelativeTransform (scb);
			BrushTest.DestructiveTransform (scb);
			// but it's safe to execute since we revert the changes
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void RelativeTransform ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			BrushTest.RelativeTransform (scb);
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void Transform ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			BrushTest.Transform (scb);
		}
	}
}
