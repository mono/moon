//
// CollectionViewGroup Unit Tests
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
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Specialized;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public partial class CollectionViewGroupTest {

		[TestMethod]
		public void Constructor_Null ()
		{
			var g = new ConcreteCollectionViewGroup (null);
			Assert.IsNull (g.Name, "#1");
		}

		[TestMethod]
		public void Constructor_Object ()
		{
			object o = new object ();
			var g = new ConcreteCollectionViewGroup (o);
			Assert.AreSame (o, g.Name, "#1");
		}

		[TestMethod]
		public void AddItem_DoNotIncrementItemCount ()
		{
			var g = new ConcreteCollectionViewGroup ("");
			g.ProtectedItems.Add (new object ());

			Assert.AreEqual (0, g.OnPropertyChangedCalled.Count, "#1");
			Assert.AreEqual (0, g.ProtectedItemCount, "#2");
			Assert.AreEqual (0, g.ItemCount, "#3");
		}

		[TestMethod]
		public void AddItem_IncrementItemCount ()
		{
			var g = new ConcreteCollectionViewGroup ("");
			g.ProtectedItems.Add (new object ());
			g.ProtectedItemCount++;

			Assert.AreEqual (1, g.OnPropertyChangedCalled.Count, "#1");
			Assert.AreEqual ("ItemCount", g.OnPropertyChangedCalled [0], "#2");
			Assert.AreEqual (1, g.ProtectedItemCount, "#3");
			Assert.AreEqual (1, g.ItemCount, "#4");
		}

		[TestMethod]
		public void GroupsAreRecreated ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, CultureInfo, object> nameCreator = (item, level, culture) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var desc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			source.GroupDescriptions.Add (desc);

			var groups = source.View.Groups;
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];


			using (source.DeferRefresh ())
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Clear ();
				source.GroupDescriptions.Add (desc);
			}

			Assert.AreSame (groups, source.View.Groups, "#1");
			Assert.AreNotSame (lowerGroup, source.View.Groups [0], "#2");
			Assert.AreNotSame (upperGroup, source.View.Groups [1], "#3");
		}

		[TestMethod]
		public void OneGroupDesciption ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, CultureInfo, object> nameCreator = (item, level, culture) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var desc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Add (desc);
			}
			Assert.AreEqual (2, source.View.Groups.Count, "#1");
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];

			Assert.AreEqual ("Lower0", lowerGroup.Name, "#2");
			Assert.AreEqual ("Upper0", upperGroup.Name, "#3");

			Assert.IsTrue (lowerGroup.IsBottomLevel, "#4");
			Assert.IsTrue (upperGroup.IsBottomLevel, "#5");

			Assert.AreEqual (3, lowerGroup.ItemCount, "#6");
			Assert.AreEqual (3, upperGroup.ItemCount, "#7");

			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i, (int) lowerGroup.Items [i], "#8." + i);
			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i + 3, (int) upperGroup.Items [i], "#9." + i);
		}

		[TestMethod]
		public void TwoGroupDesciptions ()
		{
			/* This test generates two subgroups giving this hierarchy:
			 * Root
			 *     Lower0
			 *         Lower1 [Will contain (0, 1, 2)]
			 *         Upper1
			 *     Upper0
			 *        Lower1
			 *        Upper1 [Will contain (3, 4, 5)]
			 */

			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, CultureInfo, object> nameCreator = (item, level, culture) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var level0 = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};
			var level1 = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Add (level0);
				source.GroupDescriptions.Add (level1);
			}
			Assert.AreEqual (2, source.View.Groups.Count, "#1");
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];

			Assert.AreEqual ("Lower0", lowerGroup.Name, "#2");
			Assert.AreEqual ("Upper0", upperGroup.Name, "#3");

			Assert.IsFalse (lowerGroup.IsBottomLevel, "#4");
			Assert.IsFalse (upperGroup.IsBottomLevel, "#5");

			Assert.AreEqual (3, lowerGroup.ItemCount, "#6");
			Assert.AreEqual (3, upperGroup.ItemCount, "#7");

			Assert.AreEqual (1, lowerGroup.Items.Count, "#8");
			Assert.AreEqual (1, upperGroup.Items.Count, "#9");

			// Check the contents of Lower0
			var lower = (CollectionViewGroup) lowerGroup.Items [0];
			Assert.AreEqual ("Lower1", lower.Name, "#10");
			Assert.IsTrue (lower.IsBottomLevel, "#12");
			Assert.AreEqual (3, lower.ItemCount, "#14");
			Assert.AreEqual (3, lower.Items.Count, "#16");

			// Check the contents of Upper0
			var upper = (CollectionViewGroup) upperGroup.Items [0];
			Assert.AreEqual ("Upper1", upper.Name, "#11");
			Assert.IsTrue (upper.IsBottomLevel, "#13");
			Assert.AreEqual (3, upper.ItemCount, "#15");
			Assert.AreEqual (3, upper.Items.Count, "#17");
		}

		[TestMethod]
		public void OneItem_TwoGroups()
		{
			var o = new object();
			var source = new CollectionViewSource { Source = new[] { o } };
			source.GroupDescriptions.Add (new ConcretePropertyGroupDescription() {
				GroupNameFromItemFunc = (item, level, culture) => new [] { "First", "Second" }
			});

			Assert.AreEqual (2, source.View.Groups.Count, "#1");
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			Assert.AreEqual(1, lowerGroup.Items.Count, "#2");
			Assert.AreEqual(o, lowerGroup.Items [0], "#3");

			var upperGroup = (CollectionViewGroup)source.View.Groups[1];
			Assert.AreEqual(1, upperGroup.Items.Count, "#2");
			Assert.AreEqual(o, upperGroup.Items[0], "#3");
		}

		[TestMethod]
		public void OneItem_TwoGroups_Remove()
		{
			var o = new object();
			var source = new CollectionViewSource { Source = new List<object> { o } };
			source.GroupDescriptions.Add (new ConcretePropertyGroupDescription() {
				GroupNameFromItemFunc = (item, level, culture) => new [] { "First", "Second" }
			});

			((IEditableCollectionView)source.View).RemoveAt(0);
			Assert.AreEqual(0, source.View.Cast<object>().Count(), "#1");
		}

		[TestMethod]
		[MoonlightBug ("When we have an item in the groups twice we only emit one Remove event, we need two")]
		public void OneItem_TwoGroups_Remove_Events ()
		{
			var args = new List<NotifyCollectionChangedEventArgs>();
			var source = new CollectionViewSource { Source = new List<object> { new object () } };
			source.GroupDescriptions.Add(new ConcretePropertyGroupDescription() {
				GroupNameFromItemFunc = (item, level, culture) => new[] { "First", "Second" }
			});

			source.View.CollectionChanged += (o, e) => args.Add(e);
			((IEditableCollectionView)source.View).RemoveAt(0);
			Assert.AreEqual(2, args.Count, "#1");

			Assert.AreEqual(NotifyCollectionChangedAction.Remove, args[0].Action, "#2");
			Assert.AreEqual(1, args[0].OldStartingIndex, "#3");

			Assert.AreEqual(NotifyCollectionChangedAction.Remove, args[1].Action, "#4");
			Assert.AreEqual(0, args[1].OldStartingIndex, "#5");
		}
	}

	class ConcreteCollectionViewGroup : CollectionViewGroup {
		public List<string> OnPropertyChangedCalled = new List<string> ();
		public List<string> PropertyChangedRaised = new List<string> ();

		public ConcreteCollectionViewGroup (object name)
			: base (name)
		{
			PropertyChanged += (o, e) => PropertyChangedRaised.Add (e.PropertyName);
		}

		public override bool IsBottomLevel
		{
			get { return true; }
		}
		
		protected override void OnPropertyChanged (global::System.ComponentModel.PropertyChangedEventArgs e)
		{
			OnPropertyChangedCalled.Add (e.PropertyName);
			base.OnPropertyChanged (e);
		}

		public new int ProtectedItemCount {
			get { return base.ProtectedItemCount; }
			set { base.ProtectedItemCount = value; }
		}

		public new ObservableCollection<object> ProtectedItems
		{
			get { return base.ProtectedItems; }
		}
	}
}
