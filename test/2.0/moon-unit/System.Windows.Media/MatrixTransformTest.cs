/*
 * MatrixTrasformTest.cs.
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
	public class MatrixTest
	{
		[TestMethod]
		[MoonlightBug]
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
			Assert.AreEqual (10, m.Matrix.OffsetX, "#1");
			Assert.AreEqual (100, m.Matrix.OffsetY, "#2");
			Assert.AreEqual (3, m.Matrix.M11, "#3");
			Assert.AreEqual (2, m.Matrix.M12, "#4");
			Assert.AreEqual (0, m.Matrix.M21, "#5");
			Assert.AreEqual (0, m.Matrix.M22, "#6"); 
		}

		[TestMethod]
		[MoonlightBug ("Matrix is instantiated as an Identity matrix, but all elements need to be set to zero if unspecified")]
		public void PartialSetTest ()
		{
			MatrixTransform m = (MatrixTransform) XamlReader.Load (
@"<MatrixTransform xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
	<MatrixTransform.Matrix >
	  <Matrix OffsetX=""10"" M11=""5"" OffsetY=""100"" M12=""2""/>
	</MatrixTransform.Matrix>
</MatrixTransform>");
			Assert.AreEqual (10, m.Matrix.OffsetX, "#1");
			Assert.AreEqual (100, m.Matrix.OffsetY, "#2");
			Assert.AreEqual (5, m.Matrix.M11, "#3");
			Assert.AreEqual (2, m.Matrix.M12, "#4");
			Assert.AreEqual (0, m.Matrix.M21, "#5");
			Assert.AreEqual (1, m.Matrix.M22, "#6"); 
		}
	}
}
