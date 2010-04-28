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

using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Windows.Data;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public class CollectionViewTest {

		public int CurrentChanged { get; set; }
		public int CurrentChanging { get; set; }
		public List<NotifyCollectionChangedEventArgs> CollectionChanged;

		public List<object> Items {
			get; set;
		}

		public ICollectionView View {
			get; set;
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

			View = new CollectionViewSource { Source = Items }.View;
			View.CurrentChanged += (o, e) => {
				CurrentChanged++;
			};
			View.CurrentChanging += (o, e) => {
				CurrentChanging++;
			};
			View.CollectionChanged += (o, e) => CollectionChanged.Add (e);
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
			Check (Items.Last (), Items.Count - 1,false, false, "#2");
		}

		[TestMethod]
		public void MoveToItem ()
		{
			Check (Items [0], 0, false, false, "#1");
			Assert.IsTrue (View.MoveCurrentTo (Items [2]), "#a");
			Check (Items[2], 2, false, false,"#2");
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
