//
// CollectionViewTest.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Windows.Data;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Windows.Shapes;
using System.Globalization;
using System.Collections;
using System.Windows;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public class CollectionViewTest_IEditable {

		public IEditableCollectionView Editable {
			get { return (IEditableCollectionView) View; }
		}

		List<NotifyCollectionChangedEventArgs> CollectionChangedArgs;

		public List<object> Items {
			get; set;
		}

		public CollectionViewSource Source {
			get; set;
		}

		public ICollectionView View {
			get { return Source.View; }
		}

		[TestInitialize]
		public void Setup ()
		{
			Items = new List<object> (new [] {
				new object (),
				new object (),
				new object (),
				new object (),
				new object (),
			});

			CollectionChangedArgs = new List<NotifyCollectionChangedEventArgs> ();
			Source = new CollectionViewSource { Source = Items };
			View.CollectionChanged += (o, e) => CollectionChangedArgs.Add (e);
		}


		[TestMethod]
		public void AddNew_ActuallyAdded ()
		{
			var orig = new List<object> (Items);

			var o = Editable.AddNew ();
			Assert.AreEqual (orig.Count + 1, Items.Count, "#1");
			Assert.IsFalse (orig.Contains (o), "#2");
		}

		[TestMethod]
		public void AddNew_ArrayInt ()
		{
			// You can't add to arrays
			Source.Source = new int [] { 1, 2, 3 };
			Assert.Throws<InvalidOperationException> (() => Editable.AddNew (), "#1");
		}

		[TestMethod]
		public void AddNew_CancelAdd ()
		{
			var o1 = Editable.AddNew ();
			Assert.IsTrue (Editable.IsAddingNew, "#1");
			Assert.IsTrue (Items.Contains (o1), "#2");

			Editable.CancelNew ();
			Assert.IsFalse (Editable.IsAddingNew, "#3");
			Assert.IsFalse (Items.Contains (o1), "#4");
		}

		[TestMethod]
		public void AddNew_CommitFilteredItem()
		{
			// Filter everything then add a new item
			View.Filter = o => false;

			var item = Editable.AddNew();
			Assert.IsTrue(View.Cast<object>().Contains(item), "#1");
			Editable.CommitNew();

			Assert.IsFalse(View.Cast<object>().Contains(item), "#2");
		}

		[TestMethod]
		public void AddNew_DoesNotRegenerateGroups ()
		{
			var groups = View.Groups;

			Editable.AddNew ();
			Assert.AreSame (groups, View.Groups, "#1");

			Editable.CommitNew ();
			Assert.AreSame (groups, View.Groups, "#2");
		}

		[TestMethod]
		public void AddNew_IsAddedToGroups ()
		{
			// If we 'AddNew' the item ends up in a special group it seems
			View.GroupDescriptions.Add (new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = (item, depth, culture) => Items.IndexOf (item) < 3 ? "First" : "Second"
			});
			var groups = View.Groups;

			var added = Editable.AddNew ();
			Assert.AreEqual (3, View.Groups.Count, "#1");
			Assert.IsFalse (((CollectionViewGroup) View.Groups [0]).Items.Contains (added), "#2");
			Assert.IsFalse (((CollectionViewGroup) View.Groups [1]).Items.Contains (added), "#3");
			Assert.AreSame (added, groups [2], "#4");
		}

		[TestMethod]
		public void AddAndCommitNew_IsAddedToGroups ()
		{
			// If we 'AddNew' the item ends up in a special group it seems
			View.GroupDescriptions.Add (new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = (item, depth, culture) => Items.IndexOf (item) < 3 ? "First" : "Second"
			});
			var groups = View.Groups;

			var added = Editable.AddNew ();
			Editable.CommitNew ();
			Assert.AreEqual (2, groups.Count, "#1");

			Assert.IsFalse (((CollectionViewGroup) groups [0]).Items.Contains (added), "#2");
			Assert.IsTrue (((CollectionViewGroup) groups [1]).Items.Contains (added), "#3");
			Assert.AreSame (added, ((CollectionViewGroup) groups [1]).Items.Last (), "#4");
			Assert.AreSame (groups, View.Groups, "#5");
		}

		[TestMethod]
		public void AddNew_MyList_WithInt ()
		{
			Source.Source = new MyList { 1, 2, 3 };
			Assert.Throws<InvalidOperationException> (() => Editable.AddNew (), "#1");
		}

		[TestMethod]
		public void AddNew_OutOfSync ()
		{
			var initialCount = Items.Count;
			Items.Add (new object ());
			Items.Add (new object ());

			var alteredCount = Items.Count;
			Assert.AreEqual (alteredCount, View.Cast<object> ().Count (), "#1");

			Editable.AddNew ();
			Assert.AreEqual (alteredCount + 1, View.Cast<object> ().Count (), "#2");

			Assert.IsTrue (View.MoveCurrentTo (Items [initialCount + 1]), "#3");
			Assert.AreEqual (Items [initialCount + 1], View.CurrentItem, "#4");
		}

		[TestMethod]
		public void AddNew_OutOfSync_Filtered ()
		{
			Source.Filter += (o, e) => e.Accepted = true;
			TestClonesCollection ();
		}

		[TestMethod]
		public void AddNew_OutOfSync_Grouped ()
		{
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = (item, depth, culture) => Items.IndexOf (item) < 3 ? "A" : "B",
			});
			TestClonesCollection ();
		}

		[TestMethod]
		public void AddNew_OutOfSync_Sorted ()
		{
			Source.SortDescriptions.Add (new SortDescription ("blah", ListSortDirection.Ascending));
			TestClonesCollection ();
		}

		void TestClonesCollection ()
		{
			var initialCount = Items.Count;
			Items.Add (new object ());
			Items.Add (new object ());

			Assert.AreEqual (initialCount, View.Cast<object> ().Count (), "#1");

			Editable.AddNew ();
			Assert.AreEqual (initialCount + 1, View.Cast<object> ().Count (), "#2");

			Assert.IsFalse (View.MoveCurrentTo (Items [initialCount + 1]), "#3");
			Assert.IsNull (View.CurrentItem, "#4");
		}

		[TestMethod]
		public void AddNew_ListObject_WithInt ()
		{
			Source.Source = new List<object> { 1, 2, 3 };
			Assert.IsInstanceOfType<object> (Editable.AddNew (), "#1");
		}

		[TestMethod]
		public void AddNew_ListObject_WithObject ()
		{
			Assert.IsInstanceOfType<object> (Editable.AddNew (), "#1");
		}

		[TestMethod]
		public void AddNew_ListObject_WithRectangles ()
		{
			Source.Source = new List<object> { new Rectangle (), new Rectangle () };
			Assert.IsInstanceOfType<object> (Editable.AddNew (), "#1");
		}

		[TestMethod]
		public void EventOrdering_EditableAddItem ()
		{
			var item = Editable.AddNew ();
			Assert.AreEqual (1, CollectionChangedArgs.Count, "#1");

			var args = CollectionChangedArgs [0];
			Assert.AreEqual (NotifyCollectionChangedAction.Add, args.Action, "#3");
			Assert.AreEqual (5, args.NewStartingIndex, "#4");
			Assert.AreEqual (item, args.NewItems [0], "#5");
		}

		[TestMethod]
		public void EventOrdering_CommitFilteredItem ()
		{
			// Filter everything then add a new item
			View.Filter = o => false;

			Editable.AddNew ();
			CollectionChangedArgs.Clear ();
			Editable.CommitNew ();

			Assert.AreEqual (1, CollectionChangedArgs.Count, "#1");
			Assert.AreEqual (NotifyCollectionChangedAction.Remove, CollectionChangedArgs[0].Action, "#2");
		}

		[TestMethod]
		public void EventOrdering_CommitItem_Sorted()
		{
			var items = new List<Rectangle>() {
				new Rectangle { Width = 0 },
				new Rectangle { Width = 1 },
				new Rectangle { Width = 3 },
				new Rectangle { Width = 4 },
			};
			Source.Source = items;
			View.CollectionChanged += (o, e) => CollectionChangedArgs.Add(e);
			View.SortDescriptions.Add(new SortDescription("Width", ListSortDirection.Ascending));

			Editable.AddNew();
			CollectionChangedArgs.Clear();
			Editable.CommitNew();

			Assert.AreEqual(2, CollectionChangedArgs.Count, "#1");

			Assert.AreEqual(NotifyCollectionChangedAction.Remove, CollectionChangedArgs[0].Action, "#2");
			Assert.AreEqual(4, CollectionChangedArgs[0].OldStartingIndex, "#3");

			Assert.AreEqual(NotifyCollectionChangedAction.Add, CollectionChangedArgs[1].Action, "#4");
			Assert.AreEqual(0, CollectionChangedArgs[1].NewStartingIndex, "#5");
		}

		[TestMethod]
		public void AddNew_Sorted ()
		{
			var items = new List<Rectangle> () {
				new Rectangle { Width = 0 },
				new Rectangle { Width = 1 },
				new Rectangle { Width = 3 },
				new Rectangle { Width = 4 },
			};
			Source.Source = items;
			Source.SortDescriptions.Add (new SortDescription ("Width", ListSortDirection.Ascending));

			var rect = Editable.AddNew () as Rectangle;
			Assert.AreSame (rect, View.Cast<object> ().Last (), "#1");
			Assert.AreSame (rect, View.CurrentItem, "#2");
			Assert.AreEqual (4, View.CurrentPosition, "#3");

			rect.Width = 2;
			Assert.AreSame (rect, View.Cast<object> ().Last (), "#4");
			Assert.AreSame (rect, View.CurrentItem, "#5");
			Assert.AreEqual (4, View.CurrentPosition, "#6");

			Editable.CommitNew ();
			Assert.AreSame (rect, View.Cast<object> ().ElementAt (2), "#7");
			Assert.AreSame (rect, View.CurrentItem, "#8");
			Assert.AreEqual (2, View.CurrentPosition, "#9");
		}

		[TestMethod]
		public void AddNew_Sorted_SameKeys ()
		{
			var items = new List<Rectangle> () {
				new Rectangle { Width = 0 },
				new Rectangle { Width = 2, Height = 1 },
				new Rectangle { Width = 1 },
				new Rectangle { Width = 2, Height = 2 },
				new Rectangle { Width = 3 },
				new Rectangle { Width = 2, Height = 3 },
				new Rectangle { Width = 4 },
				new Rectangle { Width = 2, Height = 4 },
			};

			Source.Source = items;
			Source.SortDescriptions.Add (new SortDescription ("Width", ListSortDirection.Ascending));

			var rect = Editable.AddNew () as Rectangle;
			Assert.AreSame (rect, View.Cast<object> ().Last (), "#1");
			Assert.AreSame (rect, View.CurrentItem, "#2");
			Assert.AreEqual (8, View.CurrentPosition, "#3");

			rect.Width = 2;
			Assert.AreSame (rect, View.Cast<object> ().Last (), "#4");
			Assert.AreSame (rect, View.CurrentItem, "#5");
			Assert.AreEqual (8, View.CurrentPosition, "#6");

			Editable.CommitNew ();
			Assert.AreSame (rect, View.CurrentItem, "#7");
			Assert.AreEqual (3, View.CurrentPosition, "#8");
			Assert.AreSame (rect, View.Cast<object> ().ElementAt (3), "#9");
		}

		[TestMethod]
		public void AddNew_Twice ()
		{
			var o1 = Editable.AddNew ();
			var o2 = Editable.AddNew ();

			Assert.IsTrue (Items.Contains (o1), "#1");
			Assert.IsTrue (Items.Contains (o2), "#2");
		}

		[TestMethod]
		public void AddNew_Twice_CancelAdd ()
		{
			var o1 = Editable.AddNew ();
			var o2 = Editable.AddNew ();

			Editable.CancelNew ();
			Assert.IsTrue (Items.Contains (o1), "#1");
			Assert.IsFalse (Items.Contains (o2), "#2");
		}

		[TestMethod]
		public void AddNew_UpdatesViewImmediately ()
		{
			var item = Editable.AddNew ();
			Assert.AreEqual (item, View.Cast<object> ().Last (), "#1");
		}

		[TestMethod]
		public void AddNew_WhileUpdating ()
		{
			using (View.DeferRefresh ())
				Assert.Throws<InvalidOperationException> (() => Editable.AddNew ());
		}

		[TestMethod]
		public void CommitNew_NoAdd ()
		{
			// This has no effect if there's no add new
			Editable.CommitNew ();
		}

		[TestMethod]
		public void CancelNew_NoAdd ()
		{
			// This has no effect if there's no add new
			Editable.CancelNew ();
		}

		[TestMethod]
		public void SourceIsEnumerable ()
		{
			// Raw enumerables are not editable and have their own
			// ICollectionView implementation
			Source.Source = Items.Select (d => true);
			Assert.IsNotInstanceOfType<IEditableCollectionView> (View);
		}

		[TestMethod]
		public void SourceIsArray ()
		{
			Source.Source = Items.ToArray ();
			Assert.IsFalse (Editable.CanAddNew, "#1");
			Assert.IsFalse (Editable.CanCancelEdit, "#2");
			Assert.IsFalse (Editable.CanRemove, "#3");
		}

		[TestMethod]
		public void SourceIsList ()
		{
			Assert.IsTrue (Editable.CanAddNew, "#1");
			Assert.IsFalse (Editable.CanCancelEdit, "#2");
			Assert.IsTrue (Editable.CanRemove, "#3");
		}
	}

	public class MyList : IList {
		List<object> list = new List<object> ();

		public int Add (object value)
		{
			list.Add (value);
			return 1;
		}

		public void Clear ()
		{
			list.Clear ();
		}

		public bool Contains (object value)
		{
			return list.Contains (value);
		}

		public int IndexOf (object value)
		{
			return list.IndexOf (value);
		}

		public void Insert (int index, object value)
		{
			list.Insert (index, value);
		}

		public bool IsFixedSize
		{
			get { return ((IList) list).IsFixedSize; }
		}

		public bool IsReadOnly
		{
			get { return ((IList) list).IsReadOnly; }
		}

		public void Remove (object value)
		{
			list.Remove (value);
		}

		public void RemoveAt (int index)
		{
			list.RemoveAt (index);
		}

		public object this [int index]
		{
			get
			{
				return list [index];
			}
			set
			{
				list [index] = value;
			}
		}

		public void CopyTo (Array array, int index)
		{
			((IList)list).CopyTo (array, index);
		}

		public int Count
		{
			get { return list.Count; }
		}

		public bool IsSynchronized
		{
			get { return ((IList) list).IsSynchronized; }
		}

		public object SyncRoot
		{
			get { return ((IList) list).SyncRoot; }
		}

		public IEnumerator GetEnumerator ()
		{
			return list.GetEnumerator ();
		}
	}
}
