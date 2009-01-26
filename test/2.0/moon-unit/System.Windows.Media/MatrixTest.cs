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

		class MatrixFormatter : IFormatProvider, ICustomFormatter {

			public object GetFormat (Type formatType)
			{
				return (formatType == typeof (ICustomFormatter)) ? this : null;
			}

			public string Format (string format, object arg, IFormatProvider formatProvider)
			{
				CallCount++;
				Assert.AreEqual (this, formatProvider, "formatProvider");
				if (arg.Equals (','))
					return "#";

				Assert.IsTrue (arg is double, "arg");
				int n = (int) (double) arg;
				switch (n) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					if (format == null)
						return String.Format ("[{0}]", n);
					if (format.Length == 0)
						return "@";
					Assert.Fail (n.ToString ());
					return null;
				default:
					Assert.Fail (n.ToString ());
					return null;
				}
			}

			static public int CallCount = 0;
		}

		[TestMethod]
		public void ToStringIFormatProvider ()
		{
			Matrix m = new Matrix (1, 2, 3, 4, 5, 6);
			MatrixFormatter.CallCount = 0;
			Assert.AreEqual ("1,2,3,4,5,6", m.ToString (null), "null");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("[1]#[2]#[3]#[4]#[5]#[6]", m.ToString (new MatrixFormatter ()), "MatrixFormatter");
			// 11 times: one per double (6) and 5 for ','
			Assert.AreEqual (11, MatrixFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void ToStringIFormatProvider_Identity ()
		{
			Matrix m = new Matrix ();
			MatrixFormatter.CallCount = 0;
			Assert.AreEqual ("Identity", m.ToString (null), "null");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("Identity", m.ToString (new MatrixFormatter ()), "MatrixFormatter");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void ToStringIFormattable ()
		{
			Matrix m = new Matrix (15, 14, 13, 12, 11, 10);
			MatrixFormatter.CallCount = 0;
			IFormattable f = (m as IFormattable);
			Assert.AreEqual ("15,14,13,12,11,10", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("[15]#[14]#[13]#[12]#[11]#[10]", f.ToString (null, new MatrixFormatter ()), "null,MatrixFormatter");
			Assert.AreEqual (11, MatrixFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("1.500000e+001,1.400000e+001,1.300000e+001,1.200000e+001,1.100000e+001,1.000000e+001", f.ToString ("e", null), "e,null");
			Assert.AreEqual (11, MatrixFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("[15]#[14]#[13]#[12]#[11]#[10]", f.ToString (String.Empty, new MatrixFormatter ()), "Empty,MatrixFormatter");
			Assert.AreEqual (22, MatrixFormatter.CallCount, "CallCount-d");
		}

		[TestMethod]
		public void ToStringIFormattable_Identity ()
		{
			Matrix m = new Matrix ();
			MatrixFormatter.CallCount = 0;
			IFormattable f = (m as IFormattable);
			Assert.AreEqual ("Identity", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("Identity", f.ToString (null, new MatrixFormatter ()), "null,MatrixFormatter");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("Identity", f.ToString ("e", null), "e,null");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("Identity", f.ToString (String.Empty, new MatrixFormatter ()), "Empty,MatrixFormatter");
			Assert.AreEqual (0, MatrixFormatter.CallCount, "CallCount-d");
		}
	}
}
