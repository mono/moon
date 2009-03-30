//
// Unit tests for TextBlock
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
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class TextBlockTest {
		double line_height;
		
		[TestInitialize]
		public void Setup ()
		{
			TextBlock tb = new TextBlock ();
			tb.Text = "M";
			
			line_height = tb.ActualHeight;
		}
		
		int GetLineCount (double height)
		{
			return (int) (height / line_height);
		}
		
		[TestMethod]
		public void NullifyFontFamily ()
		{
			TextBlock tb = new TextBlock ();
			tb.FontFamily = null;
		}

		[TestMethod]
		//[MoonlightBug ("Extents are slightly off (27,17) instead of (28,16), likely due to font metrics")]
		public void MeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello";
			
			b.Child = tb;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "line count based on tb.ActualHeight");
			Assert.AreEqual (1, GetLineCount (tb.DesiredSize.Height), "line count based on tb.DesiredSize");
			//Assert.AreEqual (new Size (28,16), tb.DesiredSize, "tb.DesiredSize");
		}

		[TestMethod]
		//[MoonlightBug ("Extents are slightly off (104,34) instead of (107,32), likely due to font metrics")]
		public void MeasureNewlineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you\nforget Who I am";
			
			b.Child = tb;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (2, GetLineCount (tb.ActualHeight), "line count based on tb.ActualHeight");
			Assert.AreEqual (2, GetLineCount (tb.DesiredSize.Height), "line count based on tb.DesiredSize");
			//Assert.AreEqual (new Size (107,32), tb.DesiredSize, "tb.DesiredSize");
		}

		[TestMethod]
		//[MoonlightBug ("Extents are slightly off (44,17) instead of (44,16), likely due to font metrics")]
		public void MeasureTooLongLineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			
			b.Child = tb;
			b.Width = 44;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "line count based on tb.ActualHeight");
			Assert.AreEqual (1, GetLineCount (tb.DesiredSize.Height), "line count based on tb.DesiredSize");
			//Assert.AreEqual (new Size (44,16), tb.DesiredSize, "tb.DesiredSize");
		}

		[TestMethod]
		[MoonlightBug ("Extents are slightly off (32,118) instead of (32,112), likely due to font metrics")]
		public void MeasureTooLongLineWrapTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			tb.TextWrapping = TextWrapping.Wrap;

			b.Child = tb;
			b.Width = 44;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (7, GetLineCount (tb.ActualHeight), "line count based on tb.ActualHeight");
			Assert.AreEqual (7, GetLineCount (tb.DesiredSize.Height), "line count based on tb.DesiredSize");
			//Assert.AreEqual (new Size (33,112), tb.DesiredSize, "tb.DesiredSize");
		}

		[TestMethod]
		//[MoonlightBug ("Pre-Measure()'d Extents are slightly off likely due to font metrics, Post-Measure() extents off definitely due to font metrics")]
		public void ArrangeTooLongLineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			
			b.Child = tb;
			b.Width = 44;
			
			Assert.IsTrue (tb.ActualWidth > 192.8 && tb.ActualWidth < 202.4, "1. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "1. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			//Assert.AreEqual (16, tb.ActualHeight, "1. tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.IsTrue (tb.ActualWidth > 192.8 && tb.ActualWidth < 202.4, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			Assert.AreEqual (1, GetLineCount (tb.DesiredSize.Height), "2. line count based on textblock.DesiredSize");
			Assert.AreEqual (1, GetLineCount (b.DesiredSize.Height), "2. line count based on border.DesiredSize");
			//Assert.AreEqual (new Size (44,16), tb.DesiredSize, "2. tb.DesiredSize");
			//Assert.AreEqual (new Size (44,16), b.DesiredSize, "2. b.DesiredSize");
			//Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "2. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			//Assert.AreEqual (16, tb.ActualHeight, "2. tb.ActualHeight");

			b.Arrange (new Rect (0,0, b.DesiredSize.Width, b.DesiredSize.Height));
			
			Assert.IsTrue (tb.ActualWidth > 192.8 && tb.ActualWidth < 202.4, "3. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "3. line count based on textblock.ActualHeight");
			Assert.AreEqual (1, GetLineCount (tb.DesiredSize.Height), "3. line count based on textblock.DesiredSize");
			Assert.AreEqual (1, GetLineCount (b.ActualHeight), "3. line count based on border.ActualHeight");
			Assert.AreEqual (1, GetLineCount (b.DesiredSize.Height), "3. line count based on border.DesiredSize");
			//Assert.AreEqual (new Size (44,16), tb.DesiredSize, "3. tb.DesiredSize");
			//Assert.AreEqual (new Size (44,16), b.DesiredSize, "3. b.DesiredSize");
			//Assert.IsTrue (tb.ActualWidth > 202.3 && tb.ActualWidth < 202.4,"3. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			//Assert.AreEqual (16, tb.ActualHeight, "3. tb.ActualHeight");
			//Assert.AreEqual (new Size (44,16), new Size (b.ActualWidth, b.ActualHeight), "3. b.Actual*");
		}

		[TestMethod]
		[MoonlightBug ("Post-Measure()'d TextBlock extents/wrapping are wrong")]
		public void ArrangeTooLongLineWrapMeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Text = "Hello and don't you forget Who I am";
			// notice this is on the border not the textblock
			b.Width = 44;
			
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "1. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			//Assert.AreEqual (16, tb.ActualHeight, "1. tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			// FIXME: wrong after this point
			
			// note: need to fix ActualWidth values to be within moonlight's tolerance
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "2. tb.ActualHeight");
		}

		[TestMethod]
		[MoonlightBug ("Post-Measure()'d TextBlock extents/wrapping are wrong")]
		public void ArrangeTooLongLocal_LineWrapMeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Text = "Hello and don't you forget Who I am";
			// notice this is on the textblock not the border
			tb.Width = 44;

			Assert.IsTrue (tb.ActualWidth >= 32 && tb.ActualWidth < 34, "1. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (7, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.AreEqual (112, tb.ActualHeight, "1. tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			// FIXME: wrong after this point
			
			// note: need to fix ActualWidth values to be within moonlight's tolerance
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "2. tb.ActualHeight");
		}

		[TestMethod]
		[MoonlightBug ("Post-Measure() TextBlock extents/wrapping are wrong")]
		public void ArrangeTooLongLineWrapMeasureReverseTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			b.Width = 44;
			tb.Text = "Hello and don't you forget Who I am";

			Assert.IsTrue (tb.ActualWidth > 192.8 && tb.ActualWidth < 202.4, "1. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			// FIXME: wrong after this point
			
			// note: need to fix ActualWidth values to be within moonlight's tolerance
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "tb.ActualHeight1");
		}

		[TestMethod]
		[MoonlightBug ("TextBlock extents/wrapping are wrong after first Measure()")]
		public void ArrangeTooLongLocal_LineWrapMeasureReverseTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Width = 44;
			tb.Text = "Hello and don't you forget Who I am";

			Assert.IsTrue (tb.ActualWidth > 32 && tb.ActualWidth < 34, "1. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (7, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.AreEqual (112, tb.ActualHeight, "1. tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			// FIXME: wrong after this point
			
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "2. tb.ActualHeight");

			tb.Text = "Hello and don't you forget Who I am.";

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "3. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (36, GetLineCount (tb.ActualHeight), "3. line count based on textblock.ActualHeight");
			//Assert.AreEqual (576, tb.ActualHeight, "3. tb.ActualHeight");

			tb.Width = 70;
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "4. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (36, GetLineCount (tb.ActualHeight), "4. line count based on textblock.ActualHeight");
			//Assert.AreEqual (576, tb.ActualHeight, "4. tb.ActualHeight");

			b.InvalidateMeasure ();
			tb.InvalidateMeasure ();

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "5. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (36, GetLineCount (tb.ActualHeight), "5. line count based on textblock.ActualHeight");
			//Assert.AreEqual (576, tb.ActualHeight, "5. tb.ActualHeight");
		}

		[TestMethod]
		[MoonlightBug ("After setting Width/MaxWidth, Moonlight wraps while Silverlight doesn't")]
		public void ComputeActualWidth ()
		{
			var tb = new TextBlock ();
			
			Assert.AreEqual (new Size (0,0), tb.DesiredSize, "1. textblock.DesiredSize");
			Assert.AreEqual (new Size (0,0), new Size (tb.ActualWidth, tb.ActualHeight), "1. textblock.Actual");
			
			tb.Text = "Hello";

			Assert.AreEqual (new Size (0,0), tb.DesiredSize, "2. textblock.DesiredSize");
			Assert.IsTrue (tb.ActualWidth > 27.3 && tb.ActualWidth < 27.6, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (16, tb.ActualHeight, "2. tb.ActualHeight");

			tb.MaxWidth = 25;
			tb.Width = 50;
			tb.MinHeight = 33;
			
			// FIXME: ActualWidth/Height wrong after this point
			
			Assert.AreEqual (new Size (0,0), tb.DesiredSize, "3. textblock.DesiredSize");
			Assert.IsTrue (tb.ActualWidth > 27.3 && tb.ActualWidth < 27.6, "3. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "3. line count based on textblock.ActualHeight");
			//Assert.AreEqual (16, tb.ActualHeight, "3. tb.ActualHeight");

			tb.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), tb.DesiredSize, "4. textblock.DesiredSize");
			Assert.IsTrue (tb.ActualWidth > 27.3 && tb.ActualWidth < 27.6, "4. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "4. line count based on textblock.ActualHeight");
			//Assert.AreEqual (16, tb.ActualHeight, "4. tb.ActualHeight");
		}
	}
}
