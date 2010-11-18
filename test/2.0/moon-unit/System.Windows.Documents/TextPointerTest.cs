//
// Unit tests for TextPointer
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
	public partial class TextPointerTest : SilverlightTest {

		RichTextBox rtb;

		[TestInitialize]
		public void Setup ()
		{
			rtb = new RichTextBox ();
		}

		[TestMethod]
		public void CompareTo_defaultRTBSettings ()
		{
			// for some reason RTB's initial values for ContentStart/ContentEnd are equal?
			Assert.AreEqual (0, rtb.ContentStart.CompareTo (rtb.ContentStart), "#0");
			Assert.AreEqual (0, rtb.ContentStart.CompareTo (rtb.ContentEnd), "#1");
			Assert.AreEqual (0, rtb.ContentEnd.CompareTo (rtb.ContentStart), "#2");
		}

		[TestMethod]
		[MoonlightBug ("#7 is failing")]
		public void CompareTo ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.ContentStart.CompareTo (null);
			}, "null");

			rtb.SelectAll ();
			rtb.Selection.Text = "hi";

			Assert.AreEqual (0, rtb.ContentStart.CompareTo (rtb.ContentStart), "#3");
			Assert.AreEqual (-1, rtb.ContentStart.CompareTo (rtb.ContentEnd), "#4");
			Assert.AreEqual (1, rtb.ContentEnd.CompareTo (rtb.ContentStart), "#5");

			rtb.SelectAll ();
			rtb.Selection.Text = "";

			Assert.AreEqual (0, rtb.ContentStart.CompareTo (rtb.ContentStart), "#6");
			Assert.AreEqual (-1, rtb.ContentStart.CompareTo (rtb.ContentEnd), "#7");
			Assert.AreEqual (1, rtb.ContentEnd.CompareTo (rtb.ContentStart), "#8");
		}

		[TestMethod]
		public void CompareTo_LogicalDirection ()
		{
			rtb.SelectAll ();
			rtb.Selection.Text = "";

			Paragraph p = new Paragraph ();

			Run r1 = new Run { Text = "Bold text" };

			p.Inlines.Add (r1);

			rtb.Selection.Insert (p);

			TextPointer tpb = r1.ContentStart.GetPositionAtOffset (1, LogicalDirection.Backward);
			TextPointer tpf = r1.ContentStart.GetPositionAtOffset (1, LogicalDirection.Forward);

			Assert.AreEqual (0, tpb.CompareTo(tpf), "1");

			
		}

		[TestMethod]
		[Asynchronous]
		public void GetCharacterRect ()
		{
			// all these cases return empty, presumably due to there being no RichTextBoxView

			Rect r;
			
			Enqueue (() => {
					r = rtb.ContentStart.GetCharacterRect ((LogicalDirection) Int32.MinValue);
					Assert.IsTrue (r.IsEmpty, "#0");

					r = rtb.ContentStart.GetCharacterRect (LogicalDirection.Forward);
					Assert.IsTrue (r.IsEmpty, "#1");
				});

			Enqueue (() => rtb.Selection.Text = "hi" );

			Enqueue (() => {
					r = rtb.ContentStart.GetCharacterRect (LogicalDirection.Forward);
					Assert.IsTrue (r.IsEmpty, "#2");
				});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("#0 is failing")]
		public void GetCharacterRect_inTree ()
		{
			CreateAsyncTest (rtb,
					 () => {
						 Rect r = rtb.ContentStart.GetCharacterRect ((LogicalDirection) Int32.MinValue);
						 Assert.AreEqual (new Rect(4,4,0,16), r, "#0");

						 r = rtb.ContentStart.GetCharacterRect (LogicalDirection.Forward);
						 Assert.AreEqual (new Rect(4,4,0,16), r, "#1");
					 },
					 () => {
						 rtb.Selection.Text = "hi";
						 Rect r = rtb.ContentStart.GetCharacterRect (LogicalDirection.Forward);
						 Assert.IsTrue (r.IsEmpty, "#2");
					 },
					 () => {
						 Rect r = rtb.ContentStart.GetCharacterRect (LogicalDirection.Forward);
						 Assert.AreEqual (new Rect(4,4,0,16), r, "#3");

						 r = rtb.ContentStart.GetPositionAtOffset (1, LogicalDirection.Forward).GetCharacterRect (LogicalDirection.Forward);
						 Assert.AreEqual (new Rect(4,4,0,16), r, "#4");

						 r = rtb.ContentStart.GetPositionAtOffset (2, LogicalDirection.Forward).GetCharacterRect (LogicalDirection.Forward);
						 Assert.AreEqual (new Rect(4,4,0,16), r, "#5");

						 r = rtb.ContentStart.GetPositionAtOffset (3, LogicalDirection.Forward).GetCharacterRect (LogicalDirection.Forward);
						 Assert.AreEqualWithDelta (10.960000038147, r.X, 0.001, "#6.x");
						 Assert.AreEqualWithDelta (4, r.Y, 0.001, "#6.y");
						 Assert.AreEqualWithDelta (0, r.Width, 0.001, "#6.width");
						 Assert.AreEqualWithDelta (16, r.Height, 0.001, "#6.height");
					 });
		}

		[TestMethod]
		[MoonlightBug]
		public void GetPositionAtOffset ()
		{
			Assert.IsNull (rtb.ContentStart.GetPositionAtOffset (Int32.MinValue, LogicalDirection.Forward), "invalid offset");

			// invalid enum
			TextPointer p = rtb.ContentStart.GetPositionAtOffset (0, (LogicalDirection) Int32.MinValue); 
			Assert.IsNotNull (p, "invalid enum");
			Assert.IsTrue (p.IsAtInsertionPosition, "IsAtInsertionPosition");
			Assert.AreEqual (LogicalDirection.Forward, p.LogicalDirection, "LogicalDirection");
			Assert.AreEqual (rtb, p.Parent, "Parent");
		}

		public void FlowDirectionInsert_ (FlowDirection direction, LogicalDirection ldir, string resultingtext)
		{
			rtb.SelectAll ();
			rtb.Selection.Text = "";
			rtb.FlowDirection = direction;

			Run r = new Run { Text = "Text goes here" };

			rtb.Selection.Insert (r);

			rtb.UpdateLayout ();

			TextPointer tp = r.ContentStart.GetPositionAtOffset (3, ldir);

			rtb.Selection.Select (tp, tp);
			Console.WriteLine (1);
			rtb.Selection.Text = "hi";
			Console.WriteLine (2);

			Assert.AreEqual (resultingtext, r.Text);
		}

		[TestMethod]
		public void FlowDirectionInsert ()
		{
			FlowDirectionInsert_ (FlowDirection.LeftToRight, LogicalDirection.Forward, "Texhit goes here");
			FlowDirectionInsert_ (FlowDirection.RightToLeft, LogicalDirection.Forward, "Texhit goes here");
		}

		public void TreeWalks_GetPositionAtOffset (FlowDirection fd)
		{
			rtb.SelectAll ();
			rtb.Selection.Text = "";

			// we create the following tree:

			//   paragraph
			//  /    \     
			// bold  italic
			//  |     /   \
			// run   run  run
			
			// Section sec = new Section ();
			Paragraph p = new Paragraph ();
			Bold b = new Bold ();
			Italic i = new Italic ();

			Run r1 = new Run { Text = "Bold text" };
			Run r2 = new Run { Text = "Italic text1" };
			Run r3 = new Run { Text = "Italic text2" };

			p.Inlines.Add (b);
			p.Inlines.Add (i);

			b.Inlines.Add (r1);

			i.Inlines.Add (r2);
			i.Inlines.Add (r3);

			rtb.Selection.Insert (p);

			TextPointer tp;

			int offset = 0;
			// forward tests from p.ContentStart
			////////////////////////////////////

			// forward 0 offset keeps us at p.ContentStart
			Console.WriteLine ("1");
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (p, tp.Parent, "#1-0");
			Assert.AreEqual (0, tp.CompareTo (p.ContentStart), "#1-1");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#1-2");

			// forward 1 offset moves us to b.ContentStart
			Console.WriteLine ("2");
			offset++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (b, tp.Parent, "#2-0");
			Assert.AreEqual (0, tp.CompareTo (b.ContentStart), "#2-1");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#2-2");

			// forward 2 offset moves us to r1.ContentStart
			Console.WriteLine ("3");
			offset++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (r1, tp.Parent, "#3-0");
			Assert.AreEqual (0, tp.CompareTo (r1.ContentStart), "#3-1");
			Assert.IsTrue (tp.IsAtInsertionPosition, "#3-2");

			// forward 3 offset moves us 1 character into the interior of r1
			Console.WriteLine ("4");
			offset++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (r1, tp.Parent, "#4-0");
			Assert.AreEqual (1, tp.CompareTo (r1.ContentStart), "#4-1");
			Assert.IsTrue (tp.IsAtInsertionPosition, "#4-2");

			// forward 2 + r1.Text.Length moves us to r1.ContentEnd
			Console.WriteLine ("5");
			offset += r1.Text.Length - 1;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (r1, tp.Parent, "#5-0");
			Assert.AreEqual (0, tp.CompareTo (r1.ContentEnd), "#5-1");
			Assert.IsTrue (tp.IsAtInsertionPosition, "#5-2");

			// forward 1 more and we'll be at b.ContentEnd
			Console.WriteLine ("6");
			offset ++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (b, tp.Parent, "#6-0");
			Assert.AreEqual (0, tp.CompareTo (b.ContentEnd), "#6-1");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#6-2");

			// forward 1 more and we're back in the paragraph, between the bold and italic elements
			Console.WriteLine ("7");
			offset++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (p, tp.Parent, "#7-0");
			Assert.AreEqual (1, tp.CompareTo (b.ContentEnd), "#7-1");
			Assert.AreEqual (-1, tp.CompareTo (i.ContentStart), "#7-2");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#7-3");

			// forward 1 more and we'll be at i.ContentStart
			Console.WriteLine ("8");
			offset++;
			tp = p.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Forward);
			Assert.AreEqual (i, tp.Parent, "#8-0");
			Assert.AreEqual (0, tp.CompareTo (i.ContentStart), "#8-1");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#8-2");


			// backward tests from r3.ContentStart
			//////////////////////////////////////

			// backward 0 keeps us at r3.ContentStart
			Console.WriteLine ("9");
			offset = 0;
			tp = r3.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Backward);
			Assert.AreEqual (r3, tp.Parent, "#9-0");
			Assert.AreEqual (0, tp.CompareTo (r3.ContentStart), "#9-1");
			Assert.IsTrue (tp.IsAtInsertionPosition, "#9-2");

			// backward 1 more and we're in the italic element, between r2 and r3
			Console.WriteLine ("10");
			offset--;
			tp = r3.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Backward);
			Assert.AreEqual (i, tp.Parent, "#10-0");
			Assert.AreEqual (-1, tp.CompareTo (r3.ContentStart), "#10-1");
			Assert.AreEqual (1, tp.CompareTo (r2.ContentEnd), "#10-2");
			Assert.IsFalse (tp.IsAtInsertionPosition, "#10-2");

			// backward 1 more and we're at r2.ContentEnd
			offset--;
			tp = r3.ContentStart.GetPositionAtOffset (offset, LogicalDirection.Backward);
			Assert.AreEqual (r2, tp.Parent, "#11-0");
			Assert.AreEqual (0, tp.CompareTo (r2.ContentEnd), "#11-1");
			Assert.IsTrue (tp.IsAtInsertionPosition, "#11-2");
		}

		[TestMethod]
		public void TreeWalks_GetPositionAtOffset_ltr ()
		{
			TreeWalks_GetPositionAtOffset (FlowDirection.LeftToRight);
		}

		[TestMethod]
		public void TreeWalks_GetPositionAtOffset_rtl ()
		{
			TreeWalks_GetPositionAtOffset (FlowDirection.RightToLeft);
		}

		[TestMethod]
		[Ignore ("GetNextInsertionPosition seems to actually insert new elements?  bizarre")]
		public void TreeWalks_GetNextInsertionPosition ()
		{
			rtb.SelectAll ();
			rtb.Selection.Text = "";

			// we create the following tree:

			//   paragraph
			//  /    \     
			// bold  italic
			//  |
			// run

			// Section sec = new Section ();
			Paragraph p = new Paragraph ();
			Bold b = new Bold ();
			Italic i = new Italic ();

			Run r1 = new Run { Text = "Bold text" };

			p.Inlines.Add (b);
			p.Inlines.Add (i);

			b.Inlines.Add (r1);

			rtb.Selection.Insert (p);

			// now let's do a lot of checks for moving
			// textpointers around.
			TextPointer tp;

			// move forward from paragraph.ContentStart
			tp = p.ContentStart;
			tp = tp.GetNextInsertionPosition (LogicalDirection.Forward);
			Assert.AreEqual (b, tp.Parent, "#7");

			// backing up from element.ContentStart gets
			// us... someplace.  and TextPointer.Parent is
			// bizarre.
			tp = r1.ContentStart;
			Assert.AreSame (r1, tp.Parent, "#0");
			tp = tp.GetNextInsertionPosition (LogicalDirection.Backward);
			Assert.AreEqual (typeof (Run), tp.Parent.GetType(), "#1");
			Assert.AreNotEqual (r1, tp.Parent, "#2");
			Assert.AreEqual (-1, tp.CompareTo (r1.ElementStart), "#3");

			// backing up from element.ElementStart gets
			// us... someplace else.  and
			// TextPointer.Parent is still bizarre.
			tp = r1.ElementStart.GetNextInsertionPosition (LogicalDirection.Backward);
			Assert.AreEqual (typeof (Run), tp.Parent.GetType(), "#5");
			Assert.AreNotEqual (r1, tp.Parent, "#5");
			Assert.AreEqual (-1, tp.CompareTo (r1.ContentStart), "#6");
		}
	}
}

