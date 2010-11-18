//
// Unit tests for RichTextBox
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

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class RichTextBoxTest {

		class MyRichTextBox : RichTextBox {

			public void OnGotFocus_Null ()
			{
				base.OnGotFocus (null);
			}

			public void OnKeyDown_Null ()
			{
				base.OnKeyDown (null);
			}

			public void OnKeyUp_Null ()
			{
				base.OnKeyUp (null);
			}

			public void OnLostFocus_Null ()
			{
				base.OnLostFocus (null);
			}

			public void OnMouseEnter_Null ()
			{
				base.OnMouseEnter (null);
			}

			public void OnMouseLeave_Null ()
			{
				base.OnMouseLeave (null);
			}

			public void OnMouseLeftButtonDown_Null ()
			{
				base.OnMouseLeftButtonDown (null);
			}

			public void OnMouseLeftButtonUp_Null ()
			{
				base.OnMouseLeftButtonUp (null);
			}

			public void OnMouseMove_Null ()
			{
				base.OnMouseMove (null);
			}

			public void OnTextInput_Null ()
			{
				base.OnTextInput (null);
			}

			public void OnTextInputStart_Null ()
			{
				base.OnTextInputStart (null);
			}

			public void OnTextInputUpdate_Null ()
			{
				base.OnTextInputUpdate (null);
			}
		}

		RichTextBox rtb;

		[TestInitialize]
		public void Setup ()
		{
			rtb = new RichTextBox ();
		}

		[TestMethod]
		[MoonlightBug ("The OnTextInput* methods emit NIE")]
		public void OnNullEventArgs ()
		{
			MyRichTextBox rtb = new MyRichTextBox ();
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnGotFocus_Null ();
			}, "OnGotFocus");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnKeyDown_Null ();
			}, "OnKeyDown");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnKeyUp_Null ();
			}, "OnKeyUp");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnLostFocus_Null ();
			}, "OnLostFocus");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnMouseEnter_Null ();
			}, "OnMouseEnter");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnMouseLeave_Null ();
			}, "OnMouseLeave");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnMouseLeftButtonDown_Null ();
			}, "OnMouseLeftButtonDown");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnMouseLeftButtonUp_Null ();
			}, "OnMouseLeftButtonUp");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnMouseMove_Null ();
			}, "OnMouseMove");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInput_Null ();
			}, "OnTextInput");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInputStart_Null ();
			}, "OnTextInputStart");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInputUpdate_Null ();
			}, "OnTextInputUpdate");
		}

		[TestMethod]
		public void SelectAll_startEndInRun_afterSetText ()
		{
			TextSelection ts = rtb.Selection;

			ts.Text = "Moonlight";

			rtb.SelectAll ();

			Assert.AreEqual (typeof (Run), ts.Start.Parent.GetType(), "#0");
			Assert.AreEqual (typeof (Run), ts.End.Parent.GetType(), "#1");
			Assert.AreEqual (ts.Start.Parent, ts.End.Parent, "#2");
		}

		[TestMethod]
		public void SelectAll_startEndInRun_afterDocumentCreation ()
		{
			TextSelection ts = rtb.Selection;

			Paragraph p = new Paragraph ();
			Run r = new Run ();

			r.Text = "Moonlight";

			p.Inlines.Add (r);

			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (r, ts.Start.Parent, "#0");
			Assert.AreEqual (r, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_startEndInRun_afterDocumentCreation_multiParagraphs ()
		{
			TextSelection ts = rtb.Selection;

			Paragraph p1 = new Paragraph ();
			Paragraph p2 = new Paragraph ();
			Run r = new Run ();

			r.Text = "Moonlight";

			p2.Inlines.Add (r);

			rtb.Blocks.Add (p1);
			rtb.Blocks.Add (p2);

			rtb.SelectAll ();

			Assert.AreEqual (p1, ts.Start.Parent, "#0");
			Assert.AreEqual (r, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_startEndInRun_afterDocumentCreation_multiParagraphs2 ()
		{
			TextSelection ts = rtb.Selection;

			Paragraph p1 = new Paragraph ();
			Paragraph p2 = new Paragraph ();
			Paragraph p3 = new Paragraph ();
			Run r = new Run ();

			r.Text = "Moonlight";

			p2.Inlines.Add (r);

			rtb.Blocks.Add (p1);
			rtb.Blocks.Add (p2);
			rtb.Blocks.Add (p3);

			rtb.SelectAll ();

			Assert.AreEqual (p1, ts.Start.Parent, "#0");
			Assert.AreEqual (p3, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_afterDocumentCreation_withoutRun ()
		{
			TextSelection ts = rtb.Selection;

			Paragraph p = new Paragraph ();

			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (p, ts.Start.Parent, "#0");
			Assert.AreEqual (p, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_afterDocumentCreation_withSpan ()
		{
			TextSelection ts = rtb.Selection;
			Span s = new Span ();
			Paragraph p = new Paragraph ();

			p.Inlines.Add (s);
			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (s, ts.Start.Parent, "#0");
			Assert.AreEqual (s, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_afterDocumentCreation_withSpansAndRun ()
		{
			TextSelection ts = rtb.Selection;
			Run r = new Run ();
			Paragraph p = new Paragraph ();

			r.Text = "Moonlight";

			p.Inlines.Add (new Span());
			p.Inlines.Add (new Span());
			p.Inlines.Add (new Span());
			p.Inlines.Add (r);
			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (r, ts.Start.Parent, "#0");
			Assert.AreEqual (r, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_afterDocumentCreation_withSpansAndRuns ()
		{
			TextSelection ts = rtb.Selection;
			Run r1 = new Run ();
			Run r2 = new Run ();
			Paragraph p = new Paragraph ();

			r1.Text = "Moonlight";
			r2.Text = "Moonlight";

			Span s = new Span ();

			s.Inlines.Add (r1);
			p.Inlines.Add (new Span());
			p.Inlines.Add (new Span());
			p.Inlines.Add (s);
			p.Inlines.Add (r2);
			p.Inlines.Add (new Span());
			p.Inlines.Add (new Span());
			p.Inlines.Add (new Span());

			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (r1, ts.Start.Parent, "#0");
			Assert.AreEqual (r2, ts.End.Parent, "#1");
		}

		[TestMethod]
		public void SelectAll_afterDocumentCreation_withSpanAndRun ()
		{
			TextSelection ts = rtb.Selection;
			Span s = new Span ();
			Run r1 = new Run ();
			Run r2 = new Run ();
			Paragraph p = new Paragraph ();

			s.Inlines.Add (r1);
			p.Inlines.Add (s);
			p.Inlines.Add (r2);
			rtb.Blocks.Add (p);

			rtb.SelectAll ();

			Assert.AreEqual (r1, ts.Start.Parent, "#0");
			Assert.AreEqual (r2, ts.End.Parent, "#1");
		}
	}
}

