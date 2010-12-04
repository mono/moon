//
// FontWeights Unit Tests
//
// Author:
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
// Copyright 2008 Novell, Inc. (http://www.novell.com)
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
using System.Windows.Controls;
using System.Windows.Markup;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class FontWeightsTest {

		[TestMethod]
		public void HashCode ()
		{
			Assert.AreEqual (900, FontWeights.Black.GetHashCode (), "Black");
			Assert.AreEqual (700, FontWeights.Bold.GetHashCode (), "Bold");
			Assert.AreEqual (950, FontWeights.ExtraBlack.GetHashCode (), "ExtraBlack");
			Assert.AreEqual (800, FontWeights.ExtraBold.GetHashCode (), "ExtraBold");
			Assert.AreEqual (200, FontWeights.ExtraLight.GetHashCode (), "ExtraLight");
			Assert.AreEqual (300, FontWeights.Light.GetHashCode (), "Light");
			Assert.AreEqual (500, FontWeights.Medium.GetHashCode (), "Medium");
			Assert.AreEqual (400, FontWeights.Normal.GetHashCode (), "Normal");
			Assert.AreEqual (600, FontWeights.SemiBold.GetHashCode (), "SemiBold");
			Assert.AreEqual (100, FontWeights.Thin.GetHashCode (), "Thin");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void XamlInteger_sl4 ()
		{
			//
			// if the textblock is in a managed control like the grid,
			// its legal to set FontWeights with an int.
			//

			Grid grid = XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							       	<TextBlock x:Name=""the_box"" FontWeight=""600"" />
							  </Grid>") as Grid;

			Assert.IsNotNull (grid, "a1");

			TextBlock block = grid.FindName ("the_box") as TextBlock;

			Assert.IsNotNull (block, "a2");
			Assert.AreEqual (FontWeights.SemiBold, block.FontWeight, "a3");


			//
			// But we can't just do this normally.
			//

			string bad_xaml = @"<TextBlock xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
						        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
					       FontWeight=""600"" />";
			Assert.Throws<XamlParseException> (() => XamlReader.Load (bad_xaml));
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		[MoonlightBug ("Invalid attribute value 600 for property FontWeight.")]
		public void XamlInteger_sl3 ()
		{
			//
			// if the textblock is in a managed control like the grid,
			// its legal to set FontWeights with an int.
			//

			Grid grid = XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							       	<TextBlock x:Name=""the_box"" FontWeight=""600"" />
							  </Grid>") as Grid;

			Assert.IsNotNull (grid, "a1");

			TextBlock block = grid.FindName ("the_box") as TextBlock;

			Assert.IsNotNull (block, "a2");
			Assert.AreEqual (FontWeights.SemiBold, block.FontWeight, "a3");


			//
			// But we can't just do this normally.
			//

			string bad_xaml = @"<TextBlock xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
						        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
					       FontWeight=""600"" />";
			Assert.Throws<XamlParseException> (() => XamlReader.Load (bad_xaml));
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		public void UintRDStorage_sl2 ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ResourceDictionary>
    <FontWeight x:Key=""fontWeight"">Bold</FontWeight>
  </ResourceDictionary>
</Canvas.Resources>
</Canvas>");

			object o = c.Resources["fontWeight"];

			Assert.AreEqual (typeof (uint), o.GetType(), "#0");

			c.Resources.Add ("fontWeight2", FontWeights.Bold);

			o = c.Resources["fontWeight2"];

			Assert.AreEqual (typeof (FontWeight), o.GetType(), "#1");
		}

		[TestMethod]
		[MaxRuntimeVersion(2)]
		public void PropertySetter_sl2 ()
		{
			XamlReader.Load (@"<TextBlock xmlns=""http://schemas.microsoft.com/client/2007"" FontWeight=""Bold"" />");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void UintRDStorage_sl3 ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ResourceDictionary>
    <FontWeight x:Key=""fontWeight"">Bold</FontWeight>
  </ResourceDictionary>
</Canvas.Resources>
</Canvas>");

			object o = c.Resources["fontWeight"];

			Assert.AreEqual (typeof (FontWeight), o.GetType(), "#0");
		}

		[TestMethod]
		[MinRuntimeVersion(3)]
		public void PropertySetter_sl3 ()
		{
			XamlReader.Load (@"<TextBlock xmlns=""http://schemas.microsoft.com/client/2007"" FontWeight=""Bold"" />");
		}

	}
}
