//
// Unit tests for TransformGroup
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
	public partial class TransformGroupTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			TransformGroup tg = new TransformGroup ();
			Assert.AreEqual (0, tg.Children.Count, "Children");
			Assert.IsTrue (tg.Value.IsIdentity, "IsIdentity");
		}

		public void Value_Matrix (bool refresh)
		{
			MatrixTransform mt = new MatrixTransform ();

			TransformGroup tg = new TransformGroup ();
			tg.Children.Add (mt);
			Assert.AreEqual (1, tg.Children.Count, "Children-1");
			Assert.IsTrue (tg.Value.IsIdentity, "IsIdentity-1");

			mt.Matrix = new Matrix (2.0, 0.0, 0.0, 0.5, 0.0, 0.0);
			Assert.AreEqual (1, tg.Children.Count, "Children-2");
			// update was made inside TransformGroup...
			Assert.IsFalse ((tg.Children [0] as MatrixTransform).Matrix.IsIdentity, "IsIdentity-2");

			if (refresh) {
				// ... but Value is not refreshed if changed "indirectly" ...
				Assert.IsTrue (tg.Value.IsIdentity, "IsIdentity-3");
				Assert.IsFalse (mt.Matrix.Equals (tg.Value), "Matrix-3");
			}

			// ... unless the collection is changed
			tg.Children.Add (new MatrixTransform ());
			Assert.AreEqual (2, tg.Children.Count, "Children-4");
			Assert.IsTrue (mt.Matrix.Equals (tg.Value), "Matrix-4");
		}

		[TestMethod]
		public void Value_Matrix ()
		{
			Value_Matrix (false);
		}

		[TestMethod]
		[MoonlightBug ("ML always refresh the value")]
		public void Value_Matrix_Refresh ()
		{
			Value_Matrix (true);
		}

		public void Value_Scale (bool refresh)
		{
			ScaleTransform st = new ScaleTransform ();
			st.ScaleX = 2.0;
			st.ScaleY = 0.5;

			TransformGroup tg = new TransformGroup ();
			tg.Children.Add (st);
			Assert.AreEqual (1, tg.Children.Count, "Children-1");
			Assert.IsFalse (tg.Value.IsIdentity, "IsIdentity-1");
			Matrix expected1 = new Matrix (2.0, 0.0, 0.0, 0.5, 0.0, 0.0);
			Assert.IsTrue (expected1.Equals (tg.Value), "Matrix-1");

			st.ScaleY = 2.0;
			if (refresh) {
				Matrix expected2 = new Matrix (2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
				// if updated Value should be like expected2, but it's not
				Assert.IsFalse (expected2.Equals (tg.Value), "Matrix-2a");
				// still equals to the old computed value
				Assert.IsTrue (expected1.Equals (tg.Value), "Matrix-2b");
			}

			ScaleTransform st2 = new ScaleTransform ();
			st2.ScaleX = 0.5;
			st2.ScaleY = 0.5;
			tg.Children.Add (st2);

			Assert.AreEqual (2, tg.Children.Count, "Children-3");
			Assert.IsTrue (tg.Value.IsIdentity, "IsIdentity-3");
			Assert.IsTrue (Matrix.Identity.Equals (tg.Value), "Matrix-3");
		}

		[TestMethod]
		public void Value_Scale ()
		{
			Value_Scale (false);
		}

		[TestMethod]
		[MoonlightBug ("ML always refresh the value")]
		public void Value_Scale_Refresh ()
		{
			Value_Scale (true);
		}

		public void Value_Translate (bool refresh)
		{
			TranslateTransform tt = new TranslateTransform ();
			tt.X = -10;
			tt.Y = 250;

			TransformGroup tg = new TransformGroup ();
			tg.Children.Add (tt);
			Assert.AreEqual (1, tg.Children.Count, "Children-1");
			Assert.IsFalse (tg.Value.IsIdentity, "IsIdentity-1");
			Matrix expected1 = new Matrix (1.0, 0.0, 0.0, 1.0, -10.0, 250.0);
			Assert.IsTrue (expected1.Equals (tg.Value), "Matrix-1");

			tt.Y = -10;
			if (refresh) {
				Matrix expected2 = new Matrix (1.0, 0.0, 0.0, 1.0, -10.0, -10.0);
				// if updated Value should be like expected2, but it's not
				Assert.IsFalse (expected2.Equals (tg.Value), "Matrix-2a");
				// still equals to the old computed value
				Assert.IsTrue (expected1.Equals (tg.Value), "Matrix-2b");
			}

			TranslateTransform tt2 = new TranslateTransform ();
			tt2.X = 10;
			tt2.Y = 10;
			tg.Children.Add (tt2);

			Assert.AreEqual (2, tg.Children.Count, "Children-3");
			Assert.IsTrue (tg.Value.IsIdentity, "IsIdentity-3");
			Assert.IsTrue (Matrix.Identity.Equals (tg.Value), "Matrix-3");
		}

		[TestMethod]
		public void Value_Translate ()
		{
			Value_Translate (false);
		}

		[TestMethod]
		[MoonlightBug ("ML always refresh the value")]
		public void Value_Translate_Refresh ()
		{
			Value_Translate (true);
		}
	}
}
