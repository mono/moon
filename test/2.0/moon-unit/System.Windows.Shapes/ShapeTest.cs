//
// Unit tests for Shape
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
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Shapes {
	[TestClass]
	public class ShapeTest {
		public class ConcreteShape : Shape {
			public Size _MeasureOveride (Size availableSize)
			{
				return MeasureOverride (availableSize);
			}

			public Size _ArrangeOverride (Size finalSize)
			{
				return ArrangeOverride (finalSize);
			}

			protected override Size MeasureOverride (Size availableSize)
			{
				Tester.WriteLine ("MeasureOverride input = " + availableSize.ToString ());
				Size output = base.MeasureOverride (availableSize);
				Tester.WriteLine ("MeasureOverride output = " + output.ToString ());
				return output;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				Tester.WriteLine ("ArrangeOverride input = " + finalSize.ToString ());
				Size output = base.MeasureOverride (finalSize);
				Tester.WriteLine ("ArrangeOverride output = " +  output.ToString ());
				return output;
			}
		}

		[TestMethod]
		public void MeasureTest ()
		{
			var shape = new ConcreteShape ();
			shape.Width = 25;
			shape.Height = 25;
			shape.Measure (new Size (50,50));
		}
	}
}
