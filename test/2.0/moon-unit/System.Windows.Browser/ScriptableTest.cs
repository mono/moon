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
//Copyright (c) 2008 Novell, Inc.
//
//Authors:
//   Moonlight List (moonlight-list@lists.ximian.com)
//

using System;
using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Browser;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Browser
{
	[TestClass]	
	public class ScriptableTest
	{
		Calculator calc;
		HtmlElement plugin;
		ScriptObject content;
		
		[TestMethod]
		public void AA_PropertiesTest () {	
			HtmlDocument document = HtmlPage.Document;
			bool isenabled = HtmlPage.IsEnabled;
			plugin = HtmlPage.Plugin;
			HtmlWindow window = HtmlPage.Window;
			content = plugin.GetProperty ("Content") as ScriptObject;

			//bool ispopupon = HtmlPage.IsPopupWindowAllowed;
			//HtmlWindow popup = HtmlPage.PopupWindow (new Uri ("about:blank"), "_blank", new HtmlPopupWindowOptions ());
		}

		[TestMethod]
		public void AB_Register () {
					
			calc = new Calculator ();
			HtmlPage.RegisterScriptableObject ("calc", calc);
			var c = content.GetProperty("calc") as ScriptObject;
			Assert.AreEqual (calc, c.ManagedObject, "ManagedObject");
		}
		
		[TestMethod]
		public void Casing () {
			var c = content.GetProperty("Calc") as ScriptObject;
			Assert.AreEqual (calc, c.ManagedObject, "ManagedObject");
		}

		[TestMethod]
		public void Invoke () {
			var calc = content.GetProperty ("calc") as ScriptObject;
			var a = calc.Invoke ("Add", 5, 1);
			Assert.AreEqual (a, 6, "Add");
		}
	}

	public class Calculator {

		[ScriptableMember]
		public int Add(int a, int b) {
			return a + b;
		}
		// Not callable from script.
		public int Subtract(int a, int b) {
			return a - b;
		}
	}
}
