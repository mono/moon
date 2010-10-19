//
// Unit tests for System.Windows.Media.Imaging.WriteableBitmap
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
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media.Imaging {

	[TestClass]
	public class WriteableBitmapTest {

		[TestMethod]
		public void Ctor_BitmapSource ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new WriteableBitmap (null);
			}, "BitmapSource-null");

			BitmapSource bs = new WriteableBitmap (1, 2);
			WriteableBitmap wb = new WriteableBitmap (bs);
			Assert.AreEqual (2, wb.PixelHeight, "PixelHeight");
			Assert.AreEqual (1, wb.PixelWidth, "PixelWidth");
			Assert.AreEqual (2, wb.Pixels.Length, "Pixels");

			Assert.AreEqual (0, wb.Pixels [0], "0");
			Assert.AreEqual (0, wb.Pixels [1], "1");
		}

		[TestMethod]
		public void Ctor_IntInt ()
		{
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new WriteableBitmap (-1, 0);
			}, "pixelWidth-negative");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new WriteableBitmap (0, -1);
			}, "pixelHeight-negative");

			WriteableBitmap wb = new WriteableBitmap (0, 0);
			Assert.AreEqual (0, wb.PixelHeight, "PixelHeight");
			Assert.AreEqual (0, wb.PixelWidth, "PixelWidth");
			Assert.AreEqual (0, wb.Pixels.Length, "Pixels");

			Assert.Throws<OutOfMemoryException> (delegate {
				new WriteableBitmap (Int32.MaxValue, 1);
			}, "Int32.MaxValue");
			Assert.Throws<ArgumentException> (delegate {
				new WriteableBitmap (Int32.MaxValue, Int32.MaxValue);
			}, "Int32.MaxValue^2");
		}

		[TestMethod]
		[MoonlightBug ("Actual value is '11' while the expected value was '10'. Rectangle-PixelWidth")]
		public void Ctor_UIElementTransform ()
		{
			MatrixTransform transform = new MatrixTransform ();

			Assert.Throws<ArgumentNullException> (delegate {
				new WriteableBitmap (null, transform);
			}, "null,transform");

			// not a FrameworkElement (which defines Height and Width properties)
			UIElementTest.ConcreteUIElement ui = new UIElementTest.ConcreteUIElement ();

			// Transform is optional
			WriteableBitmap wb = new WriteableBitmap (ui, null);
			Assert.AreEqual (0, wb.PixelHeight, "UIElement-PixelHeight");
			Assert.AreEqual (0, wb.PixelWidth, "UIElement-PixelWidth");
			Assert.AreEqual (0, wb.Pixels.Length, "UIElement-Pixels");

			// FrameworkElement
			Rectangle r = new Rectangle ();
			wb = new WriteableBitmap (r, null);
			Assert.AreEqual (0, wb.PixelHeight, "Empty-Rectangle-PixelHeight");
			Assert.AreEqual (0, wb.PixelWidth, "Empty-Rectangle-PixelWidth");
			Assert.AreEqual (0, wb.Pixels.Length, "Empty-Rectangle-Pixels");

			r.Width = 10.9;
			r.Height = 20.1;
			wb = new WriteableBitmap (r, null);
			Assert.AreEqual (20, wb.PixelHeight, "Rectangle-PixelHeight");
			Assert.AreEqual (10, wb.PixelWidth, "Rectangle-PixelWidth");
			Assert.AreEqual (200, wb.Pixels.Length, "Rectangle-Pixels");

			ScaleTransform st = new ScaleTransform ();
			st.ScaleX = 2;
			st.ScaleY = 0.5;
			wb = new WriteableBitmap (r, st);
			Assert.AreEqual (10, wb.PixelHeight, "Scaled-Rectangle-PixelHeight");
			Assert.AreEqual (21, wb.PixelWidth, "Scaled-Rectangle-PixelWidth");
			Assert.AreEqual (210, wb.Pixels.Length, "Scaled-Rectangle-Pixels");
		}

		[TestMethod]
		public void Render ()
		{
			WriteableBitmap wb = new WriteableBitmap (0, 0);

			MatrixTransform transform = new MatrixTransform ();
			Assert.Throws<ArgumentNullException> (delegate {
				wb.Render (null, transform);
			}, "null,transform");

			Rectangle r = new Rectangle ();
			wb.Render (r, null);

			wb.Render (r, transform);
		}
	}
}

