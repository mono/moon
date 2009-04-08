/*
 * MatrixTransform.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
using System;
using System.Windows.Markup;
using System.Windows.Media;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public partial class MatrixTransformTest
	{

		[TestMethod]
		public void Defaults ()
		{
			MatrixTransform mt = new MatrixTransform ();
			MatrixTest.CheckIdentity (mt.Matrix, "default");
		}

		[TestMethod]
		public void Set ()
		{
			MatrixTransform mt = new MatrixTransform ();
			mt.Matrix = new Matrix (1, 2, 3, 4, 5, 6);
			MatrixTest.CheckMatrix (mt.Matrix, 1, 2, 3, 4, 5, 6, "custom");
			Assert.IsFalse (mt.Matrix.IsIdentity, "IsNotIdentity");
		}

		[TestMethod]
		public void Brush_Defaults ()
		{
			SolidColorBrush scb = new SolidColorBrush ();
			MatrixTest.CheckIdentity ((scb.RelativeTransform as MatrixTransform).Matrix, "RelativeTransform");
			MatrixTest.CheckIdentity ((scb.Transform as MatrixTransform).Matrix, "Transform");
		}

		[TestMethod]
		public void LoadFromXamlTest ()
		{
			MatrixTransform m = (MatrixTransform) XamlReader.Load (
@"<MatrixTransform xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
	<MatrixTransform.Matrix >
	
	  <!-- OffsetX and OffsetY specify the position of the button,
	  M11 stretches it, and M12 skews it. -->
	  <Matrix OffsetX=""10"" OffsetY=""100"" M11=""3"" M12=""2"" M22=""0""/>
	</MatrixTransform.Matrix>
</MatrixTransform>");
			MatrixTest.CheckMatrix (m.Matrix, 3, 2, 0, 0, 10, 100, "xaml");
		}

		[TestMethod]
		public void PartialSetTest ()
		{
			MatrixTransform m = (MatrixTransform) XamlReader.Load (
@"<MatrixTransform xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
	<MatrixTransform.Matrix >
	  <Matrix OffsetX=""10"" M11=""5"" OffsetY=""100"" M12=""2""/>
	</MatrixTransform.Matrix>
</MatrixTransform>");
			MatrixTest.CheckMatrix (m.Matrix, 5, 2, 0, 1, 10, 100, "xaml");
		}

		// Check for a Moonlight issue which freeze the Matrix (throwing an UnautorizedException)
		public static bool CheckFreezer (MatrixTransform mt)
		{
			bool result = false;
			Matrix m = mt.Matrix;
			try {
				// NOTE: ML DO is frozen and can't be updated
				mt.Matrix = m;
				result = true;
			}
			catch (UnauthorizedAccessException) {
				// Moonlight known issue
			}
			finally {
				mt.Matrix = Matrix.Identity;
			}
			return result;
		}
	}
}
