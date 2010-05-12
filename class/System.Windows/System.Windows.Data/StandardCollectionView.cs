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
		INPCProperty<object> currentItem;
		INPCProperty<int> currentPosition;
		INPCProperty<bool> isAddingNew;
		INPCProperty<bool> isCurrentAfterLast;
		INPCProperty<bool> isCurrentBeforeFirst;
		INPCProperty<bool> isEditingItem;
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
			get { return currentItem.Value; }
			set {
				if (CurrentItem != value)
					currentItem.Value = value;
			}
		}

		public int CurrentPosition {
			get { return currentPosition.Value; }
			set {
				if (CurrentPosition != value)
					currentPosition.Value = value;
			}
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
			currentItem = INPCProperty.Create (() => CurrentItem, changed);
			currentPosition = INPCProperty.Create (() => CurrentPosition, changed);
			isAddingNew = INPCProperty.Create (() => IsAddingNew, changed);
			isCurrentAfterLast = INPCProperty.Create (() => IsCurrentAfterLast, changed);
			isCurrentBeforeFirst = INPCProperty.Create (() => IsCurrentBeforeFirst, changed);
			isEditingItem = INPCProperty.Create (() => IsEditingItem, changed);
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
			filteredList = new List <object> (SourceCollection.Cast <object> ());
			CurrentPosition = -1;
			MoveCurrentToPosition (0);

			if (SourceCollection is INotifyCollectionChanged)
				((INotifyCollectionChanged) SourceCollection).CollectionChanged += HandleSourceCollectionChanged;

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged) SortDescriptions).CollectionChanged += (o, e) => Refresh ();
		}

		void HandleSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			if (ActiveList == SourceCollection)
				return;

			// FIXME: If i inserted to the middle of the source list it'll be appended at the end
			// of our filtered list. This is probably not right. Similarly for the rest.
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				foreach (object o in e.NewItems)
					AddToFilteredAndGroup (o);
				break;

			case NotifyCollectionChangedAction.Remove:
				foreach (object o in e.OldItems)
					RemoveFromFilteredAndGroup (o);
				break;

			case NotifyCollectionChangedAction.Replace:
				foreach (object o in e.OldItems)
					RemoveFromFilteredAndGroup (o);
				foreach (object o in e.NewItems)
					AddToFilteredAndGroup (o);
				break;

			case NotifyCollectionChangedAction.Reset:
				filteredList.Clear ();
				RootGroup.ClearSubtree ();
				foreach (var o in SourceCollection)
					AddToFilteredAndGroup (o);
				break;
			}
		}

		void AddToFilteredAndGroup (object item)
		{
			// If we're adding an item because of a call to the 'AddNew' method, we
			//
			if (AddToFiltered (item) && Groups != null && CurrentAddItem == null)
				RootGroup.AddInSubtree (item, Culture, GroupDescriptions);
		}

		void RemoveFromFilteredAndGroup (object item)
		{
			if (RemoveFromFiltered (item) && Groups != null)
				RootGroup.AddInSubtree (item, Culture, GroupDescriptions);
		}

		bool AddToFiltered (object item)
		{
			if (Filter == null || Filter (item)) {
				filteredList.Add (item);
				return true;
			}
			return false;
		}

		bool RemoveFromFiltered (object item)
		{
			return filteredList.Remove (item);
		}

		public bool Contains (object item)
		{
			return filteredList.Contains (item);
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

		public bool MoveCurrentTo (object item)
		{
			return MoveCurrentTo (ActiveList.IndexOf (item));
		}

		bool MoveCurrentTo (int position)
		{
			return MoveCurrentTo (position, false);
		}

		bool MoveCurrentTo (int position, bool force)
		{
			if (CurrentPosition == position && !force)
				return IsValidSelection;

			var h = CurrentChanging;
			if (h != null) {
				CurrentChangingEventArgs e = new CurrentChangingEventArgs (true);
				h (this, e);
				if (e.Cancel)
					return true;
			}

			IsCurrentAfterLast = position == ActiveList.Count || ActiveList.Count == 0;
			IsCurrentBeforeFirst = position == -1 || ActiveList.Count == 0;
			CurrentPosition = position;
			CurrentItem = position < 0 || position >= ActiveList.Count ? null : ActiveList [position];

			var h2 = CurrentChanged;
			if (h2 != null)
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
				RootGroup = new StandardCollectionViewGroup (null, null, 0, false);

			Groups = null;
			RootGroup.ClearItems ();

			if (ActiveList != SourceCollection) {
				filteredList.Clear ();
				foreach (var item in SourceCollection)
					AddToFiltered (item);
	
				if (SortDescriptions.Count > 0)
					filteredList.Sort (new PropertyComparer (SortDescriptions));

				if (GroupDescriptions.Count > 0 && filteredList.Count > 0) {
					foreach (var item in filteredList)
						RootGroup.AddInSubtree (item, Culture, GroupDescriptions);
					Groups = RootGroup.Items;
				}
			}

			if (IsAddingNew && CurrentItem == CurrentAddItem) {
				MoveCurrentTo (ActiveList.IndexOf (CurrentAddItem), true);
			} else if (ActiveList.Count > 0) {
				MoveCurrentTo (ActiveList.IndexOf (CurrentItem), true);
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
			CommitEdit ();
			CommitNew ();

			var newObject = Activator.CreateInstance (ItemType);
			// FIXME: I need to check the ordering on the events when the source is INCC
			CurrentAddItem = newObject;
			IsAddingNew = true;
			AddToSourceCollection (newObject);
			if (Groups != null)
				RootGroup.AddItem (newObject);
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
				if (Groups != null) {
					RootGroup.RemoveItem (CurrentAddItem);
				}
				RemoveFromSourceCollection (CurrentAddItem);
				CurrentAddItem = null;
				IsAddingNew = false;
			}
		}

		public void CommitEdit ()
		{
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
				// When adding a new item, we initially put it in the root group. Once it's committed
				// we need to place it in the correct subtree group.
				if (Groups != null) {
					RootGroup.RemoveItem (CurrentAddItem);
					RootGroup.AddInSubtree (CurrentAddItem, Culture, GroupDescriptions);
				}
				Refresh ();
				CurrentAddItem = null;
				IsAddingNew = false;
			}
		}

		public void EditItem (object item)
		{
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
			RemoveFromSourceCollection (SourceCollection[index]);
		}

		void UpdateCanAddNew ()
		{
			var value = ItemType != null && !SourceCollection.IsFixedSize && !IsEditingItem;
			if (value != CanAddNew)
				CanAddNew = value;
		}
	}
}
