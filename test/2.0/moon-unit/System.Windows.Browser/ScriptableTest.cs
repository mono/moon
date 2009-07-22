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

		Scriptable scriptable;
		ScriptableType scriptabletype;
		string strplugin;

		public ScriptableTest ()
		{
			plugin = HtmlPage.Plugin;
			window = HtmlPage.Window;
			content = plugin.GetProperty ("Content") as ScriptObject;

			//bool ispopupon = HtmlPage.IsPopupWindowAllowed;
			//HtmlWindow popup = HtmlPage.PopupWindow (new Uri ("about:blank"), "_blank", new HtmlPopupWindowOptions ());
			calc = new Calculator ();
			scriptable = new Scriptable ();
			scriptabletype = new ScriptableType ();

			HtmlPage.RegisterScriptableObject ("calc", calc);
			HtmlPage.RegisterScriptableObject ("scriptable", scriptable);
			HtmlPage.RegisterScriptableObject ("scriptabletype", scriptabletype);
			HtmlPage.RegisterCreateableType ("createable", typeof (CreateableType));

			if (Environment.OSVersion.Platform == PlatformID.Unix)
				strplugin = "document.getElementById('silverlight')";
			else
				strplugin = "document.getElementById('silverlightControlHost').getElementsByTagName('object')[0]";
		}

		[TestMethod]
		public void AA_PropertiesTest () {
			HtmlDocument document = HtmlPage.Document;

			var c = content.GetProperty("calc") as ScriptObject;
			Assert.AreEqual (calc, c.ManagedObject, "ManagedObject");

		}

		[TestMethod]
		public void Casing () {
			var c = content.GetProperty("Calc") as ScriptObject;
			Assert.AreEqual (null, c, "A1");
			c = content.GetProperty("calc") as ScriptObject;
			Assert.AreEqual (calc, c.ManagedObject, "A2");
		}

		[TestMethod]
		public void CreateableTypeTest () {
			window.Eval ("createabletype = " + strplugin + ".content.services.createObject ('createable');");
			window.Eval ("createabletype.MethodAdd ()");
			window.Eval ("c = createabletype.GetCount();");
			window.Eval (strplugin + ".content.scriptable.SetCount(c)");
			Assert.AreEqual (1, scriptable.MethodCount, "D1");
			window.Eval ("createabletype.Property = 'test test'");
			window.Eval (strplugin + ".content.scriptable.Property = createabletype.Property");
			Assert.AreEqual ("test test", scriptable.Property, "D2");
			scriptable.MethodCount = 0;
		}

		[TestMethod]
		public void Invoke () {
			var calc = content.GetProperty ("calc") as ScriptObject;
			var a = calc.Invoke ("Add", 5, 1);
			Assert.AreEqual (6.0, a, "Invoke");
		}

		[TestMethod]
		public void InvokeJS () {
			var o = window.Eval (strplugin + ".content.calc.Add (5, 1);");
			Assert.IsNotNull (o, "InvokeJS 1");
			Assert.AreEqual (6.0, o, "InvokeJS");
		}

		[TestMethod]
		public void InvokeOverload () {
			var calc = content.GetProperty ("calc") as ScriptObject;
			var o = calc.Invoke ("AddOverload", 5);
			Assert.AreEqual (6.0, o, "InvokeOverload 1");
			o = calc.Invoke ("AddOverload", 10);
			Assert.AreEqual (11.0, o, "InvokeOverload 2");
		}

		[TestMethod]
		public void InvokeOverloadJS () {
			var o = window.Eval (strplugin + ".content.calc.AddOverload (5, 1);");
			Assert.AreEqual (6.0, o, "InvokeOverloadJS 1");
			o = window.Eval (strplugin + ".content.calc.AddOverload (10);");
			Assert.AreEqual (11.0, o, "InvokeOverloadJS 1");
		}

		[TestMethod]
		public void Properties () {
			var calc = content.GetProperty ("scriptable") as ScriptObject;
			calc.SetProperty ("Property", "test");

			Assert.AreEqual ("test", scriptable.Property, "Properties 1");

			object o = calc.GetProperty ("Property");
			Assert.AreEqual (scriptable.Property, (string)o, "Properties 2");

			scriptable.Property = String.Empty;
		}

		[TestMethod]
		public void PropertiesJS () {
			window.Eval (strplugin + ".content.scriptable.Property = 'test';");
			Assert.AreEqual ("test", scriptable.Property, "PropertiesJS 1");

			object o = window.Eval ("function f(){return " + strplugin + ".content.scriptable.Property;} f();");
			Assert.AreEqual (scriptable.Property, (string) o, "PropertiesJS 2");
		}

		[TestMethod]
		public void ScriptableMemberTest () {
			window.Eval ("scriptable = " + strplugin + ".content.scriptable;");
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

			window.Eval ("scriptabletype = " + strplugin + ".content.scriptabletype;");
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
		public void EvalTest () {
			object o = window.Eval ("scriptabletype = " + strplugin + ".content.scriptabletype;");
			Assert.IsNotNull (o, "EvalTest 1");
			Assert.IsInstanceOfType (o, typeof (ScriptObject), "EvalTest 2");
			ScriptObject so = (ScriptObject) o;
			Assert.AreSame (so.ManagedObject, scriptabletype, "EvalTest 3");
		}

		[ScriptableType]
		public class TestButton : Button
		{
			public override void OnApplyTemplate ()
			{
				base.OnApplyTemplate ();
				OnApplyTemplateCalled = true;
			}

			public bool OnApplyTemplateCalled;
		}

		[TestMethod]
		public void RegisteringCallsLoaded ()
		{
			TestButton b = new TestButton ();
			bool loaded = false;

			b.Loaded += delegate { loaded = true; };

			HtmlPage.RegisterScriptableObject("testButton", b);

			Assert.IsFalse (loaded, "1");

			b.ApplyTemplate ();

			Assert.IsFalse (b.OnApplyTemplateCalled, "2");
		}

		[TestMethod]
		public void TypeTest ()
		{
			object a;

			a = window.Eval ("a = 1;");
			Assert.IsInstanceOfType (a, typeof (Double), "should be  a Double");
			a = window.Eval ("a = 'hmmm';");
			Assert.IsInstanceOfType (a, typeof (String), "should be  a String");
			a = window.Eval ("a = window;");
			Assert.IsInstanceOfType (a, typeof (HtmlWindow), "should be a HtmlWindow");
			a = window.Eval ("a = window.document;");
			Assert.IsInstanceOfType (a, typeof (HtmlDocument), "should be a HtmlDocument");
			a = window.Eval ("a = window.document.body;");
			Assert.IsInstanceOfType (a, typeof (HtmlElement), "should be a HtmlElement");
		}

		[TestMethod]
		public void AddRemoveHandlerTest ()
		{
			object o, result;

			try {
				HtmlPage.RegisterCreateableType ("eventCalculator", typeof (EventCalculator));

				o = window.Eval ("scriptabletest_addeventhandler_1_result = 0;");
				o = window.Eval ("scriptabletest_addeventhandler_2_result = 0;");
				o = window.Eval (@"
scriptabletest_addeventhandler_1_eventhandler = 
function (sender, args) {
	scriptabletest_addeventhandler_1_event_raised = true;
	scriptabletest_addeventhandler_1_result = args.Result;
}");
				o = window.Eval (@"
scriptabletest_addeventhandler_2_eventhandler = 
function (sender, args) {
	scriptabletest_addeventhandler_2_event_raised = true;
	scriptabletest_addeventhandler_2_result = args.Result;
}");
				o = window.Eval ("scriptable = " + strplugin + ".content.services.createObject ('eventCalculator');");
				o = window.Eval ("scriptable.addEventListener ('DoneEvent', scriptabletest_addeventhandler_1_eventhandler);");
				o = window.Eval ("scriptable.addEventListener ('Done2Event', scriptabletest_addeventhandler_2_eventhandler);");
				o = window.Eval ("scriptable.Add (1, 2);");

				o = window.GetProperty ("scriptabletest_addeventhandler_1_event_raised");
				Assert.IsInstanceOfType (o, typeof (bool), "#1 return type");
				Assert.IsTrue ((bool) o, "#1 event raised");
				result = window.GetProperty ("scriptabletest_addeventhandler_1_result");
				Assert.IsInstanceOfType (result, typeof (double), "#1 result type");
				Assert.AreEqual ((double) result, 3.0, "#1 result");


				o = window.GetProperty ("scriptabletest_addeventhandler_2_event_raised");
				Assert.IsInstanceOfType (o, typeof (bool), "#1.2 return type");
				Assert.IsTrue ((bool) o, "#1.2 event raised");
				result = window.GetProperty ("scriptabletest_addeventhandler_2_result");
				Assert.IsInstanceOfType (result, typeof (double), "#1.2 result type");
				Assert.AreEqual ((double) result, 3.0, "#1.2 result");

				o = window.Eval ("scriptabletest_addeventhandler_1_event_raised = false;");

				o = window.GetProperty ("scriptabletest_addeventhandler_1_event_raised");
				Assert.IsInstanceOfType (o, typeof (bool), "#2 return type");
				Assert.IsFalse ((bool) o, "#2 event raised");

				o = window.Eval ("scriptable.removeEventListener ('DoneEvent', scriptabletest_addeventhandler_1_eventhandler);");
				o = window.Eval ("scriptable.Add (1, 2);");

				o = window.GetProperty ("scriptabletest_addeventhandler_1_event_raised");
				Assert.IsInstanceOfType (o, typeof (bool), "#3 return type");
				Assert.IsFalse ((bool) o, "#3 event raised");

			} catch (Exception ex) {
				throw;
			} finally {
				HtmlPage.UnregisterCreateableType ("eventCalculator");
			}
		}
	}

	[ScriptableType]
	public class CalculationEventArgs : EventArgs
	{
		private int _result;
		public int Result { get { return _result; } }
		public CalculationEventArgs (int result)
		{
			_result = result;
		}
	}

	[ScriptableType]
	public class EventCalculator
	{
		public void Add (int a, int b)
		{
			if (DoneEvent != null)
				DoneEvent (this, new CalculationEventArgs (a + b));
			if (Done2Event != null)
				Done2Event (this, new CalculationEventArgs (a + b));
		}
		public delegate void CalculationDone (object sender, EventArgs ea);
		public delegate void CalculationDone2 (object sender, CalculationEventArgs ea);

		
		public event CalculationDone DoneEvent;
		public event CalculationDone2 Done2Event;
	}

	public class Calculator {

		[ScriptableMember]
		public int Add(int a, int b) {
			return a + b;
		}

		[ScriptableMember]
		public int AddOverload (int a, int b) {
			return a + b;
		}

		[ScriptableMember]
		public int AddOverload (int a) {
			return a + 1;
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
