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

		ItemCollection GetCollection ()
		{
			return new ItemsControl ().Items;
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
		public void Methods ()
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
		[MoonlightBug ("other types inheriting from PresentationFrameworkCollection are throwing ArgumentNullException")]
		public void AddNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Add (null);
			}, "add/null");
		}

		[TestMethod]
		[MoonlightBug ("other types inheriting from PresentationFrameworkCollection are throwing ArgumentNullException")]
		public void InsertNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Insert (0, null);
			}, "insert/null");
		}

		[TestMethod]
		[MoonlightBug ("other types inheriting from PresentationFrameworkCollection do not throw")]
		public void RemoveNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Remove (null);
			}, "remove/null");
		}

		[TestMethod]
		[MoonlightBug ("other types inheriting from PresentationFrameworkCollection do not throw")]
		public void IndexOfNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.IndexOf (null);
			}, "indexof/null");
		}

		[TestMethod]
		[MoonlightBug ("other types inheriting from PresentationFrameworkCollection do not throw")]
		public void ContainsNull ()
		{
			ItemCollection ic = GetCollection ();
			Assert.Throws<ArgumentException> (delegate {
				ic.Contains (null);
			}, "contains/null");
		}
	}
}
