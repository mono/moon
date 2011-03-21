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
using System.Windows.Controls;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public class BrushTest : SilverlightTest {

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

		[TestMethod]
		public void UseTwice()
		{
			string name = "SuperSecretBrush";
			Brush b1 = new SolidColorBrush();
			b1.SetValue(FrameworkElement.NameProperty, name);

			var g1 = new Grid { Name = "g1", Background = b1 };
			var g2 = new Grid { Name = "g2", Background = b1 };

			TestPanel.Children.Add(g1);
			Assert.IsNull(TestPanel.FindName(name), "#1");

			TestPanel.Children.Add(g2);
			Assert.IsNull(TestPanel.FindName(name), "#2");
		}

		[TestMethod]
		public void UseTwice_ThenOnce()
		{
			string name = "SuperSecretBrush";
			Brush brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, name);

			var g1 = new Grid { Name = "g1", Background = brush };
			var g2 = new Grid { Name = "g2", Background = brush };

			TestPanel.Children.Add(g1);
			Assert.IsNull(TestPanel.FindName(name), "#1");

			g2.Background = null;
			Assert.IsNull(TestPanel.FindName(name), "#2");

			TestPanel.Children.Clear();
			TestPanel.Children.Add(g1);
			Assert.AreSame(brush, TestPanel.FindName(name), "#3");
		}

		static public void DestructiveRelativeTransform (Brush b)
		{
			SolidColorBrush old_scb = new SolidColorBrush ();

			try {
				MatrixTransform mt = (b.RelativeTransform as MatrixTransform);
				Assert.IsTrue (mt.Matrix.IsIdentity, "Original/Identity");

				// note: this is DESTRUCTIVE (hence the finally clause)
				mt.Matrix = new Matrix (1, 2, 3, 4, 5, 6);
				Assert.IsFalse (mt.Matrix.IsIdentity, "New/NonIdentity");

				// *ALL* new brushes will have this new Matrix by default
				Assert.IsTrue (CompareMatrix (mt.Matrix, new SolidColorBrush ().RelativeTransform), "SolidColorBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new LinearGradientBrush ().RelativeTransform), "LinearGradientBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new RadialGradientBrush ().RelativeTransform), "RadialGradientBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new VideoBrush ().RelativeTransform), "VideoBrush");
				// Note: ImageBrush is a special case where RelativeTransform is, by default, null

				// even existing brush use it (very like shared)
				Assert.IsTrue (CompareMatrix (mt.Matrix, old_scb.RelativeTransform), "Old/SolidColorBrush");
			}
			finally {
				(b.RelativeTransform as MatrixTransform).Matrix = Matrix.Identity;

				// everything gets back to normal after we revert the change
				Assert.IsTrue ((old_scb.RelativeTransform as MatrixTransform).Matrix.IsIdentity, "Revert/Identity");
			}
		}

		static public void DestructiveTransform (Brush b)
		{
			SolidColorBrush old_scb = new SolidColorBrush ();

			try {
				MatrixTransform mt = (b.Transform as MatrixTransform);
				Assert.IsTrue (mt.Matrix.IsIdentity, "Original/Identity");

				// note: this is DESTRUCTIVE (hence the finally clause)
				mt.Matrix = new Matrix (1, 2, 3, 4, 5, 6);
				Assert.IsFalse (mt.Matrix.IsIdentity, "New/NonIdentity");

				// *ALL* new brushes will have this new Matrix by default
				Assert.IsTrue (CompareMatrix (mt.Matrix, new SolidColorBrush ().Transform), "SolidColorBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new LinearGradientBrush ().Transform), "LinearGradientBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new RadialGradientBrush ().Transform), "RadialGradientBrush");
				Assert.IsTrue (CompareMatrix (mt.Matrix, new VideoBrush ().Transform), "VideoBrush");
				// Note: ImageBrush is a special case where Transform is, by default, null

				// even existing brush use it (very like shared)
				Assert.IsTrue (CompareMatrix (mt.Matrix, old_scb.RelativeTransform), "Old/SolidColorBrush");
			}
			finally {
				(b.Transform as MatrixTransform).Matrix = Matrix.Identity;

				// everything gets back to normal after we revert the change
				Assert.IsTrue ((old_scb.RelativeTransform as MatrixTransform).Matrix.IsIdentity, "Revert/Identity");
			}
		}

		static bool CompareMatrix (Matrix expected, Transform actual)
		{
			return (actual as MatrixTransform).Matrix.Equals (expected);
		}

		static public void RelativeTransform (Brush b)
		{
			Assert.IsNotNull (b.RelativeTransform, "RelativeTransform");

			MatrixTransform mt = (b.RelativeTransform as MatrixTransform);
			Matrix m = mt.Matrix;
			m.M11 = 2.0;
			Assert.IsTrue (mt.Matrix.IsIdentity, "OldCopy/Identity");

			b.RelativeTransform = null;
			Assert.IsNotNull (b.RelativeTransform, "RelativeTransform/NeverNull");
			Assert.IsTrue ((b.RelativeTransform as MatrixTransform).Matrix.IsIdentity, "RelativeTransform/Null/Identity");

			b.SetValue (Brush.RelativeTransformProperty, null);
			Assert.IsNotNull (b.RelativeTransform, "RelativeTransform/NeverNullDP");
		}

		static public void Transform (Brush b)
		{
			Assert.IsNotNull (b.Transform, "Transform");

			MatrixTransform mt = (b.Transform as MatrixTransform);
			Matrix m = mt.Matrix;
			m.M11 = 2.0;
			Assert.IsTrue (mt.Matrix.IsIdentity, "OldCopy/Identity");

			b.Transform = null;
			Assert.IsNotNull (b.Transform, "Transform/NeverNull");
			Assert.IsTrue ((b.Transform as MatrixTransform).Matrix.IsIdentity, "Transform/Null/Identity");

			b.SetValue (Brush.TransformProperty, null);
			Assert.IsNotNull (b.Transform, "Transform/NeverNullDP");
		}
	}
}
