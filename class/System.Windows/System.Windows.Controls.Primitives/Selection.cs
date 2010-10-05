//
// Selection.cs
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
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Windows.Input;

using Mono;

namespace System.Windows.Controls.Primitives
{
	class Selection  {
		static readonly object [] Empty = new object [0];

		Selector Owner {
			get; set;
		}

		public SelectionMode Mode {
			get; set;
		}

		object SelectedItem {
			get; set;
		}

		internal List<object> SelectedItems {
			get; private set;
		}

		public bool Updating {
			get; private set;
		}

		public Selection (Selector owner)
		{
			Owner = owner;
			Owner.SelectedItems.CollectionChanged += HandleOwnerSelectionChanged;
			SelectedItems = new List<object> ();
		}

		void HandleOwnerSelectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			// When 'Updating' is false it means the user has directly modified the collection
			// by calling ListBox.SelectedItems.[Add|Remove]. In this case we need to ensure we
			// don't have a duplicate selection.
			if (!Updating) {
				if (Mode == SelectionMode.Single)
					throw new InvalidOperationException ("SelectedItems cannot be modified directly when in Single select mode");
				try {
					Updating = true;
					switch (e.Action) {
					case NotifyCollectionChangedAction.Add:
						if (!SelectedItems.Contains (e.NewItems [0]))
							AddToSelected (e.NewItems [0]);
						break;
					case NotifyCollectionChangedAction.Remove:
						if (SelectedItems.Contains (e.OldItems [0]))
							RemoveFromSelected (e.OldItems [0]);
						break;
					case NotifyCollectionChangedAction.Replace:
						if (SelectedItems.Contains (e.OldItems [0]))
							RemoveFromSelected (e.OldItems [0]);
						if (!SelectedItems.Contains (e.NewItems [0]))
							AddToSelected (e.NewItems [0]);
						break;
					case NotifyCollectionChangedAction.Reset:
						foreach (var v in SelectedItems.Where (o => !Owner.SelectedItems.Contains (o)).ToArray ())
							if (SelectedItems.Contains (v))
								RemoveFromSelected (v);
						foreach (var v in Owner.SelectedItems.Where (o => !SelectedItems.Contains (o)).ToArray ())
							if (!SelectedItems.Contains (v))
								AddToSelected (v);
						break;
					}

					UpdateOwnerSelectedItems ();
				} finally {
					Updating = false;
				}
			}
		}

		void UpdateOwnerSelectedItems ()
		{
			// Now make sure the Selectors version of 'SelectedItems' matches the actual Selection.
			// This is an incredibly lazy way of doing it and we emit more events than is strictly required.
			// Fix it later if it breaks tests.
			Owner.SelectedItems.Clear ();
			Owner.SelectedItems.AddRange (SelectedItems);
		}

		public void Select (object item)
		{
			Select (item, false);
		}

		public void Select (object item, bool ignoreSelectedValue)
		{
			try {
				Updating = true;

				if (item == null) {
					ClearSelection (ignoreSelectedValue);
					return;
				} else if (!Owner.Items.Contains (item)) {
					if (SelectedItems.Contains (item)) {
						RemoveFromSelected (item);
					}
					return;
				}

				switch (Mode) {
				case SelectionMode.Single:
					if (SelectedItem == item && ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control))
						ClearSelection (ignoreSelectedValue);
					else
						ReplaceSelection (item);
					break;
				case SelectionMode.Multiple:
				case SelectionMode.Extended:
					if (SelectedItems.Contains (item)) {
						RemoveFromSelected (item);
					} else {
						AddToSelected (item);
					}
					break;
				default:
					throw new Exception (string.Format ("SelectionMode.{0} is not supported", Mode));
				}
			} finally {
				Updating = false;
			}
		}

		public void SelectAll (ItemCollection items)
		{

			try {
				Updating = true;

				if (Mode == SelectionMode.Single)
					throw new NotSupportedException ("Cannot call SelectAll when in Single select mode");

				var toSelect = new List<object> ();
				foreach (var v in items)
					if (!SelectedItems.Contains (v))
						toSelect.Add (v);

				if (toSelect.Count == 0)
					return;

				SelectedItems.AddRange (toSelect);
				if (SelectedItem == null) {
					SelectedItem = toSelect [0];
					UpdateOwner (SelectedItem, Owner.Items.IndexOf (SelectedItem), Owner.GetValueFromItem (SelectedItem));
				}

				UpdateOwnerSelectedItems ();
				Owner.RaiseSelectionChanged (Empty, toSelect);
			} finally {
				Updating = false;
			}
		}

		public void SelectOnly (object item)
		{
			if ((SelectedItem == item && SelectedItems.Count == 1)) {
				Console.WriteLine ("Already have: {0} selected", SelectedItem);
				return;
			}
			try {
				Updating = true;
				if (item == null) {
					ClearSelection (false);
					Console.WriteLine ("Cleared. Selected: {0}. Owner.SelectedItem: {1}, Count {2}, Owner.Count {3}", SelectedItem, Owner.SelectedItem, SelectedItems.Count, Owner.SelectedItems.Count);
				}
				else {
					ReplaceSelection (item);
					Console.WriteLine ("Replaced. Selected: {0}. Owner.SelectedItem: {1}, Count {2}, Owner.Count {3}", SelectedItem, Owner.SelectedItem, SelectedItems.Count, Owner.SelectedItems.Count);
				}
			} finally {
				Updating = false;
			}
		}

		void AddToSelected (object item)
		{
			SelectedItems.Add (item);
			if (SelectedItems.Count == 1) {
				SelectedItem = item;
				UpdateOwner (item, Owner.Items.IndexOf (item), Owner.GetValueFromItem (item));
			}

			UpdateOwnerSelectedItems ();
			Owner.RaiseSelectionChanged (Empty, new object [] { item });
		}

		void ClearSelection (bool ignoreSelectedValue)
		{
			var oldSelection = SelectedItems.Cast <object> ().ToArray ();

			SelectedItems.Clear ();
			SelectedItem = null;
			UpdateOwner (null, -1, ignoreSelectedValue ? Owner.SelectedValue : null);

			if (oldSelection.Length > 0) {
				UpdateOwnerSelectedItems ();
				Owner.RaiseSelectionChanged (oldSelection, Empty);
			}
		}

		void RemoveFromSelected (object item)
		{
			SelectedItems.Remove (item);
			if (SelectedItem == item) {
				var newItem = SelectedItems.Count == 0 ? null : SelectedItems [0];
				SelectedItem = newItem;
				UpdateOwner (newItem,newItem == null ? -1 : Owner.Items.IndexOf (newItem), Owner.GetValueFromItem (item));
			}

			UpdateOwnerSelectedItems ();
			Owner.RaiseSelectionChanged (new object [] { item }, Empty);
		}
		
		void ReplaceSelection (object item)
		{
			var addedItems = Empty;
			var oldItems = Empty;
			if (SelectedItem != item || SelectedItems.Count != 1) {
				oldItems = SelectedItems.Cast <object> ().Where (o => o != item).ToArray ();

				// Unselect all the previously selected items
				foreach (var v in oldItems)
					SelectedItems.Remove (v);

				// If we previously had the current item selected, it will be the only one the list now
				// so we only have to add it if the list is empty.
				if (SelectedItems.Count == 0) {
					addedItems = new object [] { item };
					SelectedItems.Add (item);
				}
			}

			// Always update the selection properties to keep everything nicely in sync. These could get out of sync
			// if (for example) the user inserts an item at the start of the ItemsControl.Items collection.
			SelectedItem = item;
			UpdateOwner (item, Owner.Items.IndexOf (item), Owner.GetValueFromItem (item));

			if (addedItems != Empty || oldItems != Empty) {
				// Refresh the Selector.SelectedItems list
				UpdateOwnerSelectedItems ();

				// Raise our SelectionChanged event
				Owner.RaiseSelectionChanged (oldItems, addedItems);
			}
		}

		void UpdateOwner (object item, int index, object value)
		{
			if (Owner.SelectedItem != item)
				Owner.SelectedItem = item;

			if (Owner.SelectedIndex != index)
				Owner.SelectedIndex = index;

			if (Owner.SelectedValue != value)
				Owner.SelectedValue = value;
		}
	}
}

