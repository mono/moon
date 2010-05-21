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
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Windows.Input;

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

		internal IList SelectedItems {
			get; private set;
		}

		public bool Updating {
			get; private set;
		}

		public Selection (Selector owner)
		{
			Owner = owner;

			var items = new ObservableCollection <object> ();
			items.CollectionChanged += HandleItemsCollectionChanged;
			SelectedItems = items;
		}

		void HandleItemsCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			// When 'Updating' is false it means the user has directly modified the collection
			// by calling ListBox.SelectedItems.<Action>. In this case we need to emit the event
			if (!Updating) {
				if (Mode == SelectionMode.Single)
					throw new InvalidOperationException ("SelectedItems cannot be modified directly when in Single select mode");

				Owner.RaiseSelectionChanged (e.OldItems ?? Empty, e.NewItems ?? Empty);
			}
		}

		public void Select (object item)
		{
			try {
				Updating = true;

				if (item == null) {
					ClearSelection ();
					return;
				} else if (!Owner.Items.Contains (item)) {
					if (SelectedItems.Contains (item))
						RemoveFromSelected (item);
					return;
				}

				switch (Mode) {
				case SelectionMode.Single:
					if (SelectedItem == item && ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control))
						ClearSelection ();
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

		void AddToSelected (object item)
		{
			SelectedItems.Add (item);
			if (SelectedItems.Count == 1) {
				Owner.SelectedItem = item;
				Owner.SelectedIndex  = Owner.Items.IndexOf (item);
				Owner.SelectedValue = Owner.SelectedValueWalker.GetValue (item);
				SelectedItem = item;
			}

			Owner.RaiseSelectionChanged (Empty, new object [] { item });
		}

		void ClearSelection ()
		{
			bool hasSelection = SelectedItem != null;
				var oldSelection = SelectedItems.Cast <object> ().ToArray ();
				SelectedItems.Clear ();
				Owner.SelectedItem = null;
				Owner.SelectedIndex = -1;
				Owner.SelectedValue = null;
				SelectedItem = null;
				if (hasSelection)
					Owner.RaiseSelectionChanged (oldSelection, Empty);
		}

		void RemoveFromSelected (object item)
		{
			SelectedItems.Remove (item);
			if (SelectedItem == item) {
				var newItem = SelectedItems.Count == 0 ? null : SelectedItems [0];
				Owner.SelectedItem = newItem;
				Owner.SelectedIndex = newItem == null ? -1 : Owner.Items.IndexOf (newItem);
				Owner.SelectedValue = newItem == null ? null : Owner.SelectedValueWalker.GetValue (Owner.SelectedItem);
				SelectedItem = newItem;
			}

			Owner.RaiseSelectionChanged (new object [] { item }, Empty);
		}
		
		void ReplaceSelection (object item)
		{
			if (SelectedItem == null) {
				AddToSelected (item);
			} else {
				var olditem = SelectedItem;
				SelectedItems.Remove (olditem);
				SelectedItems.Add (item);
				SelectedItem = item;
				Owner.SelectedItem  = item;
				Owner.SelectedIndex = Owner.Items.IndexOf (item);
				Owner.SelectedValue = Owner.SelectedValueWalker.GetValue (item);

				if (olditem != item)
					Owner.RaiseSelectionChanged (new object [] { olditem }, new object []  { item }); 
			}
		}
	}
}

