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
		HtmlWindow window;
		ScriptObject content;
		string root = "var plugin = document.getElementById('silverlight');";

		Scriptable scriptable;
		ScriptableType scriptabletype;

		[TestMethod]
		public void AA_PropertiesTest () {	
			HtmlDocument document = HtmlPage.Document;
			bool isenabled = HtmlPage.IsEnabled;
			plugin = HtmlPage.Plugin;
			window = HtmlPage.Window;
			content = plugin.GetProperty ("Content") as ScriptObject;

			//bool ispopupon = HtmlPage.IsPopupWindowAllowed;
			//HtmlWindow popup = HtmlPage.PopupWindow (new Uri ("about:blank"), "_blank", new HtmlPopupWindowOptions ());
			window.Eval (root);
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

		[TestMethod]
		public void ScriptableMemberTest () {
			scriptable = new Scriptable ();
			HtmlPage.RegisterScriptableObject ("scriptable", scriptable);
			window.Eval ("scriptable = plugin.content.scriptable;");
			window.Eval ("scriptable.MethodAdd();");
			Assert.AreEqual (1, scriptable.MethodCount, "B1");

			window.Eval ("scriptable.SetCount(10);");
			Assert.AreEqual (10, scriptable.MethodCount, "B2");

			window.Eval ("c = scriptable.GetCount();");
			window.Eval ("scriptable.SetCount(--c);");
			Assert.AreEqual (9, scriptable.MethodCount, "B3");
		}

		[TestMethod]
		public void ScriptableTypeTest () {
			scriptabletype = new ScriptableType ();
			HtmlPage.RegisterScriptableObject ("scriptabletype", scriptabletype);
			window.Eval ("scriptabletype = plugin.content.scriptabletype;");
			window.Eval ("scriptabletype.MethodAdd();");
			Assert.AreEqual (1, scriptabletype.MethodCount, "C1");

			window.Eval ("scriptabletype.SetCount(10);");
			Assert.AreEqual (10, scriptabletype.MethodCount, "C2");

			window.Eval ("c = scriptabletype.GetCount();");
			window.Eval ("scriptabletype.SetCount(--c);");
			Assert.AreEqual (9, scriptabletype.MethodCount, "C3");

			window.Eval ("referencedtype = scriptabletype.GetAType();");
			window.Eval ("referencedtype.MethodAdd();");
			Assert.AreEqual (1, scriptabletype.referencedType.MethodCount, "C4");

			window.Eval ("referencedtype.SetCount(10);");
			Assert.AreEqual (10, scriptabletype.referencedType.MethodCount, "C5");

			window.Eval ("referencedtype.SetCount(referencedtype.GetCount()-1);");
			Assert.AreEqual (9, scriptabletype.referencedType.MethodCount, "C6");

			window.Eval ("newreferencedtype = scriptabletype.createManagedObject('ReferencedType');");
			window.Eval ("newreferencedtype.MethodAdd();");
			window.Eval ("scriptabletype.SetCount(newreferencedtype.GetCount());");
			Assert.AreEqual (1, scriptabletype.MethodCount, "C7");

			window.Eval ("newreferencedtype.SetCount(10);");
			window.Eval ("scriptabletype.SetCount(newreferencedtype.GetCount());");
			Assert.AreEqual (10, scriptabletype.MethodCount, "C8");


			window.Eval ("newreferencedtype.SetCount(newreferencedtype.GetCount()-1);");
			window.Eval ("scriptabletype.SetCount(newreferencedtype.GetCount());");
			Assert.AreEqual (9, scriptabletype.MethodCount, "C9");
		}

		[TestMethod]
		public void CreateableTypeTest () {
			scriptable = new Scriptable ();
			HtmlPage.RegisterScriptableObject ("scriptable", scriptable);
			HtmlPage.RegisterCreateableType ("createable", typeof (CreateableType));
			window.Eval ("createabletype = plugin.content.services.createObject ('createable');");
			window.Eval ("createabletype.MethodAdd ()");
			window.Eval ("c = createabletype.GetCount();");
			window.Eval ("plugin.content.scriptable.SetCount(c)");
			Assert.AreEqual (1, scriptable.MethodCount, "D1");
			window.Eval ("createabletype.Property = 'test test'");
			window.Eval ("plugin.content.scriptable.Property = createabletype.Property");
			Assert.AreEqual ("test test", scriptable.Property, "D2");
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

	public class Scriptable {

		int methodCount;
		public int MethodCount {
			get {
				return methodCount;
			}
			set {
				methodCount = value;
			}
		}

		[ScriptableMember]
		public void MethodAdd () {
			++MethodCount;
		}

		[ScriptableMember]
		public int GetCount () {
			return MethodCount;
		}

		[ScriptableMember]
		public void SetCount (int c) {
			MethodCount = c;
		}

		[ScriptableMember]
		public string Property { get; set; }

	}

	[ScriptableType]
	public class ScriptableType : Scriptable {
		public ReferencedType referencedType;
		
		public ReferencedType GetAType ()
		{
			referencedType = new ReferencedType ();
			return referencedType;
		}
	}

	public class CreateableType : Scriptable{
	}

	public class ReferencedType : Scriptable {
	}
}
	  