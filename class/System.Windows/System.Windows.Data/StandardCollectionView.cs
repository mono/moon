using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

using Mono;

namespace System.Windows.Data {

	sealed class ListCollectionView : EditableCollectionView, IDeferRefresh, IComparer<object> {

		public IList ActiveList {
			get {
				if (Filter == null && GroupDescriptions.Count == 0 && SortDescriptions.Count == 0)
					return SourceCollection;
				return filteredList;
			}
		}


		PropertyComparer Comparer {
			get { return new PropertyComparer (SortDescriptions); }
		}

		bool DeferCurrentChanged {
			get; set;
		}

		int DeferLevel {
			get; set;
		}

		int IDeferRefresh.DeferLevel {
			get { return DeferLevel; }
			set { DeferLevel = value; }
		}

		ObservableList<object> filteredList {
			get; set;
		}

		bool Grouping {
			get { return Groups != null; }
		}

		bool IsValidSelection {
			get { return CurrentPosition >= 0 && CurrentPosition < ActiveList.Count; }
		}

		ConstructorInfo ItemConstructor {
			get; set;
		}

		RootCollectionViewGroup RootGroup {
			get; set;
		}

		public new IList SourceCollection {
			get { return (IList) base.SourceCollection; }
		}

		public ListCollectionView (IList collection)
			: base (collection)
		{
			var interfaces = SourceCollection.GetType ().GetInterfaces ();
			foreach (var t in interfaces) {
				if (t.IsGenericType && t.GetGenericTypeDefinition () == typeof (IList<>)) {
					Type type = t.GetGenericArguments () [0];
					ItemConstructor = type.GetConstructor (Type.EmptyTypes);
				}
			}

			UpdateCanAddNewAndRemove ();
			filteredList = new ObservableList<object> ();
			RootGroup = new RootCollectionViewGroup (null, null, 0, false, SortDescriptions);
			CurrentPosition = -1;
			IsEmpty = ActiveList.Count == 0;
			MoveCurrentToPosition (0);

			if (SourceCollection is INotifyCollectionChanged)
				((INotifyCollectionChanged) SourceCollection).CollectionChanged += HandleSourceCollectionChanged;

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged) SortDescriptions).CollectionChanged += (o, e) => {
				if (IsAddingNew || IsEditingItem)
					throw new InvalidOperationException ("Cannot modify SortDescriptions while adding or editing an item");
				Refresh ();
			};

			filteredList.CollectionChanged += HandleFilteredListCollectionChanged;
			RootGroup.CollectionChanged += HandleRootGroupCollectionChanged;
		}

		void HandleRootGroupCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			if (DeferLevel == 0 && Grouping)
				RaiseCollectionChanged (e);
		}

		void HandleFilteredListCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			if (DeferLevel == 0 && ActiveList == filteredList && !Grouping)
				RaiseCollectionChanged (e);
		}

		void HandleSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			// Firstly if we are copying the source collection into our filtered list, update
			// the copy with the new changes and compute the actual index of our item in the
			// sorted/grouped/filtered list.
			int actualOldIndex = -1;
			int actualNewIndex = -1;
			bool originalList = ActiveList == SourceCollection;

			if (!originalList) {
				switch (e.Action) {
				case NotifyCollectionChangedAction.Add:
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					break;

				case NotifyCollectionChangedAction.Remove:
					actualOldIndex = IndexOf (e.OldItems [0]);
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					break;

				case NotifyCollectionChangedAction.Replace:
					actualOldIndex = IndexOf (e.OldItems [0]);
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					break;
	
				case NotifyCollectionChangedAction.Reset:
					filteredList.Clear ();
					RootGroup.ClearSubtree ();
					foreach (var o in SourceCollection)
						AddToFilteredAndGroup (o);
					break;
				}
			} else {
				// Raise the collection changed event
				RaiseCollectionChanged (e);
			}

			IsEmpty = ActiveList.Count == 0;
		}
		protected override void RaiseCollectionChanged (NotifyCollectionChangedEventArgs e)
		{
			RaiseCollectionChanged (e, true);
		}

		void RaiseCollectionChanged (NotifyCollectionChangedEventArgs e, bool moveOnReset)
		{
			base.RaiseCollectionChanged (e);

			// Finally update the selected item if needed.
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				int actualNewIndex = e.NewStartingIndex;
				if (actualNewIndex <= CurrentPosition)
					MoveCurrentTo (CurrentPosition + 1);
				break;

			case NotifyCollectionChangedAction.Remove:
				int actualOldIndex = e.OldStartingIndex;
				if (actualOldIndex < CurrentPosition) {
					MoveCurrentTo (CurrentPosition - 1);
				} else if (actualOldIndex == CurrentPosition) {
					if (CurrentAddItem == CurrentItem)
						MoveCurrentTo (CurrentPosition - 1);
					else
						MoveCurrentTo (CurrentPosition);
				}
				break;

			case NotifyCollectionChangedAction.Replace:
				MoveCurrentTo (CurrentPosition);
				break;

			case NotifyCollectionChangedAction.Reset:
				if (moveOnReset)
					MoveCurrentTo (IndexOf (CurrentItem));
				break;
			}
		}

		void AddToFilteredAndGroup (object item)
		{
			// If we're adding an item because of a call to the 'AddNew' method, we
			//
			if (AddToFiltered (item, false) && Grouping && CurrentAddItem == null)
				RootGroup.AddInSubtree (item, Culture, GroupDescriptions);
		}

		void AddToFilteredAndGroupSorted (object item)
		{
			// If we're adding an item because of a call to the 'AddNew' method, we
			//
			if (AddToFiltered (item, true) && Grouping && CurrentAddItem == null)
				RootGroup.AddInSubtree (item, Culture, GroupDescriptions);
		}

		void RemoveFromFilteredAndGroup (object item)
		{
			if (RemoveFromFiltered (item) || true) {
				RootGroup.RemoveInSubtree (item);
			}
		}

		bool AddToFiltered (object item, bool sorted)
		{
			// If we are not adding a new item and we have a filter, see if the item passes the filter
			if (CurrentAddItem == null && Filter != null)
				if (!Filter (item))
					return false;

			// Only do a sorted insert if we are not adding a new item
			if (CurrentAddItem == null && sorted && SortDescriptions.Count > 0) {
				int index = filteredList.BinarySearch (item, new PropertyComparer (SortDescriptions));
				if (index < 0)
					index = ~index;
				filteredList.Insert (index, item);
			} else {
				// If we're not sorting we need to keep the items in the filtered list
				// in the same order as they are in the SourceCollection.
				int insertIndex;
				if (Filter == null) {
					insertIndex = SourceCollection.IndexOf (item);
				} else {
					insertIndex = -1;
					for (int i = 0; i < SourceCollection.Count; i ++) {
						if (Filter (SourceCollection [i]))
							insertIndex ++;
						if (SourceCollection [i] == item)
							break;
					}
					// If everything is filtered out except for this item, we insert at 0.
					// This can happen if the Filter returns false for everything and we use 'AddNew'.
					if (insertIndex == -1)
						insertIndex = 0;

					// If some items are filtered out, SourceCollection.Count will be greater than filteredList.Count
					// so ensure that if we are inserting at the end of the list we don't exceed the filteredList Count.
					insertIndex = Math.Min (insertIndex, filteredList.Count);
				}

				filteredList.Insert (Math.Min (insertIndex, filteredList.Count), item);
			}
			return true;
		}

		bool RemoveFromFiltered (object item)
		{
			return filteredList.Remove (item);
		}

		int IComparer<object>.Compare (object x, object y)
		{
			return IndexOf (x) - IndexOf (y);
		}

		public override bool Contains (object item)
		{
			return ActiveList.Contains (item);
		}

		public override IDisposable DeferRefresh ()
		{
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot defer refresh while adding or editing");

			return new Deferrer (this);
		}

		public override IEnumerator GetEnumerator ()
		{
			if (GroupDescriptions.Count > 0 && RootGroup != null)
				return new GroupEnumerator (RootGroup);
			return ActiveList.GetEnumerator ();
		}

		int IndexOf (object item)
		{
			if (Grouping)
				return RootGroup.IndexOfSubtree (item);
			else
				return ActiveList.IndexOf (item);
		}

		public override bool MoveCurrentTo (object item)
		{
			return MoveCurrentTo (IndexOf (item));
		}

		bool MoveCurrentTo (int position)
		{
			return MoveCurrentTo (position, false);
		}

		object ItemAtIndex (int index)
		{
			if (Groups == null)
				return index < 0 || index >= ActiveList.Count ? null : ActiveList [index];
			foreach (var o in this) {
				if (index == 0)
					return o;
				index --;
			}

			return null;
		}

		bool MoveCurrentTo (int position, bool force)
		{
			return MoveCurrentTo (position, force, true);
		}

		bool MoveCurrentTo (int position, bool force, bool cancellable)
		{
			if (DeferCurrentChanged)
				return false;

			object newItem = ItemAtIndex (position);
			bool raiseEvents = force || CurrentItem != newItem;

			if (raiseEvents) {
				CurrentChangingEventArgs e = new CurrentChangingEventArgs (cancellable);
				RaiseCurrentChanging (e);
				if (e.Cancel) {
					// If we're cancelling the change, ensure that any selector tracking the CurrentItem
					// is forcibly re-updated. This will ensure that if the user modifies the selection
					// by setting Selector.CurrentItem and the ICollectionView cancels the change, the selector
					// will show the correct item
					if (raiseEvents)
						RaiseCurrentChanged (EventArgs.Empty);
					return false;
				}
			}

			IsCurrentAfterLast = position == ActiveList.Count || ActiveList.Count == 0;
			IsCurrentBeforeFirst = position == -1 || ActiveList.Count == 0;
			UpdateCurrentPositionAndItem (position, newItem);

			if (raiseEvents)
				RaiseCurrentChanged (EventArgs.Empty);

			return IsValidSelection;
		}

		public override bool MoveCurrentToFirst ()
		{
			return MoveCurrentTo (0);
		}

		public override bool MoveCurrentToLast ()
		{
			return MoveCurrentTo (ActiveList.Count - 1);
		}

		public override bool MoveCurrentToNext ()
		{
			return CurrentPosition != ActiveList.Count && MoveCurrentTo (CurrentPosition + 1);
		}

		public override bool MoveCurrentToPosition (int position)
		{
			return MoveCurrentTo (position);
		}

		public override bool MoveCurrentToPrevious ()
		{
			return CurrentPosition != -1 && MoveCurrentTo (CurrentPosition - 1);
		}

		public override void Refresh ()
		{
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot refresh while adding or editing an item");

			if (((IDeferRefresh) this).DeferLevel != 0)
				return;

			try {
				DeferLevel ++;
				Groups = null;
				RootGroup.ClearItems ();
	
				if (ActiveList != SourceCollection) {
					filteredList.Clear ();
					foreach (var item in SourceCollection)
						AddToFiltered (item, false);
	
					if (SortDescriptions.Count > 0)
						filteredList.Sort (new PropertyComparer (SortDescriptions));
	
					if (GroupDescriptions.Count > 0 && filteredList.Count > 0) {
						foreach (var item in filteredList)
							RootGroup.AddInSubtree (item, Culture, GroupDescriptions, false);
						Groups = RootGroup.Items;
					}
				}
	
				IsEmpty = ActiveList.Count == 0;
				int index = IndexOf (CurrentItem);
				if (index < 0 && CurrentPosition != -1 && !IsEmpty)
					index = 0;
	
				RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset), false);
				MoveCurrentTo (index, true, false);
			} finally {
				DeferLevel --;
			}
		}

		public override object AddNew ()
		{
			if (((IDeferRefresh) this).DeferLevel != 0)
				throw new InvalidOperationException ("Cannot add a new item while refresh is deferred");

			if (ItemConstructor == null)
				throw new InvalidOperationException ("The underlying collection does not support adding new items");

			if (SourceCollection.IsFixedSize)
				throw new InvalidOperationException ("The source collection is of fixed size");

			// If there's an existing AddNew or Edit, we commit it. Commit the edit first because
			// we're not allowed CommitNew if we're in the middle of an edit.
			if (IsEditingItem)
				CommitEdit ();
			if (IsAddingNew)
				CommitNew ();

			var newObject = ItemConstructor.Invoke (null);
			// FIXME: I need to check the ordering on the events when the source is INCC
			CurrentAddItem = newObject;
			IsAddingNew = true;
			if (Grouping) {
				RootGroup.AddItem (newObject, false);
				HandleRootGroupCollectionChanged (RootGroup, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, newObject, RootGroup.IndexOfSubtree (newObject)));
			}
			AddToSourceCollection (newObject);
			MoveCurrentTo (newObject);

			if (newObject is IEditableObject)
				((IEditableObject) newObject).BeginEdit ();

			UpdateCanAddNewAndRemove ();
			return newObject;
		}

		void AddToSourceCollection (object item)
		{
			SourceCollection.Add (item);
			if (!(SourceCollection is INotifyCollectionChanged)) {
				HandleSourceCollectionChanged (SourceCollection, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, item, SourceCollection.Count - 1));
			}
		}

		void RemoveFromSourceCollection (int index)
		{
			var item = SourceCollection [index];
			SourceCollection.RemoveAt (index);
			if (!(SourceCollection is INotifyCollectionChanged)) {
				HandleSourceCollectionChanged (SourceCollection, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, item, index));
			}
		}

		public override void CancelEdit ()
		{
			if (IsAddingNew)
				throw new InvalidOperationException ("Cannot cancel edit while adding new");

			if (IsEditingItem) {
				if (!CanCancelEdit)
					throw new InvalidOperationException ("Cannot cancel edit when CanCancelEdit is false");

				((IEditableObject) CurrentEditItem).CancelEdit ();
				CurrentEditItem = null;
				IsEditingItem = false;
				CanCancelEdit = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		public override void CancelNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CancelNew while editing an item");

			if (IsAddingNew) {
				if (CurrentAddItem is IEditableObject)
					((IEditableObject) CurrentAddItem).CancelEdit ();
				if (Grouping) {
					RootGroup.RemoveItem (CurrentAddItem);
				}
				RemoveFromSourceCollection (SourceCollection.IndexOf (CurrentAddItem));
				CurrentAddItem = null;
				IsAddingNew = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		public override void CommitEdit ()
		{
			if (IsAddingNew)
				throw new InvalidOperationException ("Cannot cancel edit while adding new");

			if (IsEditingItem) {
				var editItem = CurrentEditItem;

				CurrentEditItem = null;
				IsEditingItem = false;

				if (CanCancelEdit) {
					((IEditableObject) editItem).EndEdit ();
					CanCancelEdit = false;
				}

				UpdateCanAddNewAndRemove ();

				int originalIndex = IndexOf (editItem);
				int newIndex;

				// If we're filtering the item out just nuke it
				if (Filter != null && !Filter (editItem)) {
					RemoveFromFilteredAndGroup (editItem);
					if (CurrentItem == editItem)
						MoveCurrentTo (CurrentPosition);
					return;
				}

				// We could also have changed the property which sorts it
				if (SortDescriptions.Count > 0) {
					// We can't just remove the item and binary search for the correct place as that will change the
					// order of elements which compare as equal and breaks more tests than it fixes. We need to first
					// check to see if the editItem is >= than the previous one and <= the next one. If that is true
					// we need to do nothing. Otherwise we need to binary search either the upper or lower half and
					// find the new index where the editItem should be placed.
					if (originalIndex > 0 && Comparer.Compare (filteredList [originalIndex - 1], editItem) > 0) {
						newIndex = filteredList.BinarySearch (0, originalIndex, editItem, Comparer);
					} else if (originalIndex < (filteredList.Count - 1) && Comparer.Compare (filteredList [originalIndex + 1], editItem) < 0) {
						newIndex = filteredList.BinarySearch (originalIndex + 1, filteredList.Count - (originalIndex + 1), editItem, Comparer);
					} else {
						// We're already in the right place.
						newIndex = originalIndex;
					}
				} else {
					// No sorting == no index change
					newIndex = originalIndex;
				}

				var currentSelection = CurrentItem;
				if (newIndex != originalIndex) {
					if (newIndex < 0)
						newIndex = ~newIndex;

					 // When we remove the element from the original index, our newIndex will be off by 1 as everything
					// gets shuffled down so decrement it here.
					if (newIndex > originalIndex)
						newIndex --;

					DeferCurrentChanged = true;
					filteredList.RemoveAt (originalIndex);
					filteredList.Insert (newIndex, editItem);
					DeferCurrentChanged = false;
				}

				// We may have edited the property which controls which group the item is in
				// so re-seat it
				if (Grouping) {
					DeferCurrentChanged = true;
					RootGroup.RemoveInSubtree (editItem);
					RootGroup.AddInSubtree (editItem, Culture, GroupDescriptions);
					DeferCurrentChanged = false;
					newIndex = IndexOf (currentSelection);
				}

				if (originalIndex != newIndex) {
					MoveCurrentTo (IndexOf (currentSelection));
				}
			}
		}

		public override void CommitNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CommitNew while editing an item");
			if (IsAddingNew) {
				if (CurrentAddItem is IEditableObject)
					((IEditableObject) CurrentAddItem).EndEdit ();

				RootGroup.RemoveInSubtree (CurrentAddItem);
				if (Filter != null && !Filter (CurrentAddItem)) {
					RemoveFromSourceCollection (SourceCollection.IndexOf (CurrentAddItem));
				} else {
					// When adding a new item, we initially put it in the root group. Once it's committed
					// we need to place it in the correct subtree group.
					if (Grouping) {
						RootGroup.AddInSubtree (CurrentAddItem, Culture, GroupDescriptions);
					}

					// The item was not filtered out of the tree. Do we need to resort it?
					if (SortDescriptions.Count > 0) {
						// The newly added item is at the end of the array. If we're sorting, we may have to move it.
						// Use a binary search to figure out where the item should be in the list and put it in there.
						int actualIndex = filteredList.IndexOf (CurrentAddItem);
						int sortedIndex = filteredList.BinarySearch (0, filteredList.Count - 1, CurrentAddItem, new PropertyComparer (SortDescriptions));
						if (sortedIndex < 0)
							sortedIndex = ~sortedIndex;

						if (actualIndex != sortedIndex) {
							MoveAndSelectFromSorted (actualIndex, sortedIndex, CurrentItem);
						}
					}
				}
				CurrentAddItem = null;
				IsAddingNew = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		void MoveAndSelectFromSorted (int removeIndex, int addIndex, object item)
		{
			DeferCurrentChanged = true;
			filteredList.RemoveAt (removeIndex);
			filteredList.Insert (addIndex, item);
			DeferCurrentChanged = false;
			UpdateCurrentPositionAndItem (IndexOf (item), item);
		}

		public override void EditItem (object item)
		{
			// We can't edit an item which hasn't been comitted.
			if (IsAddingNew && item == CurrentAddItem)
				return;

			if (IsAddingNew)
				CommitNew ();
			if (IsEditingItem)
				CommitEdit ();

			CurrentEditItem = item;
			IsEditingItem = true;
			if (item is IEditableObject) {
				CanCancelEdit = true;
				((IEditableObject) item).BeginEdit ();
			}
			UpdateCanAddNewAndRemove ();
		}

		public override void Remove (object item)
		{
			if (!CanRemove)
				throw new InvalidOperationException ("Removing is not supported by this collection");
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot remove an item when adding or editing an item");
			int index = SourceCollection.IndexOf (item);
			if (index != -1)
				RemoveFromSourceCollection (index);
		}

		public override void RemoveAt (int index)
		{
			if (!CanRemove)
				throw new InvalidOperationException ("Removing is not supported by this collection");
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot remove an item when adding or editing an item");

			RemoveFromSourceCollection (SourceCollection.IndexOf (ItemAtIndex (index)));
		}

		void UpdateCanAddNewAndRemove ()
		{
			var value = ItemConstructor != null && !SourceCollection.IsFixedSize && !IsEditingItem;
			if (value != CanAddNew)
				CanAddNew = value;

			value = !SourceCollection.IsFixedSize && !IsEditingItem && !IsAddingNew;
			if (value != CanRemove)
				CanRemove = value;
		}

		void UpdateCurrentPositionAndItem (int position, object item)
		{
			bool emitPositionChanged = CurrentPosition != position;
			bool emitItemChanged = CurrentItem != item;

			CurrentPosition = position;
			CurrentItem = item;

			if (emitPositionChanged)
				RaisePropertyChanged ("CurrentPosition");
			if (emitItemChanged)
				RaisePropertyChanged ("CurrentItem");
		}
	}
}
