using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace System.Windows.Data {

	sealed class StandardCollectionView : ICollectionView, IEditableCollectionView, INotifyPropertyChanged, IDeferRefresh {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;
		public event PropertyChangedEventHandler PropertyChanged;

		INPCProperty<bool> canAddNew;
		INPCProperty<bool> canCancelEdit;
		INPCProperty<bool> canRemove;
		INPCProperty<object> currentAddItem;
		INPCProperty<object> currentEditItem;
		INPCProperty<bool> isAddingNew;
		INPCProperty<bool> isCurrentAfterLast;
		INPCProperty<bool> isCurrentBeforeFirst;
		INPCProperty<bool> isEditingItem;
		INPCProperty<bool> isempty;
		INPCProperty<NewItemPlaceholderPosition> newItemPlaceholderPosition;

		INPCProperty<CultureInfo> culture;
		Predicate<object> filter;
		List<object> filteredList;

		public IList ActiveList {
			get {
				if (Filter == null && GroupDescriptions.Count == 0 && SortDescriptions.Count == 0)
					return SourceCollection;
				return filteredList;
			}
		}

		public bool CanAddNew {
			get { return canAddNew.Value; }
			private set { canAddNew.Value = value;}
		}

		public bool CanCancelEdit {
			get { return canCancelEdit.Value; }
			private set {
				if (CanCancelEdit != value)
					canCancelEdit.Value = value;
			}
		}

		public bool CanRemove {
			get { return canRemove.Value; }
			private set { canRemove.Value = value;}
		}

		PropertyComparer Comparer {
			get { return new PropertyComparer (SortDescriptions); }
		}

		public bool IsAddingNew {
			get { return isAddingNew.Value; }
			private set { isAddingNew.Value = value;}
		}

		public bool IsEditingItem {
			get { return isEditingItem.Value; }
			private set { isEditingItem.Value = value;}
		}

		public NewItemPlaceholderPosition NewItemPlaceholderPosition {
			get { return newItemPlaceholderPosition.Value; }
			set { newItemPlaceholderPosition.Value = value;}
		}

		public bool CanFilter {
			get { return true; }
		}

		public bool CanGroup {
			get { return true; }
		}

		public bool CanSort {
			get { return true; }
		}

		public CultureInfo Culture {
			get { return culture.Value; }
			set { culture.Value = value; }
		}

		public object CurrentAddItem {
			get { return currentAddItem.Value; }
			private set { currentAddItem.Value = value; }
		}

		public object CurrentEditItem {
			get { return currentEditItem.Value; }
			private set { currentEditItem.Value = value; }
		}

		public object CurrentItem {
			get; private set;
		}

		public int CurrentPosition {
			get; private set;
		}

		int IDeferRefresh.DeferLevel {
			get; set;
		}

		public Predicate<object> Filter {
			get { return filter; }
			set {
				filter = value;
				Refresh ();
			}
		}

		bool Grouping {
			get { return Groups != null; }
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public ReadOnlyObservableCollection<object> Groups {
			get; private set;
		}

		public bool IsCurrentAfterLast {
			get { return isCurrentAfterLast.Value; }
			private set {
				if (IsCurrentAfterLast != value)
					isCurrentAfterLast.Value = value;
			}
		}

		public bool IsCurrentBeforeFirst {
			get { return isCurrentBeforeFirst.Value; }
			private set {
				if (IsCurrentBeforeFirst != value)
					isCurrentBeforeFirst.Value = value;
			}
		}

		public bool IsEmpty {
			get; private set;
		}

		bool IsValidSelection {
			get { return CurrentPosition >= 0 && CurrentPosition < ActiveList.Count; }
		}

		ConstructorInfo ItemConstructor {
			get; set;
		}

		StandardCollectionViewGroup RootGroup {
			get; set;
		}

		public SortDescriptionCollection SortDescriptions {
			get; private set;
		}

		IEnumerable ICollectionView.SourceCollection {
			get { return SourceCollection; }
		}

		public IList SourceCollection {
			get; private set;
		}

		public StandardCollectionView (IList collection)
		{
			SourceCollection = collection;
			Func<PropertyChangedEventHandler> changed = () => PropertyChanged;

			canAddNew = INPCProperty.Create (() => CanAddNew, changed);
			canCancelEdit = INPCProperty.Create (() => CanCancelEdit, changed);
			canRemove = INPCProperty.Create (() => CanRemove, changed);
			culture = INPCProperty.Create (() => Culture, changed);
			currentAddItem = INPCProperty.Create (() => CurrentAddItem, changed);
			currentEditItem = INPCProperty.Create (() => CurrentEditItem, changed);
			isAddingNew = INPCProperty.Create (() => IsAddingNew, changed);
			isCurrentAfterLast = INPCProperty.Create (() => IsCurrentAfterLast, changed);
			isCurrentBeforeFirst = INPCProperty.Create (() => IsCurrentBeforeFirst, changed);
			isEditingItem = INPCProperty.Create (() => IsEditingItem, changed);
			isempty = INPCProperty.Create (() => IsEmpty, changed);
			newItemPlaceholderPosition = INPCProperty.Create (() => NewItemPlaceholderPosition, changed);

			SourceCollection = collection;
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			var interfaces = SourceCollection.GetType ().GetInterfaces ();
			foreach (var t in interfaces) {
				if (t.IsGenericType && t.GetGenericTypeDefinition () == typeof (IList<>)) {
					Type type = t.GetGenericArguments () [0];
					ItemConstructor = type.GetConstructor (Type.EmptyTypes);
				}
			}

			UpdateCanAddNewAndRemove ();
			filteredList = new List <object> ();
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
		}

		void HandleSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			// Firstly if we are copying the source collection into our filtered list, update
			// the copy with the new changes and compute the actual index of our item in the
			// sorted/grouped/filtered list.
			int actualOldIndex = -1;
			int actualNewIndex = -1;
			bool originalList = ActiveList == SourceCollection;
			NotifyCollectionChangedEventArgs modifed = e;
			if (!originalList) {
				switch (e.Action) {
				case NotifyCollectionChangedAction.Add:
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					actualNewIndex = IndexOf (e.NewItems [0]);
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, e.NewItems [0], actualNewIndex));
					break;

				case NotifyCollectionChangedAction.Remove:
					actualOldIndex = IndexOf (e.OldItems [0]);
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, e.OldItems [0], actualOldIndex));
					break;

				case NotifyCollectionChangedAction.Replace:
					actualOldIndex = IndexOf (e.OldItems [0]);
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					actualNewIndex = IndexOf (e.NewItems [0]);
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, e.OldItems[0], actualOldIndex));
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, e.NewItems [0], actualNewIndex));
					break;
	
				case NotifyCollectionChangedAction.Reset:
					filteredList.Clear ();
					RootGroup.ClearSubtree ();
					foreach (var o in SourceCollection)
						AddToFilteredAndGroup (o);
					modifed = e;
					break;
				}
			} else {
				// Raise the collection changed event
				RaiseCollectionChanged (modifed);
			}

			IsEmpty = ActiveList.Count == 0;

			// Finally update the selected item if needed.
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				if (originalList)
					actualNewIndex = e.NewStartingIndex;
				if (actualNewIndex <= CurrentPosition)
					MoveCurrentTo (CurrentPosition + 1);
				break;

			case NotifyCollectionChangedAction.Remove:
				if (originalList)
					actualOldIndex = e.OldStartingIndex;
				if (actualOldIndex < CurrentPosition) {
					MoveCurrentTo (CurrentPosition - 1);
				} else if (actualOldIndex == CurrentPosition) {
					if (CurrentAddItem == CurrentItem)
						MoveCurrentTo (CurrentPosition - 1);
					else
						MoveCurrentTo (CurrentPosition, true);
				}
				break;

			case NotifyCollectionChangedAction.Replace:
				MoveCurrentTo (CurrentPosition, true);
				break;

			case NotifyCollectionChangedAction.Reset:
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
			if (RemoveFromFiltered (item) && Grouping)
				RootGroup.RemoveInSubtree (item);
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
				filteredList.Add (item);
			}
			return true;
		}

		bool RemoveFromFiltered (object item)
		{
			return filteredList.Remove (item);
		}

		public bool Contains (object item)
		{
			return ActiveList.Contains (item);
		}

		public IDisposable DeferRefresh ()
		{
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot defer refresh while adding or editing");

			return new Deferrer (this);
		}

		public IEnumerator GetEnumerator ()
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
		public bool MoveCurrentTo (object item)
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
			object newItem = ItemAtIndex (position);
			bool raiseEvents = CurrentItem != newItem;

			var h = CurrentChanging;
			if (raiseEvents && h != null) {
				CurrentChangingEventArgs e = new CurrentChangingEventArgs (true);
				h (this, e);
				if (e.Cancel)
					return true;
			}

			IsCurrentAfterLast = position == ActiveList.Count || ActiveList.Count == 0;
			IsCurrentBeforeFirst = position == -1 || ActiveList.Count == 0;
			UpdateCurrentPositionAndItem (position, newItem);

			var h2 = CurrentChanged;
			if (raiseEvents && h2 != null)
				h2 (this, EventArgs.Empty);

			return IsValidSelection;
		}

		public bool MoveCurrentToFirst ()
		{
			return MoveCurrentTo (0);
		}

		public bool MoveCurrentToLast ()
		{
			return MoveCurrentTo (ActiveList.Count - 1);
		}

		public bool MoveCurrentToNext ()
		{
			return CurrentPosition != ActiveList.Count && MoveCurrentTo (CurrentPosition + 1);
		}

		public bool MoveCurrentToPosition (int position)
		{
			return MoveCurrentTo (position);
		}

		public bool MoveCurrentToPrevious ()
		{
			return CurrentPosition != -1 && MoveCurrentTo (CurrentPosition - 1);
		}

		void RaiseCollectionChanged (NotifyCollectionChangedEventArgs e)
		{
			var h = CollectionChanged;
			if (h != null)
				h (this, e);
		}

		void RaisePropertyChanged (string propertyName)
		{
			var h = PropertyChanged;
			if (h != null)
				h (this, new PropertyChangedEventArgs (propertyName));
		}

		public void Refresh ()
		{
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot refresh while adding or editing an item");

			if (((IDeferRefresh) this).DeferLevel != 0)
				return;

			if (RootGroup == null)
				RootGroup = new StandardCollectionViewGroup (null, null, 0, false, SortDescriptions);

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

			MoveCurrentTo (index, true);

			var h = CollectionChanged;
			if (h != null)
				h (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		public object AddNew ()
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
			AddToSourceCollection (newObject);
			if (Grouping)
				RootGroup.AddItem (newObject, false);
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

		void RemoveFromSourceCollection (object item)
		{
			int index = SourceCollection.IndexOf (item);
			SourceCollection.RemoveAt (index);
			if (!(SourceCollection is INotifyCollectionChanged)) {
				HandleSourceCollectionChanged (SourceCollection, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, item, index));
			}
		}

		public void CancelEdit ()
		{
			if (IsAddingNew)
				throw new InvalidOperationException ("Cannot cancel edit while adding new");

			if (IsEditingItem) {
				if (CanCancelEdit) {
					((IEditableObject) CurrentEditItem).CancelEdit ();
				}
				CurrentEditItem = null;
				IsEditingItem = false;
				CanCancelEdit = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		public void CancelNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CancelNew while editing an item");

			if (IsAddingNew) {
				if (CurrentAddItem is IEditableObject)
					((IEditableObject) CurrentAddItem).CancelEdit ();
				if (Grouping) {
					RootGroup.RemoveItem (CurrentAddItem);
				}
				RemoveFromSourceCollection (CurrentAddItem);
				CurrentAddItem = null;
				IsAddingNew = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		public void CommitEdit ()
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
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, editItem, originalIndex));
					if (CurrentItem == editItem)
						MoveCurrentTo (CurrentPosition, true);
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


				if (newIndex != originalIndex) {
					if (newIndex < 0)
						newIndex = ~newIndex;

					 // When we remove the element from the original index, our newIndex will be off by 1 as everything
					// gets shuffled down so decrement it here.
					if (newIndex > originalIndex)
						newIndex --;

					filteredList.RemoveAt (originalIndex);
					filteredList.Insert (newIndex, editItem);
				}

				// We may have edited the property which controls which group the item is in
				// so re-seat it
				if (Grouping) {
					RootGroup.RemoveInSubtree (editItem);
					RootGroup.AddInSubtree (editItem, Culture, GroupDescriptions);
					newIndex = IndexOf (editItem);
				}

				if (originalIndex != newIndex) {
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, editItem, originalIndex));
					RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, editItem, newIndex));
					MoveCurrentTo (IndexOf (CurrentItem), true);
				}
			}
		}

		public void CommitNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CommitNew while editing an item");
			if (IsAddingNew) {
								if (CurrentAddItem is IEditableObject)
					((IEditableObject) CurrentAddItem).EndEdit ();
				if (Filter != null && !Filter (CurrentAddItem)) {
					RemoveFromSourceCollection (CurrentAddItem);
				} else {
					// When adding a new item, we initially put it in the root group. Once it's committed
					// we need to place it in the correct subtree group.
					if (Grouping) {
						RootGroup.RemoveItem (CurrentAddItem);
						RootGroup.AddInSubtree (CurrentAddItem, Culture, GroupDescriptions);
					}

					// The item was not filtered out of the tree. Do we need to resort it?
					if (SortDescriptions.Count > 0) {
						// The newly added item is at the end of the array. If we're sorting, we may have to move it.
						// Use a binary search to figure out where the item should be in the list and put it in there.
						int actualIndex = SourceCollection.IndexOf (CurrentAddItem);
						int sortedIndex = filteredList.BinarySearch (0, filteredList.Count - 1, CurrentAddItem, new PropertyComparer (SortDescriptions));
						if (sortedIndex < 0)
							sortedIndex = ~sortedIndex;

						if (actualIndex != sortedIndex) {
							filteredList.RemoveAt (actualIndex);
							filteredList.Insert (sortedIndex, CurrentAddItem);
							RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, currentAddItem, actualIndex));
							RaiseCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, currentAddItem, sortedIndex));
							if (CurrentAddItem == CurrentItem)
								UpdateCurrentPositionAndItem (sortedIndex, CurrentAddItem);
						}
					}
				}
				CurrentAddItem = null;
				IsAddingNew = false;
				UpdateCanAddNewAndRemove ();
			}
		}

		public void EditItem (object item)
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

		public void Remove (object item)
		{
			if (!CanRemove)
				throw new InvalidOperationException ("Removing is not supported by this collection");
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot remove an item when adding or editing an item");
			RemoveFromSourceCollection (item);
		}

		public void RemoveAt (int index)
		{
			if (!CanRemove)
				throw new InvalidOperationException ("Removing is not supported by this collection");
			if (IsAddingNew || IsEditingItem)
				throw new InvalidOperationException ("Cannot remove an item when adding or editing an item");
			RemoveFromSourceCollection (ItemAtIndex (index));
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
