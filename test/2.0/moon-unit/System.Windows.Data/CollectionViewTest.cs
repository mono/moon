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
using System.Collections.ObjectModel;
using System.Windows.Controls;
using System.Collections;

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

		ObservableCollection<Rectangle> Rectangles
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

			Rectangles = new ObservableCollection<Rectangle> {
				new Rectangle { Width= 1 },
				new Rectangle { Width= 2 },
				new Rectangle { Width= 3 },
				new Rectangle { Width= 4 },
				new Rectangle { Width= 5 },
			};

			Source = new CollectionViewSource { };
			SetSource(Items);
			ResetCounters();
		}

		void SetSource(IEnumerable items)
		{
			Source.Source = items;
			View = Source.View;

			View.CurrentChanged += (o, e) => {
				CurrentChanged++;
			};
			View.CurrentChanging += (o, e) => {
				CurrentChanging++;
			};
			View.CollectionChanged += (o, e) => CollectionChanged.Add (e);
		}

		void ResetCounters()
		{
			CurrentChanged = 0;
			CurrentChanging = 0;
			CollectionChanged = new List<NotifyCollectionChangedEventArgs>();
		}

		[TestMethod]
		public void BindDirectlyToCVS_ICVProperty ()
		{
			var HostPanel = new StackPanel ();
			var collection = new ObservableCollection<int> () { 1, 2, 5, 0 };
			var cvs = new CollectionViewSource () { Source = collection };
			var binding = new Binding ("IsCurrentAfterLast") {
				Source = cvs,
				BindsDirectlyToSource = true
			};

			HostPanel.SetBinding (Panel.TagProperty, binding);
			Assert.IsNull (HostPanel.Tag, "#1");
		}

		[TestMethod]
		public void BindDirectlyToICV_ICVProperty ()
		{
			var HostPanel = new StackPanel ();
			var collection = new ObservableCollection<int> () { 1, 2, 5, 0 };
			var cvs = new CollectionViewSource () { Source = collection };
			var binding = new Binding ("IsCurrentAfterLast") {
				Source = cvs.View,
				BindsDirectlyToSource = true
			};

			HostPanel.SetBinding (Panel.TagProperty, binding);
			Assert.IsNotNull (HostPanel.Tag, "#2");
		}

		[TestMethod]
		public void BindToCVS_ICVProperty ()
		{
			var HostPanel = new StackPanel ();
			var collection = new ObservableCollection<int> () { 1, 2, 5, 0 };
			var cvs = new CollectionViewSource () { Source = collection };
			var binding = new Binding ("IsCurrentAfterLast") {
				Source = cvs,
				BindsDirectlyToSource = false
			};

			HostPanel.SetBinding (Panel.TagProperty, binding);
			Assert.IsNotNull (HostPanel.Tag, "#3");
		}

		[TestMethod]
		public void BindToICV__ICVProperty ()
		{
			var HostPanel = new StackPanel ();
			var collection = new ObservableCollection<int> () { 1, 2, 5, 0 };
			var cvs = new CollectionViewSource () { Source = collection };
			var binding = new Binding ("IsCurrentAfterLast") {
				Source = cvs.View,
				BindsDirectlyToSource = false
			};

			HostPanel.SetBinding (Panel.TagProperty, binding);
			Assert.IsNotNull (HostPanel.Tag, "#4");
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
		public void ChangeGroupsOnView ()
		{
			View.GroupDescriptions.Add (new ConcretePropertyGroupDescription {
				GroupNameFromItemFunc = (item, depth, culture) => "A",
			});
			Assert.IsNotNull(View.Groups, "#1");
			Assert.IsInstanceOfType<CollectionViewGroup>(View.Groups[0], "#2");
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
		public void Contains()
		{
			Assert.IsTrue(View.Contains(Items[0]), "#1");
		}

		[TestMethod]
		public void CulturePropagatesToDescriptions ()
		{
			CultureInfo culture = null;
			Source.Culture = CultureInfo.InvariantCulture;
			Assert.AreSame (Source.Culture, Source.View.Culture, "#1");

			var group = new ConcretePropertyGroupDescription {
				GroupNameFromItemFunc = (item, level, cult) => {
					culture = cult;
					return "A";
				}
			};

			// Verify that the method was called and we passed in the right cultureinfo
			Source.GroupDescriptions.Add (group);
			Assert.AreSame (Source.Culture, culture, "#2");
		}

		[TestMethod]
		public void DefaultImplIsINPC ()
		{
			Assert.IsTrue (View is INotifyPropertyChanged, "#1");
		}

		[TestMethod]
		public void INPCEvents ()
		{
			INotifyPropertyChanged source = (INotifyPropertyChanged)View;
			VerifyPropertyChanged ("#1", source, () => View.Culture = CultureInfo.InvariantCulture, "Culture");
			VerifyPropertyChanged ("#3", source, () => View.MoveCurrentToNext (), "CurrentPosition", "CurrentItem");

			View.MoveCurrentToFirst ();
			VerifyPropertyChanged ("#4", source, () => View.MoveCurrentToPrevious (), "IsCurrentBeforeFirst", "CurrentPosition", "CurrentItem");
		}

		[TestMethod]
		[MoonlightBug ("wtf?!")]
		public void HiddenINPCEvents ()
		{
			// These INPC events don't have publicly visible properties so I don't know why/how they're being raised.
			INotifyPropertyChanged source = (INotifyPropertyChanged)View;
			VerifyPropertyChanged ("#2", source, () => View.Filter = delegate { return true; }, "Count");
		}

		void VerifyPropertyChanged (string message, INotifyPropertyChanged source, Action action, params string[] expectedProperties)
		{
			List<string> props = new List<string> (expectedProperties);
			PropertyChangedEventHandler h = (o, e) => {
				Assert.AreEqual (props.First (), e.PropertyName, message + ": Incorrect property name received");
				props.RemoveAt (0);
			};

			source.PropertyChanged += h;
			action ();
			source.PropertyChanged -= h;

			Assert.AreEqual (0, props.Count, message + ": All expected PropertyChanged events were not raised for");
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
		public void EventOrdering_SelectNewItem ()
		{
			int count = 0;
			List<object> events = new List<object> ();
			((INotifyPropertyChanged) View).PropertyChanged += (o, e) => {
				Assert.AreSame (Items [1], View.CurrentItem, "#1");
				Assert.AreEqual (1, View.CurrentPosition, "#2");

				if (count == 0)
					Assert.AreEqual ("CurrentPosition", e.PropertyName, "position");
				else if (count == 1)
					Assert.AreEqual ("CurrentItem", e.PropertyName, "#item");
				else
					Assert.Fail ("Too many events");
				count++;
			};

			View.MoveCurrentToNext ();
			Assert.AreEqual (2, count, "#two events should fire");
		}

		[TestMethod]
		public void FilterAndGroup_FilterUpper_GroupBySelf ()
		{
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription (""));
			View.Filter = o => Items.IndexOf (o) < 2;
			Assert.AreEqual (Items.Count - 3, View.Groups.Count, "#1");
		}

		[TestMethod]
		public void FilterAll ()
		{
			Check (Items [0], 0, false, false, "#1");
			View.Filter = o => false;
			Check (null, -1, true, true, "#2");
		}

		[TestMethod]
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
		public void Group_IndexOf()
		{
			List<Rectangle> rects = new List<Rectangle> {
				new Rectangle { Width = 10, Height = 10 },
				new Rectangle { Width = 10, Height = 20 },
				new Rectangle { Width = 20, Height = 10 },
				new Rectangle { Width = 20, Height = 20 }
			};

			SetSource(rects);
			using (Source.DeferRefresh ()) {
				Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription {
					GroupNameFromItemFunc = (item, level, culture) => rects.IndexOf ((Rectangle)item) < 2 ? "A" : "B"
				});

				Source.SortDescriptions.Add (new SortDescription ("Width", ListSortDirection.Descending));
				Source.SortDescriptions.Add (new SortDescription ("Height", ListSortDirection.Ascending));
			}

			Assert.AreSame (rects [2], View.Cast<object>().ElementAt(0),  "#1");
			Assert.AreSame(rects[3], View.Cast<object>().ElementAt(1), "#2");
			Assert.AreSame(rects[0], View.Cast<object>().ElementAt(2), "#3");
			Assert.AreSame(rects[1], View.Cast<object>().ElementAt(3), "#4");
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
		public void InsertItemBeforeSelection ()
		{
			var index = 2;
			var item = Items [index];
			var incc = new ObservableCollection<object>(Items);
			SetSource(incc);

			View.MoveCurrentToPosition(index);
			ResetCounters();
			incc.Insert(0, new object());

			Assert.AreEqual(index + 1, View.CurrentPosition, "#1");
			Assert.AreEqual(item, View.CurrentItem, "#2");
			Assert.AreEqual(0, CurrentChanged, "#3");
			Assert.AreEqual(0, CurrentChanging, "#4");
		}

		[TestMethod]
		public void InsertItem_AfterSelection_SortedSecond ()
		{
			// This should be second in our list no matter where we insert it
			var selection = Rectangles[1];
			var rect = new Rectangle { Width = 1.5 };
			SetSource(Rectangles);

			View.SortDescriptions.Add(new SortDescription("Width", ListSortDirection.Ascending));
			View.MoveCurrentTo(selection);
			ResetCounters();

			Rectangles.Insert(0, rect);
			Assert.AreSame(selection, View.CurrentItem, "#1");
			Assert.AreEqual(2, View.CurrentPosition, "#2");
			Assert.AreEqual(0, CurrentChanged, "#3");
			Assert.AreEqual(0, CurrentChanging, "#4");
		}

		[TestMethod]
		public void InsertItem_BeforeSelection_SortedSecond()
		{
			// This should be second in our list no matter where we insert it
			var selection = Rectangles[1];
			var rect = new Rectangle { Width = 1.5 };
			SetSource(Rectangles);

			View.SortDescriptions.Add(new SortDescription("Width", ListSortDirection.Ascending));
			View.MoveCurrentTo(selection);
			ResetCounters();

			Rectangles.Insert(4, rect);
			Assert.AreSame(selection, View.CurrentItem, "#1");
			Assert.AreEqual(2, View.CurrentPosition, "#2");
			Assert.AreEqual(0, CurrentChanged, "#3");
			Assert.AreEqual(0, CurrentChanging, "#4");
		}

		[TestMethod]
		public void InsertItem_AfterSelection_SortedLast()
		{
			// This should be second in our list no matter where we insert it
			var selection = Rectangles[1];
			var rect = new Rectangle { Width = 6 };
			SetSource(Rectangles);

			View.SortDescriptions.Add(new SortDescription("Width", ListSortDirection.Ascending));
			View.MoveCurrentTo(selection);
			ResetCounters();

			Rectangles.Insert(0, rect);
			Assert.AreSame(selection, View.CurrentItem, "#1");
			Assert.AreEqual(1, View.CurrentPosition, "#2");
			Assert.AreEqual(0, CurrentChanged, "#3");
			Assert.AreEqual(0, CurrentChanging, "#4");
		}

		[TestMethod]
		public void InsertItem_BeforeSelection_SortedLast()
		{
			// This should be second in our list no matter where we insert it
			var selection = Rectangles[1];
			var rect = new Rectangle { Width = 6 };
			SetSource(Rectangles);

			View.SortDescriptions.Add(new SortDescription("Width", ListSortDirection.Ascending));
			View.MoveCurrentTo(selection);
			ResetCounters();

			Rectangles.Insert(4, rect);
			Assert.AreSame(selection, View.CurrentItem, "#1");
			Assert.AreEqual(1, View.CurrentPosition, "#2");
			Assert.AreEqual(0, CurrentChanged, "#3");
			Assert.AreEqual(0, CurrentChanging, "#4");
		}

		[TestMethod]
		public void DifferentGroupDescriptions ()
		{
			Assert.AreNotSame (View.GroupDescriptions, Source.GroupDescriptions, "#1");
		}

		[TestMethod]
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

		[TestMethod]
		public void RefreshEmitsCurrentChangingEvents()
		{
			var events = new List<CurrentChangingEventArgs>();
			var data = new List<object> {
				"First",
				"Second",
				"Third",
				"Fourth",
				"Fifth",
			};

			var view = new CollectionViewSource() { Source = data }.View;
			view.CurrentChanging += (o, e) => events.Add(e);

			view.Refresh();
			Assert.AreEqual(1, events.Count, "#1");

			view.Refresh();
			Assert.AreEqual(2, events.Count, "#2");
		}

		[TestMethod]
		public void Sort_OneDescription ()
		{
			Source.Source = new [] { 1, 2, 3, 4, 5 };
			Source.SortDescriptions.Add (new SortDescription ("", ListSortDirection.Descending));

			Source.View.MoveCurrentToFirst ();
			Assert.AreEqual (5, (int) Source.View.CurrentItem, "#1");

			Source.View.MoveCurrentToLast ();
			Assert.AreEqual (1, (int) Source.View.CurrentItem, "#2");
		}

		[TestMethod]
		public void Sort_TwoDescription ()
		{
			Source.Source = new [] { 1, 2, 3, 4, 5 };
			
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription { GroupNameFromItemFunc = (item, level, culture) => (int) item < 3 ? "A" : "B" });
			Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription { GroupNameFromItemFunc = (item, level, culture) => (int) item < 3 ? "A" : "B" });
			
			Source.SortDescriptions.Add (new SortDescription ("", ListSortDirection.Descending));
			Source.SortDescriptions.Add (new SortDescription ("", ListSortDirection.Ascending));

			Source.View.MoveCurrentToFirst ();
			Assert.AreEqual (5, (int) Source.View.CurrentItem, "#1");

			Source.View.MoveCurrentToLast ();
			Assert.AreEqual (1, (int) Source.View.CurrentItem, "#2");
		}

		[TestMethod]
		public void Sort_TwoDescription_ActualProperties ()
		{
			List<Rectangle> rects = new List<Rectangle> {
				new Rectangle { Width = 10, Height = 10 },
				new Rectangle { Width = 10, Height = 20 },
				new Rectangle { Width = 20, Height = 10 },
				new Rectangle { Width = 20, Height = 20 }
			};

			using (Source.DeferRefresh ()) {
				Source.Source = rects;
				Source.GroupDescriptions.Add (new ConcretePropertyGroupDescription {
					GroupNameFromItemFunc = (item, level, culture) => rects.IndexOf ((Rectangle)item) < 2 ? "A" : "B"
				});

				Source.SortDescriptions.Add (new SortDescription ("Width", ListSortDirection.Descending));
				Source.SortDescriptions.Add (new SortDescription ("Height", ListSortDirection.Ascending));
			}

			// Check the first group
			var group = (CollectionViewGroup) Source.View.Groups [0];
			Assert.AreSame (rects [2], group.Items [0], "#1");
			Assert.AreSame (rects [3], group.Items [1], "#2");
			
			// Check the second group
			group = (CollectionViewGroup) Source.View.Groups [1];
			Assert.AreSame (rects [0], group.Items [0], "#3");
			Assert.AreSame (rects [1], group.Items [1], "#4");
		}

		void Check (object item, int position, bool beforeFirst, bool afterLast, string message)
		{
			Assert.AreEqual (item, View.CurrentItem, message + ".1");
			Assert.AreEqual (position, View.CurrentPosition, message + ".2");
			Assert.AreEqual (beforeFirst, View.IsCurrentBeforeFirst, message + ".3");
			Assert.AreEqual (afterLast, View.IsCurrentAfterLast, message + ".4");
		}
	}
}
