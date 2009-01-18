//
// Unit tests for Matrix
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
	public class MatrixTest {

		static public void CheckMatrix (Matrix m, double m11, double m12, double m21, double m22, double ox, double oy, string message)
		{
			Assert.AreEqual (m11, m.M11, message + ".M11");
			Assert.AreEqual (m12, m.M12, message + ".M12");
			Assert.AreEqual (m21, m.M21, message + ".M21");
			Assert.AreEqual (m22, m.M22, message + ".M22");
			Assert.AreEqual (ox, m.OffsetX, message + ".OffsetX");
			Assert.AreEqual (oy, m.OffsetY, message + ".OffsetY");
			Assert.AreEqual (m.IsIdentity, m.Equals (Matrix.Identity), "Equals(Identity)");
		}

		static public void CheckIdentity (Matrix m, string message)
		{
			CheckMatrix (m, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, message);
			Assert.IsTrue (m.IsIdentity, message + ".IsIdentity");
		}

		[TestMethod]
		public void Default ()
		{
			Matrix m = new Matrix ();
			CheckIdentity (m, "default");
			Assert.AreEqual ("Identity", m.ToString (), "ToString");

			Point original = new Point (0, 0);
			Point result = m.Transform (original);
			Assert.AreEqual (original, result, "Transform w/Identity");
		}

		[TestMethod]
		public void Identity ()
		{
			CheckIdentity (Matrix.Identity, "default");
			Assert.AreEqual ("Identity", Matrix.Identity.ToString (), "ToString");

			Point original = new Point (0, 0);
			Point result = Matrix.Identity.Transform (original);
			Assert.AreEqual (original, result, "Transform w/Identity");

			// a new Identity matrix is already returned because the value is not readonly
			Assert.IsFalse (Object.ReferenceEquals (Matrix.Identity, Matrix.Identity), "Same");
		}

		[TestMethod]
		public void Custom ()
		{
			Matrix m = new Matrix (1, 2, 3, 4, 5, 6);
			CheckMatrix (m, 1, 2, 3, 4, 5, 6, "custom");
			Assert.IsFalse (m.IsIdentity, "IsNotIdentity");
			Assert.AreEqual ("1,2,3,4,5,6", m.ToString (), "ToString");
		}

		[TestMethod]
		public void Transform ()
		{
			Matrix m = new Matrix (1, 2, 3, 4, 5, 6);
			Point result = m.Transform (new Point (0, 0));
			Assert.AreEqual (new Point (5, 6), result, "Transform/0,0/Translation only");
			result = m.Transform (new Point (1, 1));
			Assert.AreEqual (new Point (9, 12), result, "Transform/1,1");
			result = m.Transform (new Point (-2, 3));
			Assert.AreEqual (new Point (12, 14), result, "Transform/-2,3");
		}

		[TestMethod]
		public void TransformUsingNonInvertibleMatrix ()
		{
			Matrix m = new Matrix (123, 24, 82, 16, 47, 30);
			Point result = m.Transform (new Point (0, 0));
			Assert.AreEqual (new Point (47, 30), result, "Transform/0,0/Translation only");
			result = m.Transform (new Point (1, 1));
			Assert.AreEqual (new Point (252, 70), result, "Transform/1,1");
			result = m.Transform (new Point (-2, 3));
			Assert.AreEqual (new Point (47, 30), result, "Transform/-2,3");
		}
	}
}
