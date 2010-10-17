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
using System.Collections.Generic;

namespace MoonTest.System.Windows.Browser
{
	[TestClass]	
	public class ScriptableArraysTest
	{
	
		[TestMethod]
		public void LengthTest ()
		{
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				return arg.length;
			}}");
			object ret = so.Invoke ("test1", new object[] {new List<string> () {"a", "b"}});
			Assert.AreEqual (2.0, ret, "LengthTest #1");
		}

		[TestMethod]
		public void PushTest ()
		{
			List<string> list = new List<string> () {"a", "b"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, val) {
				arg.push (val);
			}}");
			so.Invoke ("test1", new object[] {list, "c"});
			Assert.AreEqual (3, list.Count, "PushTest #1");
			Assert.AreEqual ("c", list[2], "PushTest #2");
		}

		[TestMethod]
		public void PopTest ()
		{
			List<string> list = new List<string> () {"a", "b"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				return arg.pop ();
			}}");
			object ret = so.Invoke ("test1", new object[] {list});
			Assert.AreEqual (1, list.Count, "PopTest #1");
			Assert.AreEqual ("a", list[0], "PopTest #2");
			Assert.AreEqual ("b", ret, "PopTest #3");
		}

		[TestMethod]
		public void ShiftTest ()
		{
			List<string> list = new List<string> () {"a", "b"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				return arg.shift ();
			}}");
			object ret = so.Invoke ("test1", new object[] {list});
			Assert.AreEqual (1, list.Count, "ShiftTest #1");
			Assert.AreEqual ("b", list[0], "ShiftTest #2");
			Assert.AreEqual ("a", ret, "ShiftTest #3");
		}

		[TestMethod]
		public void UnshiftTest ()
		{
			List<string> list = new List<string> () {"a", "b"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, val) {
				arg.unshift (val);
			}}");
			so.Invoke ("test1", new object[] {list, "c"});
			Assert.AreEqual (3, list.Count, "UnshiftTest #1");
			Assert.AreEqual ("b", list[2], "UnshiftTest #2");
			Assert.AreEqual ("c", list[0], "UnshiftTest #3");
		}

		[TestMethod]
		public void ReverseTest ()
		{
			List<string> list = new List<string> () {"a", "b", "c"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				arg.reverse ();
			}}");
			so.Invoke ("test1", new object[] {list});
			Assert.AreEqual (3, list.Count, "ReverseTest #1");
			Assert.AreEqual ("c", list[0], "Reverse #2");
			Assert.AreEqual ("b", list[1], "Reverse #3");
			Assert.AreEqual ("a", list[2], "Reverse #4");
		}

		[TestMethod]
		public void SpliceTest1 ()
		{
			List<string> list = new List<string> () {"a", "b", "c", "d", "e"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, index) {
				arg.splice (index);
			}}");
			
			so.Invoke ("test1", new object[] {list, 2});
			Assert.AreEqual (2, list.Count, "SpliceTest1 #1");
			Assert.AreEqual ("a", list[0], "SpliceTest1 #2");
			Assert.AreEqual ("b", list[1], "SpliceTest1 #3");
		}

		[TestMethod]
		public void SpliceTest2 ()
		{
			List<string> list = new List<string> () {"a", "b", "c", "d", "e"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, index, count) {
				arg.splice (index, count);
			}}");
			
			so.Invoke ("test1", new object[] {list, 2, 2});
			Assert.AreEqual (3, list.Count, "SpliceTest2 #1");
			Assert.AreEqual ("a", list[0], "SpliceTest2 #2");
			Assert.AreEqual ("b", list[1], "SpliceTest2 #3");
			Assert.AreEqual ("e", list[2], "SpliceTest2 #4");
		}

		[TestMethod]
		public void SpliceTest3 ()
		{
			List<string> list = new List<string> () {"a", "b", "c", "d", "e"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, index, count, val1, val2) {
				arg.splice (index, count, val1, val2);
			}}");
			
			so.Invoke ("test1", new object[] {list, 2, 2, "f", "g"});
			Assert.AreEqual (5, list.Count, "SpliceTest3 #1");
			Assert.AreEqual ("a", list[0], "SpliceTest3 #2");
			Assert.AreEqual ("b", list[1], "SpliceTest3 #3");
			Assert.AreEqual ("f", list[2], "SpliceTest3 #4");
			Assert.AreEqual ("g", list[3], "SpliceTest3 #5");
			Assert.AreEqual ("e", list[4], "SpliceTest3 #6");
		}

		[TestMethod]
		public void IndexTest ()
		{
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg, index) {
				return arg[index];
			}}");
			object ret = so.Invoke ("test1", new object[] {new List<string> () {"a", "b"}, 1});
			Assert.AreEqual ("b", ret, "IndexTest #1");
		}

		class ToStringClass {
			public override string ToString () {
				return "ToString override";
			}
		}

		[TestMethod]
		public void ToStringTest ()
		{
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				return arg.toString();
			}}");
			
			object ret = so.Invoke ("test1", new object[] {new ToStringClass ()});
			Assert.AreEqual (ret, new ToStringClass().ToString(), "ToStringTest #1");
		}

		class ArrayClass {
			[ScriptableMember]
			public string Prop { get; private set; }

			public ArrayClass (string prop) {
				this.Prop = prop;
			}
		}

		[TestMethod]
		public void ArrayTest1 () {
			ArrayClass c = new ArrayClass ("arraytest1");
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				return arg[0].Prop;
			}}");

			object ret = so.Invoke ("test1", new object[] {new List<ArrayClass>(){c} });
			Assert.AreEqual (ret, c.Prop, "ArrayTest1 #1");
		}

		[TestMethod]
		public void ArrayTest2 () {
			List<string> c = new List<string> () {"a", "b"};
			ScriptObject so = (ScriptObject) HtmlPage.Window.Eval (@"new function () { this.test1 = function (arg) {
				arg[1] = 'c';
			}}");

			so.Invoke ("test1", new object[] {c});
			Assert.AreEqual ("c", c[1], "ArrayTest2 #1");
		}
	}
}
