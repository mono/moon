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

		[TestMethod]
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
#if false
// throw NotImplementedException
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInput_Null ();
			}, "OnTextInput");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInputStart_Null ();
			}, "OnTextInputStart");
			Assert.Throws<ArgumentNullException> (delegate {
				rtb.OnTextInputUpdate_Null ();
			}, "OnTextInputUpdate");
#endif
		}
	}
}

