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
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Documents {

	[TestClass]
	public partial class TextSelectionTest : SilverlightTest {

		RichTextBox rtb;

		[TestInitialize]
		public void Setup ()
		{
			rtb = new RichTextBox ();
		}

		[TestMethod]
		[MoonlightBug]
		public void Defaults ()
		{
			TextSelection ts = rtb.Selection;

			Assert.AreEqual (String.Empty, ts.Text, "Text");
			Assert.AreEqual (String.Empty, ts.Xaml, "Xaml");

			Assert.AreEqual (0, ts.Start.CompareTo (rtb.ContentStart), "Start");
			Assert.AreEqual (0, ts.End.CompareTo (rtb.ContentStart), "End");
		}

		[TestMethod]
		[MoonlightBug]
		public void Text_Set_Null ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.Selection.Text = null;
			}, "null");
		}

		[TestMethod]
		[MoonlightBug]
		public void StartEndAfterSetText ()
		{
			TextSelection ts = rtb.Selection;

			// before we've done anything with the textbox, our selection start/end are equivalent to rtb.ContentStart
			Assert.AreEqual (0, ts.Start.CompareTo (rtb.ContentStart), "#0");
			Assert.AreEqual (0, ts.End.CompareTo (rtb.ContentStart), "#1");

			ts.Text = "Hello";

			// after inserting some text, selection start/end are both after ContentStart
			Assert.AreEqual (1, ts.Start.CompareTo (rtb.ContentStart), "#2");
			Assert.AreEqual (1, ts.End.CompareTo (rtb.ContentStart), "#3");

			// but they're still the same (presumably they both are the run's ContentStart?)
			Assert.AreEqual (0, ts.Start.CompareTo (ts.End), "#4");
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void Text ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moon";
			Assert.AreEqual (String.Empty, ts.Text, "#0");

			rtb.SelectAll ();
			Assert.AreEqual ("Moon", rtb.Selection.Text, "#1");
			Assert.AreEqual ("Moon", ts.Text, "#2");
			rtb = new RichTextBox ();

			ts = rtb.Selection;
			CreateAsyncTest (rtb,
					 () => {
						 ts.Text = "Hello";
						 Assert.AreEqual (String.Empty, ts.Text, "#3");
					 },
					 () => {
						 Assert.AreEqual (String.Empty, ts.Text, "#4");
						 Assert.AreEqual (String.Empty, rtb.Selection.Text, "#5");
						 rtb.SelectAll ();
						 Assert.AreEqual ("Hello", rtb.Selection.Text, "#6");
					 });
		}

		[TestMethod]
		[MoonlightBug]
		public void TextPointersAreSnapshots ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moon";

			rtb.SelectAll ();

			TextPointer start = ts.Start;
			TextPointer end = ts.End;

			ts.Select (ts.Start.GetPositionAtOffset (1, LogicalDirection.Backward), ts.End.GetPositionAtOffset (-1, LogicalDirection.Forward));
			Assert.AreEqual (-1, start.CompareTo (ts.Start), "#1");
			Assert.AreEqual (1, end.CompareTo (ts.End), "#2");
		}

		[TestMethod]
		[MoonlightBug]
		public void TextPointersUpdatePosition_insert ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moonlight";

			rtb.SelectAll ();

			TextPointer start = ts.Start;
			TextPointer insert = ts.Start;

			start = start.GetPositionAtOffset (4, LogicalDirection.Backward);
			insert = insert.GetPositionAtOffset (3, LogicalDirection.Backward);

			ts.Select (insert, insert);

			ts.Text = "1";

			ts.Select (start, start);

			ts.Text = "2";

			rtb.SelectAll ();

			Assert.AreEqual ("Moo1n2light", ts.Text, "#0");
		}

		[TestMethod]
		[MoonlightBug]
		public void TextPointersUpdatePosition_clear ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moonlight";

			rtb.SelectAll ();

			TextPointer start = ts.Start;
			TextPointer insert = ts.Start;

			start = start.GetPositionAtOffset (4, LogicalDirection.Backward);
			insert = insert.GetPositionAtOffset (3, LogicalDirection.Backward);

			ts.Select (insert, insert.GetPositionAtOffset (1, LogicalDirection.Forward));

			ts.Text = "";

			ts.Select (start, start);

			ts.Text = "2";

			rtb.SelectAll ();

			Assert.AreEqual ("Moo2light", ts.Text, "#0");
		}

		[TestMethod]
		[MoonlightBug]
		public void TextPointersUpdatePosition_clear2 ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moonlight";

			rtb.SelectAll ();

			TextPointer pos = ts.Start.GetPositionAtOffset (4, LogicalDirection.Backward);

			Run r = (Run)pos.Parent;

			ts.Select (r.ContentStart, r.ContentEnd);

			Assert.AreEqual (@"&lt;Section xml:space=""preserve"" HasTrailingParagraphBreakOnPaste=""False"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">&lt;Paragraph FontSize=""11"" FontFamily=""Portable User Interface"" Foreground=""#FF000000"" FontWeight=""Normal"" FontStyle=""Normal"" FontStretch=""Normal"" TextAlignment=""Left"">&lt;Run Text=""Moonlight"" />&lt;/Paragraph>&lt;/Section>", rtb.Xaml.Replace ("<", "&lt;"), "#0");

			ts.Text = "";

			Assert.AreEqual (@"", rtb.Xaml.Replace("<","&lt;"), "#1");

			// message on the exception is TextElement_OperationNotSupportedOutsideRTB
			Assert.Throws<NotSupportedException> ( () => pos.CompareTo (r.ContentEnd), "#2" );
		}

		[TestMethod]
		[MoonlightBug]
		public void TextPointersUpdatePosition_clear3 ()
		{
			Paragraph p = new Paragraph ();
			Run r = new Run ();

			r.Text = "Moonlight";

			p.Inlines.Add (r);

			rtb.Blocks.Add (p);

			TextSelection ts = rtb.Selection;

			rtb.SelectAll ();

			TextPointer pos = ts.Start.GetPositionAtOffset (4, LogicalDirection.Backward);

			ts.Select (r.ContentStart, r.ContentEnd);

			Assert.AreEqual (@"&lt;Section xml:space=""preserve"" HasTrailingParagraphBreakOnPaste=""False"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">&lt;Paragraph FontSize=""11"" FontFamily=""Portable User Interface"" Foreground=""#FF000000"" FontWeight=""Normal"" FontStyle=""Normal"" FontStretch=""Normal"" TextAlignment=""Left"">&lt;Run Text=""Moonlight"" />&lt;/Paragraph>&lt;/Section>", rtb.Xaml.Replace ("<", "&lt;"), "#0" );

			ts.Text = "";

			Assert.AreEqual (@"", rtb.Xaml, "#1");

			// message on the exception is TextElement_OperationNotSupportedOutsideRTB
			Assert.Throws<NotSupportedException> ( () => pos.CompareTo (r.ContentEnd), "#2" );
		}

		[TestMethod]
		[MoonlightBug]
		public void Xaml_Set_Null ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.Selection.Xaml = null;
			}, "null");
		}

		[TestMethod]
		[MoonlightBug ("no exception is thrown on invalid xaml")]
		public void Xaml_Set_Invalid ()
		{
			TextSelection ts = rtb.Selection;
			Assert.Throws<ArgumentException> (delegate {
				ts.Xaml = "Moon";
			}, "non-xaml");
		}
	}
}

