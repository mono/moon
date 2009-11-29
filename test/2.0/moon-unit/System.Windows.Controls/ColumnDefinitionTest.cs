//
// Unit tests for ColumnDefinition
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
using System.Windows.Controls;
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ColumnDefinitionTest {

		private void CheckDefault (ColumnDefinition cd)
		{
			Assert.AreEqual (0.0d, cd.ActualWidth, "ActualWidth");
			Assert.AreEqual (1.0d, cd.Width.Value, "Width.Value");
			Assert.AreEqual (GridUnitType.Star, cd.Width.GridUnitType, "Width.GridUnitType");
			Assert.IsTrue (cd.Width.IsStar, "Width.IsStar");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxWidth, "MaxWidth");
			Assert.AreEqual (0.0d, cd.MinWidth, "MinWidth");
		}

		[TestMethod]
		public void DefaultValues ()
		{
			CheckDefault (new ColumnDefinition ());
		}

		private void AutoWidthCheck (ColumnDefinition cd)
		{
			Assert.AreEqual (0.0d, cd.ActualWidth, "ActualWidth");
			Assert.AreEqual (1.0d, cd.Width.Value, "Width.Value");
			Assert.AreEqual (GridUnitType.Auto, cd.Width.GridUnitType, "Width.GridUnitType");
			Assert.IsTrue (cd.Width.IsAuto, "Width.IsAuto");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxWidth, "MaxWidth");
			Assert.AreEqual (0.0d, cd.MinWidth, "MinWidth");
		}

		[TestMethod]
		public void AutoFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"Auto\"/>") as ColumnDefinition);
		}

		[TestMethod]
		public void AutoLowerFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"auto\"/>") as ColumnDefinition);
		}

		[TestMethod]
		public void AutoMixedFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"aUtO\"/>") as ColumnDefinition);
		}

		[TestMethod]
		public void StarFromXaml ()
		{
			CheckDefault (XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"*\"/>") as ColumnDefinition);
		}

		[TestMethod]
		public void IntStarFromXaml ()
		{
			ColumnDefinition cd = (ColumnDefinition) XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"2*\"/>");
			Assert.AreEqual (0.0d, cd.ActualWidth, "ActualWidth");
			Assert.AreEqual (GridUnitType.Star, cd.Width.GridUnitType, "Width.GridUnitType");
			Assert.AreEqual (2.0d, cd.Width.Value, "Width.Value");
			Assert.IsTrue (cd.Width.IsStar, "Width.IsStar");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxWidth, "MaxWidth");
			Assert.AreEqual (0.0d, cd.MinWidth, "MinWidth");
		}

		[TestMethod]
		public void IntFromXaml ()
		{
			ColumnDefinition cd = (ColumnDefinition) XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"3\"/>");
			Assert.AreEqual (0.0d, cd.ActualWidth, "ActualWidth");
			Assert.AreEqual (GridUnitType.Pixel, cd.Width.GridUnitType, "Width.GridUnitType");
			Assert.AreEqual (3.0d, cd.Width.Value, "Width.Value");
			Assert.IsTrue (cd.Width.IsAbsolute, "Width.IsAbsolute");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxWidth, "MaxWidth");
			Assert.AreEqual (0.0d, cd.MinWidth, "MinWidth");
		}

		[TestMethod]
		[SilverlightBug ("Width.Value is not initialised and can be any random value. No point in running the test.")]
		public void EmptyWidth ()
		{
			ColumnDefinition cd = (ColumnDefinition) XamlReader.Load ("<ColumnDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Width=\"\"/>");
			Assert.AreEqual (0.0d, cd.ActualWidth, "ActualWidth");
			Assert.AreEqual (GridUnitType.Pixel, cd.Width.GridUnitType, "Width.GridUnitType");
			// looks like SL2 did not initialize the value
			Assert.AreEqual (0, cd.Width.Value, "Width.Value");
			Assert.IsTrue (cd.Width.IsAbsolute, "Width.IsAbsolute");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxWidth, "MaxWidth");
			Assert.AreEqual (0.0d, cd.MinWidth, "MinWidth");
		}
	}
}
