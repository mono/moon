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
using System.Windows.Controls.Primitives;
using System.Windows.Documents;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class TextBlockTest : SilverlightTest {
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
			return (int) Math.Round (height / line_height);
		}
		
		[TestMethod]
		public void NullifyFontFamily ()
		{
			TextBlock tb = new TextBlock ();
			tb.FontFamily = null;
		}
		
		[TestMethod]
		public void LineBreakTranslatesToUnicodeLineSeparator ()
		{
			TextBlock tb = new TextBlock ();
			Run run = new Run ();
			string expected;
			
			run.Text = "this is line 1";
			tb.Inlines.Add (run);
			tb.Inlines.Add (new LineBreak ());
			run = new Run ();
			run.Text = "this is line 2";
			tb.Inlines.Add (run);
			
			expected = String.Format ("this is line 1{0}this is line 2", (char) 8232);
			
			Assert.AreEqual (expected, tb.Text, "LineBreak should be translated to a unicode line separator");
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("Font metrics are wrong")]
		public void LayoutCanvasInTreeTest ()
		{
			var parent = new Canvas ();
			var sub = new Border ();
			var tb = new TextBlock ();
			var sub_tb = new TextBlock ();
			sub_tb.Text = tb.Text = "The truth is Hidden";
			sub.Child = sub_tb;

			parent.Children.Add (tb);
			parent.Children.Add (sub);
			
			CreateAsyncTest (parent, () => {
					Assert.AreEqual (new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), new Size (tb.ActualWidth, tb.ActualHeight), "actual are equal");
					Assert.AreEqual (sub_tb.RenderSize, new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), "render size == actual size");
					Assert.AreNotEqual (sub_tb.RenderSize, tb.RenderSize, "rendersizes are notequal");
					Assert.AreEqual (new Size (0,0), tb.RenderSize, "tb 0,0 rendersize");
					Assert.IsNull (LayoutInformation.GetLayoutClip (tb), "tb null clip");
					Assert.IsNotNull (LayoutInformation.GetLayoutClip (sub_tb), "sub_tb has clip");
					Rect slot = LayoutInformation.GetLayoutSlot (sub_tb);
					Assert.IsTrue (Math.Round (sub_tb.ActualWidth) == slot.Width, "slot is rounded down from actual");
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("font metrics are wrong")]
		public void LayoutCanvasInTreeWrapTest ()
		{
			var parent = new Canvas ();
			var sub = new Border ();
			var tb = new TextBlock ();
			var sub_tb = new TextBlock ();
			sub_tb.Text = tb.Text = "The truth is Hidden";
			sub.Child = sub_tb;
			tb.TextWrapping = sub_tb.TextWrapping = TextWrapping.Wrap;

			parent.Children.Add (tb);
			parent.Children.Add (sub);
			
			// Notice that we don't wrap even though the slot is slightly
			// smaller than our actual

			CreateAsyncTest (parent, () => {
					Assert.IsTrue (tb.UseLayoutRounding, "use layout rounding");
					Assert.IsTrue (sub_tb.UseLayoutRounding, "use layout rounding");
					Assert.AreEqual (new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), new Size (tb.ActualWidth, tb.ActualHeight), "actual are equal");
					Assert.AreNotEqual (sub_tb.RenderSize, tb.RenderSize, "rendersizes are notequal");
					Assert.AreEqual (new Size (0,0), tb.RenderSize, "tb 0,0 rendersize");
					Assert.AreEqual (sub_tb.RenderSize, new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), "render size == actual size");
					Assert.IsNull (LayoutInformation.GetLayoutClip (tb), "tb null clip");
					Assert.IsNotNull (LayoutInformation.GetLayoutClip (sub_tb), "sub_tb has clip");
					Rect slot = LayoutInformation.GetLayoutSlot (sub_tb);
					Assert.IsTrue (Math.Round (sub_tb.ActualWidth) == slot.Width, "slot is rounded down from actual");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void LayoutCanvasInTreeWidth250Test ()
		{
			var parent = new Canvas ();
			var sub = new Border ();
			var tb = new TextBlock ();
			var sub_tb = new TextBlock ();
			sub_tb.Text = tb.Text = "The truth is Hidden";
			sub_tb.Width = tb.Width = 250;
			sub.Child = sub_tb;

			parent.Children.Add (tb);
			parent.Children.Add (sub);
			
			CreateAsyncTest (parent, () => {
					Assert.AreEqual (HorizontalAlignment.Stretch, tb.HorizontalAlignment, "tb horiz");
					Assert.AreEqual (HorizontalAlignment.Stretch, sub_tb.HorizontalAlignment, "tb horiz");
					Assert.AreEqual (new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), new Size (tb.ActualWidth, tb.ActualHeight), "actual are equal");
					Assert.IsTrue (sub_tb.ActualWidth < sub_tb.Width, "sub_tb.ActualWidth < sub_tb.Width");
					Assert.IsTrue (tb.ActualWidth < tb.Width, "tb Actualwidth < tb.Width");
					Assert.AreEqual (new Size (0,0), tb.RenderSize, "tb 0,0 rendersize");
					Assert.AreNotEqual (sub_tb.RenderSize, tb.RenderSize, "rendersizes are equal");
					Assert.IsNull (LayoutInformation.GetLayoutClip (tb), "tb null clip");
					Assert.IsNull (LayoutInformation.GetLayoutClip (sub_tb), "sub_tb has clip");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void LayoutCanvasInTreeBorderWidth250Test ()
		{
			var parent = new Canvas ();
			var sub = new Border ();
			var tb = new TextBlock ();
			var sub_tb = new TextBlock ();
			sub_tb.Text = tb.Text = "The truth is Hidden";
			sub.Width = tb.Width = 250;
			sub.Child = sub_tb;

			parent.Children.Add (tb);
			parent.Children.Add (sub);
			
			CreateAsyncTest (parent, () => {
					Assert.AreEqual (new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), new Size (tb.ActualWidth, tb.ActualHeight), "actual are equal");
					Assert.IsTrue (sub_tb.ActualWidth < sub.Width, "sub_tb.ActualWidth < sub(border).Width");
					Assert.IsTrue (tb.ActualWidth < tb.Width, "tb Actualwidth < tb.Width");
					Assert.AreEqual (new Size (0,0), tb.RenderSize, "tb 0,0 rendersize");
					Assert.AreNotEqual (sub_tb.RenderSize, tb.RenderSize, "rendersizes are equal");
					Assert.IsNull (LayoutInformation.GetLayoutClip (tb), "tb null clip");
					Assert.IsNull (LayoutInformation.GetLayoutClip (sub_tb), "sub_tb has clip");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void LayoutCanvasInTreeWidth30Test ()
		{
			var parent = new Canvas ();
			var sub = new Border ();
			var tb = new TextBlock ();
			var sub_tb = new TextBlock ();
			sub_tb.Text = tb.Text = "The truth is Hidden";
			sub_tb.Width = tb.Width = 30;
			sub.Child = sub_tb;

			parent.Children.Add (tb);
			parent.Children.Add (sub);
			
			CreateAsyncTest (parent, () => {
					Assert.AreEqual (new Size (sub_tb.ActualWidth, sub_tb.ActualHeight), new Size (tb.ActualWidth, tb.ActualHeight), "actual are equal");
					Assert.IsTrue (sub_tb.ActualWidth > sub_tb.Width, "sub_tb.ActualWidth > sub_tb.Width");
					Assert.IsTrue (tb.ActualWidth > tb.Width, "tb Actualwidth > tb.Width");
					Assert.AreEqual (new Size (0,0), tb.RenderSize, "tb 0,0 rendersize");
					Assert.AreNotEqual (sub_tb.RenderSize, tb.RenderSize, "rendersizes are equal");
					Assert.IsNull (LayoutInformation.GetLayoutClip (tb), "tb null clip");
					Assert.IsNotNull (LayoutInformation.GetLayoutClip (sub_tb), "sub_tb has clip");
			});
		}

		[TestMethod]
		public void SettingTextNeverCreatesMoreThanOneRun ()
		{
			TextBlock tb = new TextBlock ();
			string text;
			Run run;
			
			tb.Text = "this is line 1\rthis is line 2";
			run = (Run) tb.Inlines[0];
			Assert.AreEqual (1, tb.Inlines.Count, "1. Setting Text property should never create more than 1 Run");
			Assert.AreEqual ("this is line 1\rthis is line 2", run.Text, "1. The Run's Text should remain unchanged");
			
			tb.Text = "this is line 1\nthis is line 2";
			run = (Run) tb.Inlines[0];
			Assert.AreEqual (1, tb.Inlines.Count, "2. Setting Text property should never create more than 1 Run");
			Assert.AreEqual ("this is line 1\nthis is line 2", run.Text, "2. The Run's Text should remain unchanged");
			
			tb.Text = "this is line 1\r\nthis is line 2";
			run = (Run) tb.Inlines[0];
			Assert.AreEqual (1, tb.Inlines.Count, "3. Setting Text property should never create more than 1 Run");
			Assert.AreEqual ("this is line 1\r\nthis is line 2", run.Text, "3. The Run's Text should remain unchanged");
			
			// now try using the exact same line separator that LineBreak maps to
			tb.Inlines.Clear ();
			run.Text = "this is line 1";
			tb.Inlines.Add (run);
			tb.Inlines.Add (new LineBreak ());
			run = new Run ();
			run.Text = "this is line 2";
			tb.Inlines.Add (run);
			
			text = tb.Text;
			tb.ClearValue (TextBlock.TextProperty);
			
			tb.Text = text;
			run = (Run) tb.Inlines[0];
			Assert.AreEqual (1, tb.Inlines.Count, "4. Setting Text property should never create more than 1 Run");
			Assert.AreEqual (text, run.Text, "4. The Run's Text should remain unchanged");
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
		[MoonlightBug ("TextBlock.ActualHeight suggests Silverlight wraps on a per-char basis rather than wrapping to 44px")]
		public void MeasureTooLongLineWrapTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			tb.TextWrapping = TextWrapping.Wrap;

			b.Child = tb;
			b.Width = 44;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.IsTrue (tb.ActualWidth > 10.8 && tb.ActualWidth < 10.9, "textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "line count based on tb.ActualHeight");
			
			Assert.AreEqual (33, tb.DesiredSize.Width, "textblock.DesiredSize.Width is " + tb.DesiredSize.Width.ToString ());
			Assert.AreEqual (7, GetLineCount (tb.DesiredSize.Height), "line count based on tb.DesiredSize");
			//Assert.AreEqual (new Size (33,112), tb.DesiredSize, "tb.DesiredSize");
		}

		[TestMethod]
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
		[MoonlightBug ("Post-Border.Measure()'d TextBlock extents/wrapping are somehow wrong?")]
		public void ArrangeTooLongLineWrapMeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Text = "Hello and don't you forget Who I am";
			// notice this is on the border not the textblock
			b.Width = 44;
			
			Assert.IsTrue (tb.ActualWidth > 192.8 && tb.ActualWidth < 202.4, "1. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (1, GetLineCount (tb.ActualHeight), "1. line count based on textblock.ActualHeight");
			//Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "1. tb.ActualWidth is " + tb.ActualWidth.ToString ());
			//Assert.AreEqual (16, tb.ActualHeight, "1. tb.ActualHeight");
			
			//Console.WriteLine ("=========== Calling Border.Measure() ============");
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			//Console.WriteLine ("========= Done calling Border.Measure() =========");
			
			// FIXME: wrong after this point
			//
			// Debug output:
			//
			// =========== Calling Border.Measure() ============
			// TextBlock::MeasureOverride(availableSize = { 44.000000, inf })
			// TextBlock::Layout(constraint = { 44.000000, inf }) => 32.403809, 118.317073
			// ========= Done calling Border.Measure() =========
			//
			// Seems to me that the resulting ActualWidth/Height are correct, so what
			// exactly is Silverlight doing here?
			
			// note: need to fix ActualWidth values to be within moonlight's tolerance
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "2. tb.ActualHeight");
		}

		[TestMethod]
		[MoonlightBug ("Post-Border.Measure()'d TextBlock extents/wrapping are somehow wrong?")]
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
			
			//Console.WriteLine ("=========== Calling Border.Measure() ============");
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			//Console.WriteLine ("========= Done calling Border.Measure() =========");
			
			// FIXME: wrong after this point
			//
			// Debug output:
			//
			// =========== Calling Border.Measure() ============
			// TextBlock::MeasureOverride(availableSize = { 44.000000, inf })
			// TextBlock::Layout(constraint = { 44.000000, inf }) => 32.403809, 118.317073
			// ========= Done calling Border.Measure() =========
			//
			// Seems to me that the resulting ActualWidth/Height are correct, so what
			// exactly is Silverlight doing here?
			
			// note: need to fix ActualWidth values to be within moonlight's tolerance
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "2. textblock.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (35, GetLineCount (tb.ActualHeight), "2. line count based on textblock.ActualHeight");
			//Assert.AreEqual (560, tb.ActualHeight, "2. tb.ActualHeight");
		}

		[TestMethod]
		[MoonlightBug ("Border.Post-Measure()'d TextBlock extents/wrapping are somehow wrong?")]
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
			
			//Console.WriteLine ("=========== Calling Border.Measure() ============");
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			//Console.WriteLine ("========= Done calling Border.Measure() =========");
			
			// FIXME: wrong after this point
			//
			// Debug output:
			//
			// =========== Calling Border.Measure() ============
			// TextBlock::MeasureOverride(availableSize = { 44.000000, inf })
			// TextBlock::Layout(constraint = { 44.000000, inf }) => 32.403809, 118.317073
			// ========= Done calling Border.Measure() =========
			//
			// Seems to me that the resulting ActualWidth/Height are correct, so what
			// exactly is Silverlight doing here?
			
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
			
			//Console.WriteLine ("=========== Calling Border.Measure() ============");
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			//Console.WriteLine ("========= Done calling Border.Measure() =========");
			
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
