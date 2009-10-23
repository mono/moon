//
// Unit tests for RowDefinition
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
	public partial class RowDefinitionTest {

		private void CheckDefault (RowDefinition cd)
		{
			Assert.AreEqual (0.0d, cd.ActualHeight, "ActualHeight");
			Assert.AreEqual (1.0d, cd.Height.Value, "Height.Value");
			Assert.AreEqual (GridUnitType.Star, cd.Height.GridUnitType, "Height.GridUnitType");
			Assert.IsTrue (cd.Height.IsStar, "Width.IsStar");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxHeight, "MaxHeight");
			Assert.AreEqual (0.0d, cd.MinHeight, "MinHeight");
		}

		[TestMethod]
		public void DefaultValues ()
		{
			CheckDefault (new RowDefinition ());
		}

		private void AutoWidthCheck (RowDefinition cd)
		{
			Assert.AreEqual (0.0d, cd.ActualHeight, "ActualHeight");
			Assert.AreEqual (1.0d, cd.Height.Value, "Width.Value");
			Assert.AreEqual (GridUnitType.Auto, cd.Height.GridUnitType, "Height.GridUnitType");
			Assert.IsTrue (cd.Height.IsAuto, "Height.IsAuto");
			Assert.AreEqual (Double.PositiveInfinity, cd.MaxHeight, "MaxHeight");
			Assert.AreEqual (0.0d, cd.MinHeight, "MinHeight");
		}

		[TestMethod]
		public void AutoFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"Auto\"/>") as RowDefinition);
		}

		[TestMethod]
		public void AutoUpperFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"AUTO\"/>") as RowDefinition);
		}

		[TestMethod]
		public void AutoMixedFromXaml ()
		{
			AutoWidthCheck (XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"AuTo\"/>") as RowDefinition);
		}

		[TestMethod]
		public void StarFromXaml ()
		{
			CheckDefault (XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"*\"/>") as RowDefinition);
		}

		[TestMethod]
		public void IntStarFromXaml ()
		{
			RowDefinition rd = (RowDefinition) XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"2*\"/>");
			Assert.AreEqual (0.0d, rd.ActualHeight, "ActualHeight");
			Assert.AreEqual (GridUnitType.Star, rd.Height.GridUnitType, "Height.GridUnitType");
			Assert.AreEqual (2.0d, rd.Height.Value, "Height.Value");
			Assert.IsTrue (rd.Height.IsStar, "Width.IsStar");
			Assert.AreEqual (Double.PositiveInfinity, rd.MaxHeight, "MaxHeight");
			Assert.AreEqual (0.0d, rd.MinHeight, "MinHeight");
		}

		[TestMethod]
		public void IntFromXaml ()
		{
			RowDefinition rd = (RowDefinition) XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"3\"/>");
			Assert.AreEqual (0.0d, rd.ActualHeight, "ActualHeight");
			Assert.AreEqual (GridUnitType.Pixel, rd.Height.GridUnitType, "Height.GridUnitType");
			Assert.AreEqual (3.0d, rd.Height.Value, "Height.Value");
			Assert.IsTrue (rd.Height.IsAbsolute, "Height.IsAbsolute");
			Assert.AreEqual (Double.PositiveInfinity, rd.MaxHeight, "MaxHeight");
			Assert.AreEqual (0.0d, rd.MinHeight, "MinHeight");
		}

		[TestMethod]
		[SilverlightBug ("Height.Value is not initialised and can be any random value. No point in running the test.")]
		public void EmptyHeight ()
		{
			RowDefinition rd = (RowDefinition) XamlReader.Load ("<RowDefinition xmlns=\"http://schemas.microsoft.com/client/2007\" Height=\"\"/>");
			Assert.AreEqual (0.0d, rd.ActualHeight, "ActualHeight");
			Assert.AreEqual (GridUnitType.Pixel, rd.Height.GridUnitType, "Height.GridUnitType");
			// looks like SL2 did not initialize the value
			Assert.AreEqual (0, rd.Height.Value, "Height.Value");
			Assert.IsTrue (rd.Height.IsAbsolute, "Height.IsAbsolute");
			Assert.AreEqual (Double.PositiveInfinity, rd.MaxHeight, "MaxHeight");
			Assert.AreEqual (0.0d, rd.MinHeight, "MinHeight");
		}
	}
}
