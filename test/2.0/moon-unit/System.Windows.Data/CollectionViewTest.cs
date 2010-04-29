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

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public class CollectionViewTest {

		public int CurrentChanged { get; set; }
		public int CurrentChanging { get; set; }
		public List<NotifyCollectionChangedEventArgs> CollectionChanged;

		public List<object> Items
		{
			get;
			set;
		}

		public CollectionViewSource Source
		{
			get;
			set;
		}

		public ICollectionView View
		{
			get;
			set;
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

			CurrentChanged = 0;
			CurrentChanging = 0;
			CollectionChanged = new List<NotifyCollectionChangedEventArgs> ();

			Source = new CollectionViewSource { Source = Items };
			View = Source.View;
			View.CurrentChanged += (o, e) => {
				CurrentChanged++;
			};
			View.CurrentChanging += (o, e) => {
				CurrentChanging++;
			};
			View.CollectionChanged += (o, e) => CollectionChanged.Add (e);
		}

		[TestMethod]
		public void ChangingGroupsClearsFilter ()
		{
			View.Filter = o => true;
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			Assert.IsNull (View.Filter, "#1");

			View.Filter = o => true;
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription ("Test"));
			Assert.IsNull (View.Filter, "#2");
		}

		[TestMethod]
		public void ChangeGroupsOnSource_EventsOnView ()
		{
			var list = new List<NotifyCollectionChangedEventArgs> ();
			View.GroupDescriptions.CollectionChanged += (o, e) => list.Add (e);

			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
			Assert.AreEqual (2, list.Count, "#1");
			Assert.AreEqual (NotifyCollectionChangedAction.Reset, list [0].Action, "#2");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, list [1].Action, "#3");

			list.Clear ();
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
			Assert.AreEqual (3, list.Count, "#4");
			Assert.AreEqual (NotifyCollectionChangedAction.Reset, list [0].Action, "#5");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, list [1].Action, "#6");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, list [2].Action, "#7");
		}

		[TestMethod]
		public void DeferAndAddGroup ()
		{
			using (View.DeferRefresh ()) {
				View.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
				Assert.IsNull (View.Groups, "#1");
			}
			Assert.IsNotNull (View.Groups, "#2");
		}

		[TestMethod]
		public void DeferTwiceAndAddGroup ()
		{
			using (View.DeferRefresh ()) {
				using (View.DeferRefresh ()) {
					View.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
					Assert.IsNull (View.Groups, "#1");
				}
				Assert.IsNull (View.Groups, "#2");
			}
			Assert.IsNotNull (View.Groups, "#3");
		}

		[TestMethod]
		public void DeferSameOneTwiceAndAddGroup ()
		{
			using (var deferrer = View.DeferRefresh ()) {
				using (View.DeferRefresh ()) {
					deferrer.Dispose ();
					deferrer.Dispose ();
					deferrer.Dispose ();
					deferrer.Dispose ();
					View.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
					Assert.IsNull (View.Groups, "#1");
				}
				Assert.IsNotNull (View.Groups, "#3");
			}
		}

		[TestMethod]
		public void EmptyList ()
		{
			View = new CollectionViewSource { Source = new object [0] }.View;
			Assert.IsTrue (View.IsCurrentBeforeFirst, "#1");
			Assert.IsTrue (View.IsCurrentAfterLast, "#2");
		}

		[TestMethod]
		[MoonlightBug]
		public void FilterAndGroup_FilterUpper_GroupBySelf ()
		{
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			View.Filter = o => Items.IndexOf (o) < 2;
			Assert.AreEqual (Items.Count - 3, View.Groups.Count, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void FilterAll ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => false;
			Check (null, -1, true, true, "#2");
		}

		[TestMethod]
		[MoonlightBug]
		public void FilterSome_LowerHalf ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => Items.IndexOf (o) >= 2;
			Check (Items [2], 0, false, false, "#2");

		}

		[TestMethod]
		public void FilterSome_UpperHalf ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => Items.IndexOf (o) < 2;
			Check (Items [0], 0, false, false, "#2");
		}

		[TestMethod]
		public void Group_TwoDescriptions_CanGroup ()
		{
			Assert.IsTrue (View.CanGroup, "#1");
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			Assert.IsTrue (View.CanGroup, "#2");
		}

		[TestMethod]
		[MoonlightBug]
		public void Group_GroupBySelf ()
		{
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			Assert.AreEqual (Items.Count, View.Groups.Count, "#1");
			for (int i = 0; i < View.Groups.Count; i++) {
				var g = (CollectionViewGroup) View.Groups [i];
				Assert.AreEqual (1, g.ItemCount, "#2." + i);
				Assert.IsTrue (g.IsBottomLevel, "#3." + i);
				Assert.AreSame (Items [i], g.Name, "#5." + i);
			}
		}

		[TestMethod]
		public void GroupDescriptions_SourceForwardsToView ()
		{
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			Assert.AreEqual (1, View.GroupDescriptions.Count, "#1");
			Assert.AreSame (Source.GroupDescriptions [0], View.GroupDescriptions [0], "#2");

			View.GroupDescriptions.Add (new ConcretePropertyGroupDescription ("Test"));
			Assert.AreEqual (1, Source.GroupDescriptions.Count, "#3");

			View.GroupDescriptions.Clear ();
			Assert.AreEqual (1, Source.GroupDescriptions.Count, "#4");

			View.GroupDescriptions.Add (new ConcretePropertyGroupDescription ("aaa"));
			Source.GroupDescriptions.Clear ();
			Assert.AreEqual (0, View.GroupDescriptions.Count, "#5");
		}

		[TestMethod]
		public void GroupsIsSameCollection ()
		{
			Assert.IsNull (View.Groups, "#1");

			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			Assert.IsNotNull (View.Groups, "#2");

			var g = View.Groups;
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription ("Tester"));
			Assert.AreSame (g, View.Groups, "#3");

			Source.GroupDescriptions.Clear ();
			Assert.IsNull (View.Groups, "#4");

			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription ());
			Assert.AreSame (g, View.Groups, "#5");
		}

		[TestMethod]
		public void ImplicitGroupDoesNotExist ()
		{
			Assert.IsNull (View.Groups, "#1");
		}

		[TestMethod]
		public void DifferentGroupDescriptions ()
		{
			Assert.AreNotSame (View.GroupDescriptions, Source.GroupDescriptions, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void MoveTo_LowerFiltered ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => Items.IndexOf (o) >= 2;
			View.MoveCurrentToPosition (1);
			Check (Items [3], 1, false, false, "#2");

		}

		[TestMethod]
		public void MoveTo_UpperFiltered ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => Items.IndexOf (o) < 2;
			View.MoveCurrentToPosition (1);
			Check (Items [1], 1, false, false, "#2");
		}

		[TestMethod]
		public void MoveEventsRaised ()
		{
			View.MoveCurrentToNext ();
			Check (Items [1], 1, false, false, "#2");
			Assert.AreEqual (1, CurrentChanging, "#3");
			Assert.AreEqual (1, CurrentChanged, "#4");
		}

		[TestMethod]
		public void MoveEventsRaised_Cancelled ()
		{
			View.CurrentChanging += (o, e) => {
				Assert.IsTrue (e.IsCancelable, "#1");
				e.Cancel = true;
			};
			// It will not move
			Assert.IsTrue (View.MoveCurrentToNext (), "#2");
			Check (Items [0], 0, false, false, "#3");
			Assert.AreEqual (1, CurrentChanging, "#4");
			Assert.AreEqual (0, CurrentChanged, "#5");
		}

		[TestMethod]
		public void MoveToPrev ()
		{
			Assert.IsTrue (View.MoveCurrentToPosition (2), "#a");
			Assert.IsTrue (View.MoveCurrentToPrevious (), "#b");
			Check (Items [1], 1, false, false, "#1");
		}

		[TestMethod]
		public void MoveToPrev_AtStart ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentToFirst (), "#a");
			Assert.IsFalse (View.MoveCurrentToPrevious (), "#b");
			Check (null, -1, true, false, "#2");
		}

		[TestMethod]
		public void MoveToPrev_BeforeFirst ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentToFirst (), "#a");
			Assert.IsFalse (View.MoveCurrentToPrevious (), "#b");
			Assert.IsFalse (View.MoveCurrentToPrevious (), "#c");
			Check (null, -1, true, false, "#2");
		}

		[TestMethod]
		public void MoveToFirst ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentToFirst (), "#a");
			Check (Items [0], 0, false, false, "#2");
		}

		[TestMethod]
		public void MoveToNext ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentToNext (), "#a");
			Check (Items [1], 1, false, false, "#2");
		}

		[TestMethod]
		public void MoveToNext_AtEnd ()
		{
			Assert.IsTrue (View.MoveCurrentToLast (), "#1");
			Assert.IsFalse (View.MoveCurrentToNext (), "#2");
			Check (null, Items.Count, false, true, "#3");
		}

		[TestMethod]
		public void MoveToNext_AfterLast ()
		{
			Assert.IsTrue (View.MoveCurrentToLast (), "#1");
			Check (Items.Last (), Items.Count - 1, false, false, "#2");

			Assert.IsFalse (View.MoveCurrentToNext (), "#3");
			Check (null, Items.Count, false, true, "#4");

			Assert.IsFalse (View.MoveCurrentToNext (), "#5");
			Check (null, Items.Count, false, true, "#6");
		}

		[TestMethod]
		public void MoveToLast ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentToLast (), "#a");
			Check (Items.Last (), Items.Count - 1, false, false, "#2");
		}

		[TestMethod]
		public void MoveToItem ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentTo (Items [2]), "#a");
			Check (Items [2], 2, false, false, "#2");
		}

		[TestMethod]
		public void MoveToItem_NotThere ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsFalse (View.MoveCurrentTo (new object ()), "#a");
			Check (null, -1, true, false, "#2");
		}

		void Check (object item, int position, bool beforeFirst, bool afterLast, string message)
		{
			Assert.AreSame (item, View.CurrentItem, message + ".1");
			Assert.AreEqual (position, View.CurrentPosition, message + ".2");
			Assert.AreEqual (beforeFirst, View.IsCurrentBeforeFirst, message + ".3");
			Assert.AreEqual (afterLast, View.IsCurrentAfterLast, message + ".4");
		}
	}
}
