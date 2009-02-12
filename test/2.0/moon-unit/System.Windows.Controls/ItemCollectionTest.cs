//
// Unit tests for ItemCollection
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ItemCollectionTest {

		private ItemsControl parent;

		public DependencyObject Parent {
			get { return parent; }
		}

		ItemCollection GetCollection ()
		{
			parent = new ItemsControl ();
			return parent.Items;
		}

		[TestMethod]
		public void DefaultValues ()
		{
			ItemCollection ic = GetCollection ();
			Assert.AreEqual (0, ic.Count, "Count");
			Assert.IsFalse (ic.IsFixedSize, "IsFixedSize");
			Assert.IsFalse (ic.IsReadOnly, "IsReadOnly");
			Assert.IsFalse (ic.IsSynchronized, "IsSynchronized");
			Assert.AreSame (ic, ic.SyncRoot, "SyncRoot");
		}

		[TestMethod]
		public void Methods_String ()
		{
			ItemCollection ic = GetCollection ();
			ic.Add ("string");
			Assert.AreEqual (1, ic.Count, "Count-1");

			ic.Insert (0, ic);
			Assert.AreEqual (2, ic.Count, "Count-2");

			ic.Insert (0, "another");
			Assert.AreEqual (3, ic.Count, "Count-3");

			Assert.AreEqual (1, ic.IndexOf (ic), "IndexOf");
			Assert.AreEqual (-1, ic.IndexOf ("mono"), "IndexOf-not in collection");

			Assert.IsTrue (ic.IndexOf ("string") >= 0, "IndexOf(string)");
			Assert.IsTrue (ic.Contains ("string"), "Contains(string)");
			Assert.IsFalse (ic.Contains ("moon"), "Contains(moon)");

			ic.Remove ("string");
			Assert.AreEqual (2, ic.Count, "Count-4");

			ic.RemoveAt (0);
			Assert.AreEqual (1, ic.Count, "Count-5");

			ic.Clear ();
			Assert.AreEqual (0, ic.Count, "Count-6");
		}

		[TestMethod]
		public void Methods_Object ()
		{
			object a = new object ();
			object b = new object ();
			object c = new object ();

			ItemCollection ic = GetCollection ();
			ic.Add (a);
			Assert.AreEqual (1, ic.Count, "Count-1");

			ic.Insert (0, ic);
			Assert.AreEqual (2, ic.Count, "Count-2");

			ic.Insert (0, b);
			Assert.AreEqual (3, ic.Count, "Count-3");

			Assert.AreEqual (1, ic.IndexOf (ic), "IndexOf");
			Assert.AreEqual (-1, ic.IndexOf (c), "IndexOf-not in collection");

			Assert.IsTrue (ic.IndexOf (a) >= 0, "IndexOf(object)");
			Assert.IsTrue (ic.Contains (a), "Contains(object)");
			Assert.IsFalse (ic.Contains (c), "Contains(moon)");

			ic.Remove (a);
			Assert.AreEqual (2, ic.Count, "Count-4");

			ic.RemoveAt (0);
			Assert.AreEqual (1, ic.Count, "Count-5");

			ic.Clear ();
			Assert.AreEqual (0, ic.Count, "Count-6");
		}

		[TestMethod]
		[MoonlightBug ("DO needs special support for setting parents")]
		public void Methods_Control ()
		{
			Slider a = new Slider ();
			Button b = new Button ();
			TextBlock c = new TextBlock ();

			ItemCollection ic = GetCollection ();
			Assert.IsNull (a.Parent, "Slider-Parent-0");

			ic.Add (a);
			Assert.IsNotNull (a.Parent, "Slider-Parent-1");
			Assert.AreSame (Parent, a.Parent, "Slider-Parent-1-Same");
			Assert.AreEqual (1, ic.Count, "Count-1");

			ic.Insert (0, ic);
			Assert.AreEqual (2, ic.Count, "Count-2");

			Assert.IsNull (b.Parent, "Button-Parent-2");
			ic.Insert (0, b);
			Assert.AreSame (Parent, b.Parent, "Button-Parent-3");
			Assert.AreEqual (3, ic.Count, "Count-3");

			Assert.AreEqual (1, ic.IndexOf (ic), "IndexOf");
			Assert.AreEqual (-1, ic.IndexOf (c), "IndexOf-not in collection");

			Assert.IsTrue (ic.IndexOf (a) >= 0, "IndexOf(object)");
			Assert.IsTrue (ic.Contains (a), "Contains(object)");
			Assert.IsFalse (ic.Contains (c), "Contains(moon)");

			ic.Remove (a);
			Assert.IsNull (a.Parent, "Slider-Parent-4");
			Assert.AreEqual (2, ic.Count, "Count-4");

			ic.RemoveAt (0);
			Assert.AreEqual (1, ic.Count, "Count-5");

			ic.Clear ();
			Assert.AreEqual (0, ic.Count, "Count-6");
		}

		[TestMethod]
		public void AddNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Add (null);
			}, "add/null");
		}

		[TestMethod]
		public void InsertNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Insert (0, null);
			}, "insert/null");
		}

		[TestMethod]
		public void RemoveNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Remove (null);
			}, "remove/null");
		}

		[TestMethod]
		public void IndexOfNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.IndexOf (null);
			}, "indexof/null");
		}

		[TestMethod]
		public void ContainsNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Contains (null);
			}, "contains/null");
		}
	}
}
