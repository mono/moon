//
// Unit tests for TextSelection
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows.Documents;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Documents {

	[TestClass]
	public partial class TextSelectionTest {

		RichTextBox rtb;

		[TestInitialize]
		public void Setup ()
		{
			rtb = new RichTextBox ();
		}

		[TestMethod]
		public void SelectAllSetText_RichTextBox_treeStructure ()
		{
			Assert.AreEqual (0, rtb.Blocks.Count, "#0");

			rtb.SelectAll ();
			rtb.Selection.Text = "hi";

			Assert.AreEqual (1, rtb.Blocks.Count, "#1");
			Assert.AreEqual (typeof (Paragraph), rtb.Blocks[0].GetType(), "#2");
			Assert.AreEqual (1, ((Paragraph)rtb.Blocks[0]).Inlines.Count, "#3");
			Assert.AreEqual (typeof (Run), ((Paragraph)rtb.Blocks[0]).Inlines[0].GetType(), "#4");
			Assert.AreEqual ("hi", ((Run)((Paragraph)rtb.Blocks[0]).Inlines[0]).Text, "#5");
		}

		[TestMethod]
		public void SelectAll_setText_CompareStartEnd ()
		{
			rtb.SelectAll ();

			Assert.AreEqual (0, rtb.ContentStart.CompareTo (rtb.Selection.Start), "#1");
			Assert.AreEqual (0, rtb.ContentEnd.CompareTo (rtb.Selection.End), "#2");

			Assert.AreEqual (LogicalDirection.Backward, rtb.Selection.Start.LogicalDirection, "#3");
			Assert.AreEqual (LogicalDirection.Forward, rtb.Selection.End.LogicalDirection, "#4");

			Assert.AreNotEqual (rtb.Selection.Start, rtb.Selection.End, "#5");

			rtb.Selection.Text = "hi";

			Console.WriteLine (1);
			Assert.AreEqual (-1, rtb.ContentStart.CompareTo (rtb.Selection.Start), "#7");
			Console.WriteLine (2);
			Assert.AreEqual (-1, rtb.ContentStart.GetPositionAtOffset (1, LogicalDirection.Forward).CompareTo (rtb.Selection.Start), "#8");
			Console.WriteLine (3);
			Assert.AreEqual (-1, rtb.ContentStart.GetPositionAtOffset (2, LogicalDirection.Forward).CompareTo (rtb.Selection.Start), "#9");
			Console.WriteLine (4);
			Assert.AreEqual (-1, rtb.ContentStart.GetPositionAtOffset (3, LogicalDirection.Forward).CompareTo (rtb.Selection.Start), "#10");
			Console.WriteLine (5);
			Assert.AreEqual (0, rtb.ContentStart.GetPositionAtOffset (4, LogicalDirection.Forward).CompareTo (rtb.Selection.Start), "#11");

			Console.WriteLine (6);
			Assert.AreEqual (1, rtb.ContentEnd.CompareTo (rtb.Selection.End), "#12");
			Assert.AreEqual (1, rtb.ContentEnd.GetPositionAtOffset (-1, LogicalDirection.Forward).CompareTo (rtb.Selection.End), "#13");
			Assert.AreEqual (0, rtb.ContentEnd.GetPositionAtOffset (-2, LogicalDirection.Forward).CompareTo (rtb.Selection.End), "#14");

			Assert.AreEqual (typeof (Run), rtb.Selection.Start.Parent.GetType(), "#15");
			Assert.AreEqual (typeof (Run), rtb.Selection.End.Parent.GetType(), "#16");

			Run r = (Run)rtb.Selection.Start.Parent;
			Assert.AreEqual (0, r.ContentEnd.CompareTo (rtb.Selection.Start), "#17");
			Assert.AreEqual (0, r.ContentEnd.CompareTo (rtb.Selection.End), "#18");

			Assert.AreNotEqual (rtb.Selection.Start, rtb.Selection.End, "#19");
		}

		[TestMethod]
		public void SelectAll_Insert_CompareStartEnd ()
		{
		}

		[TestMethod]
		public void SelectionChangesAfterRemovingElement ()
		{
			rtb.SelectAll ();

			rtb.Selection.Text = "hello world";

			// at this point rtb.Selection.Start/End.Parent == the Run, and rtb has a single paragraph child
			Run r = rtb.Selection.Start.Parent as Run;
			Paragraph p = rtb.Blocks[0] as Paragraph;

			// select "ello world" -- both start and end are in the run
			rtb.Selection.Select (r.ContentStart.GetPositionAtOffset (1, LogicalDirection.Forward),
					      r.ContentEnd);

			Assert.AreEqual ("ello world", rtb.Selection.Text, "#1");
			Assert.AreEqual (@"&lt;Section xml:space=""preserve"" HasTrailingParagraphBreakOnPaste=""False"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">&lt;Paragraph FontSize=""11"" FontFamily=""Portable User Interface"" Foreground=""#FF000000"" FontWeight=""Normal"" FontStyle=""Normal"" FontStretch=""Normal"" TextAlignment=""Left"">&lt;Run Text=""ello world"" />&lt;/Paragraph>&lt;/Section>", rtb.Selection.Xaml.Replace ("<", "&lt;"), "#2");

			// remove the run
			p.Inlines.Remove (r);

			Assert.AreEqual (rtb.Selection.Start.Parent, rtb.Selection.End.Parent, "#3");
			Assert.AreEqual (p, rtb.Selection.Start.Parent, "#4");

			Assert.AreEqual ("", rtb.Selection.Text, "#5");
			Assert.AreEqual ("", rtb.Selection.Xaml, "#6");
		}
	}
}