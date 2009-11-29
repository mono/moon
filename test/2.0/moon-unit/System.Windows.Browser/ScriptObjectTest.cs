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
//Copyright (c) 2008-2009 Novell, Inc.
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
	public class ScriptObjectTest {

		[TestMethod]
		[MoonlightBug]
		public void SetInnerHTML ()
		{
			var element = HtmlPage.Document.CreateElement ("div");

			element.SetProperty ("innerHTML", typeof (object));

			var result = element.GetProperty ("innerHTML");
			Assert.IsNotNull (result);
			Assert.IsInstanceOfType (result, typeof (string));
			Assert.AreEqual ("System.Object", result);
		}

		[TestMethod]
		public void GetProperty ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.GetProperty (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.GetProperty (String.Empty);
			}, "empty");

			ScriptObject so = HtmlPage.Window.GetProperty ("ShutdownHarness") as ScriptObject;
			Assert.IsNotNull (so, "string");

			Assert.IsNull (HtmlPage.Window.GetProperty ("DoesNotExists"), "DoesNotExists");

			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.GetProperty ("ShutdownHarness\0bad");
			}, "embedded null");
		}

		[TestMethod]
		public void SetProperty ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.Window.SetProperty (null, new object ());
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.SetProperty (String.Empty, new object ());
			}, "empty");

			Assert.IsNull (HtmlPage.Window.GetProperty ("DoesNotExists"), "DoesNotExists");
			HtmlPage.Window.SetProperty ("DoesNotExists", 12);
			// works for primitive types but not with managed types (see SetInnerHTML)
			Assert.IsNotNull (HtmlPage.Window.GetProperty ("DoesNotExists"), "DoesNotExists-2");

			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.Window.SetProperty ("ShutdownHarness\0bad", null);
			}, "embedded null");
		}

		[TestMethod]
		public void Invoke ()
		{
			string s = "new function () {{ this.ci = function () {{ try {{ return new XMLHttpRequest (); }} catch (e) {{ }} }}; }}";
			ScriptObject so = HtmlPage.Window.Eval (s) as ScriptObject;
			Assert.Throws<ArgumentNullException> (delegate {
				so.Invoke (null, new object [0]);
			}, "null,object[]");
			Assert.Throws<ArgumentException> (delegate {
				so.Invoke (String.Empty, new object [0]);
			}, "empty,object[]");

			Assert.IsNotNull (so.Invoke ("ci") as ScriptObject, "Invoke(string)");
			Assert.IsNotNull (so.Invoke ("ci", null) as ScriptObject, "Invoke(string,null)");
			Assert.IsNotNull (so.Invoke ("ci", new object [0]) as ScriptObject, "Invoke(string,[0])");
			Assert.IsNotNull (so.Invoke ("ci", new object [1]) as ScriptObject, "Invoke(string,[1])");

			Assert.Throws<ArgumentException> (delegate {
				so.Invoke ("ci\0bad", new object [0]);
			}, "embbeded-null-char,object[]");
		}

		[TestMethod]
		public void Invoke_ExceptionHandling ()
		{
			string s = "new function () {{ this.ci = function () {{ try {{ return new XMLHttpRequest (); }} catch (e) {{ }} }}; }}";
			ScriptObject so = HtmlPage.Window.Eval (s) as ScriptObject;
			Assert.IsNotNull (so, "XMLHttpRequest");
			Assert.Throws<InvalidOperationException> (delegate {
				so.Invoke ("di");
			}, "XMLHttpRequest.Invoke(di)");
		}

		[TestMethod]
		public void InvokeSelf ()
		{
			ScriptObject so = HtmlPage.Window.GetProperty ("ScriptObjectInvokeSelfTest") as ScriptObject;
			Assert.IsTrue ((bool) so.InvokeSelf (), "InvokeSelf()");
			Assert.IsTrue ((bool) so.InvokeSelf (null), "InvokeSelf(null)");
			Assert.IsTrue ((bool) so.InvokeSelf (new object [0]), "InvokeSelf([0])");
			Assert.IsTrue ((bool) so.InvokeSelf (new object [1]), "InvokeSelf([1])");
		}

		[TestMethod]
		public void InvokeSelf_ExceptionHandling ()
		{
			string s = "new function () {{ this.ci = function () {{ try {{ return new XMLHttpRequest (); }} catch (e) {{ }} }}; }}";
			ScriptObject so = HtmlPage.Window.Eval (s) as ScriptObject;
			Assert.IsNotNull (so, "XMLHttpRequest");
			Assert.Throws<InvalidOperationException> (delegate {
				so.InvokeSelf ();
			}, "XMLHttpRequest.InvokeSelf");
		}
	}
}

