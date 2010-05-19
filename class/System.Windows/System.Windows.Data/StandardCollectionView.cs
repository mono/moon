using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

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
			get; private set;
		}

		public bool CanGroup {
			get { return true; }
		}

		public bool CanSort {
			get; private set;
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

		Type ItemType {
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
			foreach (var t in interfaces)
				if (t.IsGenericType && t.GetGenericTypeDefinition () == typeof (IList<>))
					ItemType = t.GetGenericArguments () [0];

			UpdateCanAddNew ();
			CanRemove = !SourceCollection.IsFixedSize;
			filteredList = new List <object> ();
			CurrentPosition = -1;
			IsEmpty = ActiveList.Count == 0;
			MoveCurrentToPosition (0);

			if (SourceCollection is INotifyCollectionChanged)
				((INotifyCollectionChanged) SourceCollection).CollectionChanged += HandleSourceCollectionChanged;

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged) SortDescriptions).CollectionChanged += (o, e) => Refresh ();
		}

		void HandleSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			// Firstly if we are copying the source collection into our filtered list, update
			// the copy with the new changes and compute the actual index of our item in the
			// sorted/grouped/filtered list.
			int actualIndex = -1;
			bool originalList = ActiveList == SourceCollection;
			if (!originalList) {
				switch (e.Action) {
				case NotifyCollectionChangedAction.Add:
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					actualIndex = IndexOf (e.NewItems [0]);
					break;

				case NotifyCollectionChangedAction.Remove:
					actualIndex = IndexOf (e.OldItems [0]);
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					break;

				case NotifyCollectionChangedAction.Replace:
					foreach (object o in e.OldItems)
						RemoveFromFilteredAndGroup (o);
					foreach (object o in e.NewItems)
						AddToFilteredAndGroupSorted (o);
					actualIndex = IndexOf (e.NewItems [0]);
					break;
	
				case NotifyCollectionChangedAction.Reset:
					filteredList.Clear ();
					RootGroup.ClearSubtree ();
					foreach (var o in SourceCollection)
						AddToFilteredAndGroup (o);
					break;
				}
			}

			// Secondly raise our collection changed events
			RaiseCollectionChanged (e);
			IsEmpty = ActiveList.Count == 0;

			// Finally update the selected item if needed.
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				if (originalList)
					actualIndex = e.NewStartingIndex;
				if (actualIndex <= CurrentPosition)
					MoveCurrentTo (CurrentPosition + 1);
				break;

			case NotifyCollectionChangedAction.Remove:
				if (originalList)
					actualIndex = e.OldStartingIndex;
				if (actualIndex < CurrentPosition) {
					MoveCurrentTo (CurrentPosition - 1);
				} else if (actualIndex == CurrentPosition) {
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
			if (CurrentPosition == position && !force)
				return IsValidSelection;

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

			if (IsAddingNew && CurrentItem == CurrentAddItem) {
				MoveCurrentTo (IndexOf (CurrentAddItem), true);
			} else if (Grouping) {
				MoveCurrentTo (IndexOf (CurrentItem), true);
			} else if (ActiveList.Count > 0) {
				MoveCurrentTo (CurrentPosition, true);
			} else {
				MoveCurrentTo (-1);
			}

			var h = CollectionChanged;
			if (h != null)
				h (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		public object AddNew ()
		{
			if (((IDeferRefresh) this).DeferLevel != 0)
				throw new InvalidOperationException ("Cannot add a new item while refresh is deferred");

			if (ItemType == null)
				throw new InvalidOperationException ("The underlying collection does not support adding new items");

			if (SourceCollection.IsFixedSize)
				throw new InvalidOperationException ("The source collection is of fixed size");

			// If there's an existing AddNew or Edit, we commit it. Commit the edit first because
			// we're not allowed CommitNew if we're in the middle of an edit.
			if (IsEditingItem)
				CommitEdit ();
			if (IsAddingNew)
				CommitNew ();

			var newObject = Activator.CreateInstance (ItemType);
			// FIXME: I need to check the ordering on the events when the source is INCC
			CurrentAddItem = newObject;
			IsAddingNew = true;
			AddToSourceCollection (newObject);
			if (Grouping)
				RootGroup.AddItem (newObject, false);
			MoveCurrentTo (newObject);
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
				UpdateCanAddNew ();
			}
		}

		public void CancelNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CancelNew while editing an item");

			if (IsAddingNew) {
				if (Grouping) {
					RootGroup.RemoveItem (CurrentAddItem);
				}
				RemoveFromSourceCollection (CurrentAddItem);
				CurrentAddItem = null;
				IsAddingNew = false;
			}
		}

		public void CommitEdit ()
		{
			if (IsAddingNew)
				throw new InvalidOperationException ("Cannot cancel edit while adding new");

			if (IsEditingItem) {
				if (CanCancelEdit) {
					((IEditableObject) CurrentEditItem).EndEdit ();
				}
				CurrentEditItem = null;
				IsEditingItem = false;
				CanCancelEdit = false;
				UpdateCanAddNew ();
			}
		}

		public void CommitNew ()
		{
			if (IsEditingItem)
				throw new InvalidOperationException ("Cannot CommitNew while editing an item");
			if (IsAddingNew) {
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
			}
		}

		public void EditItem (object item)
		{
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
			UpdateCanAddNew ();
		}

		public void Remove (object item)
		{
			RemoveFromSourceCollection (item);
		}

		public void RemoveAt (int index)
		{
			RemoveFromSourceCollection (ItemAtIndex (index));
		}

		void UpdateCanAddNew ()
		{
			var value = ItemType != null && !SourceCollection.IsFixedSize && !IsEditingItem;
			if (value != CanAddNew)
				CanAddNew = value;
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
