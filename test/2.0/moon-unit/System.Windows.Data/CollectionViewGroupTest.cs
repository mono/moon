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
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

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
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

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
		[MoonlightBug]
		[Ignore ("Test not completed yet")]
		public void TwoGroupDesciptions ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var lowerDesc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};
			var upperDesc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Add (lowerDesc);
				source.GroupDescriptions.Add (upperDesc);
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

			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i, (int) lowerGroup.Items [i], "#8." + i);
			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i + 3, (int) upperGroup.Items [i], "#9." + i);
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
