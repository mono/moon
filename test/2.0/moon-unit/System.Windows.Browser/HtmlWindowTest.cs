//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//Copyright (c) 2009 Novell, Inc.
//
//Authors:
//   Moonlight List (moonlight-list@lists.ximian.com)
//

using System;
using System.Windows;
using System.Windows.Browser;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Browser {

	[TestClass]	
	public class HtmlWindowTest {

		[TestMethod]
		[Ignore ("popup UI with no text")]
		public void Alert ()
		{
			HtmlPage.Window.Alert (null);
		}

		[TestMethod]
		[Ignore ("popup UI with no text")]
		public void Confirm ()
		{
			HtmlPage.Window.Confirm (null);
		}

		[TestMethod]
		public void CreateInstance ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.CreateInstance (null, new object [0]);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.CreateInstance (String.Empty, new object [0]);
			}, "Empty");

			ScriptObject xmlhttprequest = HtmlPage.Window.CreateInstance ("XMLHttpRequest");
			Assert.IsNotNull (xmlhttprequest, "XMLHttpRequest");
		}

		[TestMethod]
		public void CurrentBookmark ()
		{
			Assert.AreEqual (String.Empty, HtmlPage.Window.CurrentBookmark, "get");
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.CurrentBookmark = null;
			}, "null");
			HtmlPage.Window.CurrentBookmark = String.Empty;
		}

		[TestMethod]
		public void Eval ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Eval (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.Eval (String.Empty);
			}, "Empty");

			Assert.AreEqual (4.0, (double) HtmlPage.Window.Eval ("2 + 2"), "2+2");

			string s = "new function () {{ this.ci = function () {{ try {{ return new XMLHttpRequest (); }} catch (e) {{ }} }}; }}";
			ScriptObject so = HtmlPage.Window.Eval (s) as ScriptObject;
			ScriptObject xmlhttprequest = so.Invoke ("ci", null) as ScriptObject;
			Assert.IsNotNull (xmlhttprequest, "XMLHttpRequest");
		}

		[TestMethod]
		[MoonlightBug ("actually it's a (documented) FireFox limitation - even on Windows/SL2")]
		public void Eval_Invalid ()
		{
			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.Window.Eval ("moon");
			}, "invalid");
		}

		[TestMethod]
		public void Navigate ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (null);
			}, "null");

			Uri uri = HtmlPage.Document.DocumentUri;

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (null, "target");
			}, "null,string");
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (uri, null);
			}, "uri,null");

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (null, "target", "feature");
			}, "null,string,string");
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (uri, null, "feature");
			}, "uri,null,string");
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.Navigate (uri, "target", null);
			}, "uri,string,null");
		}

		[TestMethod]
		public void NavigateToBookmark ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.NavigateToBookmark (null);
			}, "null");
			HtmlPage.Window.NavigateToBookmark (String.Empty);
		}

		[TestMethod]
		[Ignore ("popup UI with no text")]
		public void Prompt ()
		{
			HtmlPage.Window.Prompt (null);
		}
	}
}

