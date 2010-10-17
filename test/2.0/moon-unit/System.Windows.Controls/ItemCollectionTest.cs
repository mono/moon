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
using System.Windows.Shapes;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ItemCollectionTest {

		struct S
		{
			public string Name;
			public override bool Equals(object obj)
			{
				return Name.Equals(((S)obj).Name);
			}

			public override int GetHashCode ()
			{
				return base.GetHashCode();
			}
		}

		class T
		{
			public string Name;
			public override bool Equals(object obj)
			{
				return Name.Equals(((T)obj).Name);
			}

			public override int GetHashCode ()
			{
				return base.GetHashCode();
			}
		}

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
		public void AddTwice ()
		{
			ItemCollection c = GetCollection ();
			object o = new object ();
			c.Add (o);
			c.Add (o);
		}

		[TestMethod]
		public void AddTwice2 ()
		{
			// With SL3 we can add UIElements twice to an ItemsCollection
			ItemCollection c = GetCollection ();
			Rectangle o = new Rectangle ();
			c.Add (o);
			c.Add (o);
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
		public void Methods_Int()
		{
			int a = 5;
			int b = 6;
			int c = 7;

			ItemCollection ic = GetCollection();
			ic.Add(a);
			Assert.AreEqual(1, ic.Count, "Count-1");

			ic.Insert(0, ic);
			Assert.AreEqual(2, ic.Count, "Count-2");

			ic.Insert(0, b);
			Assert.AreEqual(3, ic.Count, "Count-3");

			Assert.AreEqual(1, ic.IndexOf(ic), "IndexOf");
			Assert.AreEqual(-1, ic.IndexOf(c), "IndexOf-not in collection");

			Assert.IsTrue(ic.IndexOf(a) >= 0, "IndexOf(object)");
			Assert.IsTrue (ic.Contains (a), "Contains(object)");
			Assert.IsFalse (ic.Contains (c), "Contains(moon)");

			ic.Remove(a);
			Assert.AreEqual(2, ic.Count, "Count-4");

			ic.RemoveAt(0);
			Assert.AreEqual(1, ic.Count, "Count-5");

			ic.Clear();
			Assert.AreEqual(0, ic.Count, "Count-6");
		}

		[TestMethod]
		public void IndexOf_Int ()
		{
			ItemCollection c = GetCollection ();
			c.Add (0);

			Assert.AreEqual (0, c.IndexOf (0), "Contains '0'");
			Assert.AreEqual (0, c.IndexOf (c[0]), "Contains c [0]");
		}

		[TestMethod]
		public void IndexOf_IntConst ()
		{
			ItemCollection ic = GetCollection ();

			ic.Add (5);
			Assert.IsTrue (ic.IndexOf(5) == 0, "IndexOf(int)");
		}

		[TestMethod]
		public void IndexOf_String ()
		{
			// Create two strings which are .Equal but not reference equal
			// Check that we use object equality, not reference equality, when
			// comparing items
			string s1 = new string ('a', 10);
			string s2 = new string ('a', 10);
			Assert.AreNotSame (s1, s2, "not same");
			ItemCollection c = GetCollection ();
			c.Add (s1);
			Assert.AreEqual (0, c.IndexOf (s2), "#1");
		}

		[TestMethod]
		public void Methods_Struct()
		{
			S a = new S { Name = "A" };
			S b = new S { Name = "B" };
			S c = new S { Name = "C" };

			ItemCollection ic = GetCollection();
			ic.Add(a);
			Assert.AreEqual(1, ic.Count, "Count-1");

			ic.Insert(0, ic);
			Assert.AreEqual(2, ic.Count, "Count-2");

			ic.Insert(0, b);
			Assert.AreEqual(3, ic.Count, "Count-3");

			Assert.AreEqual(1, ic.IndexOf(ic), "IndexOf");
			Assert.AreEqual(-1, ic.IndexOf(c), "IndexOf-not in collection");

			Assert.IsTrue(ic.IndexOf(a) >= 0, "IndexOf(object)");
			Assert.IsTrue (ic.Contains (a), "Contains(object)");
			Assert.IsFalse(ic.Contains(c), "Contains(moon)");

			ic.Remove(a);
			Assert.AreEqual(2, ic.Count, "Count-4");

			ic.RemoveAt(0);
			Assert.AreEqual(1, ic.Count, "Count-5");

			ic.Clear();
			Assert.AreEqual(0, ic.Count, "Count-6");
		}

		[TestMethod]
		public void Methods_Point()
		{
			Point a = new Point { X = 5, Y = 5 };
			Point b = new Point { X = 15, Y = 15 };
			Point c = new Point { X = 25, Y = 25 };

			ItemCollection ic = GetCollection();
			ic.Add(a);
			Assert.AreEqual(1, ic.Count, "Count-1");

			ic.Insert(0, ic);
			Assert.AreEqual(2, ic.Count, "Count-2");

			ic.Insert(0, b);
			Assert.AreEqual(3, ic.Count, "Count-3");

			Assert.AreEqual(1, ic.IndexOf(ic), "IndexOf");
			Assert.AreEqual(-1, ic.IndexOf(c), "IndexOf-not in collection");

			Assert.IsTrue (ic.IndexOf(a) >= 0, "IndexOf(point)"); // Fails in Silverlight 3
			Assert.IsTrue (ic.Contains (a), "Contains(point)");
			Assert.IsFalse(ic.Contains(c), "Contains(point)");

			ic.Remove(a);
			Assert.AreEqual(2, ic.Count, "Count-4");

			ic.RemoveAt(0);
			Assert.AreEqual(1, ic.Count, "Count-5");

			ic.Clear();
			Assert.AreEqual(0, ic.Count, "Count-6");
		}

		[TestMethod]
		public void CustomEquals()
		{
			ItemCollection c = GetCollection();
			T t = new T { Name = "A" };
			c.Add(t);
			Assert.IsTrue(c.Contains(t), "#1");
			Assert.IsFalse(c.Contains(new T { Name = "A" }), "#2");
		}

		[TestMethod]
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

			// try removing an element not in the
			// collection (when we know it has a different
			// parent)
			ItemsControl parent2 = new ItemsControl ();
			ItemCollection ic2 = parent2.Items;
			ic2.Add (a);

			ic.Remove (a);
			Assert.AreEqual (parent2, a.Parent, "Parent-different-collection");
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
			// SL3+ doesn't throw if you try to remove 'null'
			ItemCollection ic = GetCollection ();
			ic.Remove (null);
		}

		[TestMethod]
		public void IndexOfNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.AreEqual (-1, ic.IndexOf (null), "indexof/null");
		}

		[TestMethod]
		public void ContainsNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.IsFalse (ic.Contains (null), "contains/null");
		}
		
		[TestMethod]
		public void ParentTest ()
		{
			ItemsControl c = new ItemsControl ();
			FrameworkElement item = new ComboBoxItem ();
			c.Items.Add (item);
			Assert.AreSame (c, item.Parent, "#1");

			item = new Rectangle ();
			c.Items.Add (item);
			Assert.AreSame (c, item.Parent, "#2");

			item = new MoonTest.System.Windows.Controls.ContentControlTest.ConcreteFrameworkElement ();
			c.Items.Add (item);
			Assert.AreSame (c, item.Parent, "#3");
		}
	}
}
