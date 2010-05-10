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

		int Count {
			get { return filteredList.Count; }
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
			get { return CurrentPosition >= 0 && CurrentPosition < filteredList.Count; }
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

		public IEnumerable SourceCollection {
			get; private set;
		}

		public StandardCollectionView (IEnumerable list)
		{
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

			SourceCollection = list;
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			var interfaces = list.GetType ().GetInterfaces ();
			foreach (var t in interfaces)
				if (t.IsGenericType && t.GetGenericTypeDefinition () == typeof (IList<>))
					ItemType = t.GetGenericArguments () [0];

			CanAddNew = ItemType != null && list is IList && !((IList)list).IsFixedSize;;
			CanRemove = list is IList && !((IList)list).IsFixedSize;
			filteredList = new List <object> (list.Cast <object> ());
			CurrentPosition = -1;
			MoveCurrentToPosition (0);

			if (list is INotifyCollectionChanged)
				((INotifyCollectionChanged) list).CollectionChanged += (o, e) => Refresh ();

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged) SortDescriptions).CollectionChanged += (o, e) => Refresh ();
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
			return filteredList.GetEnumerator ();
		}

		public bool MoveCurrentTo (object item)
		{
			return MoveCurrentTo (filteredList.IndexOf (item));
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

			IsCurrentAfterLast = position == filteredList.Count || Count == 0;
			IsCurrentBeforeFirst = position == -1 || Count == 0;
			CurrentPosition = position;
			CurrentItem = position < 0 || position >= filteredList.Count ? null : filteredList [position];

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
			return MoveCurrentTo (filteredList.Count - 1);
		}

		public bool MoveCurrentToNext ()
		{
			return CurrentPosition != filteredList.Count && MoveCurrentTo (CurrentPosition + 1);
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
				RootGroup = new StandardCollectionViewGroup (null);

			filteredList.Clear ();
			foreach (var item in SourceCollection)
				if (filter == null || Filter (item))
					filteredList.Add (item);

			if (SortDescriptions.Count > 0)
				filteredList.Sort (new PropertyComparer (SortDescriptions));

			Groups = null;
			RootGroup.ClearItems ();
			if (GroupDescriptions.Count > 0 && filteredList.Count > 0) {
				foreach (var item in filteredList)
					AppendToGroup (item, 0, RootGroup);
				Groups = RootGroup.Items;
			}

			if (filteredList.Count > 0) {
				MoveCurrentTo (CurrentPosition, true);
			} else {
				MoveCurrentTo (-1);
			}

			var h = CollectionChanged;
			if (h != null)
				h (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		void AppendToGroup (object item, int depth, StandardCollectionViewGroup group)
		{
			if (depth < GroupDescriptions.Count) {
				var desc = GroupDescriptions [depth];
				var name = desc.GroupNameFromItem (item, depth, Culture);
				StandardCollectionViewGroup subGroup = null;
				foreach (StandardCollectionViewGroup g in group.Items) {
					if (desc.NamesMatch (g.Name, name)) {
						subGroup = g;
						break;
					}
				}
				if (subGroup == null) {
					subGroup = new StandardCollectionViewGroup (group, name, depth == (GroupDescriptions.Count - 1));
					group.AddItem (subGroup);
				}

				AppendToGroup (item, depth + 1, subGroup);
			} else {
				group.AddItem (item);
			}
		}

		public object AddNew ()
		{
			if (ItemType == null)
				throw new InvalidOperationException ("The underlying collection does not support adding new items");

			if (((IList) SourceCollection).IsFixedSize)
				throw new InvalidOperationException ("The source collection is of fixed size");

			// If there's an existing AddNew, we commit it
			CommitNew ();
			var newObject = Activator.CreateInstance (ItemType);
			((IList) SourceCollection).Add (newObject);
			CurrentAddItem = newObject;
			IsAddingNew = true;
			return newObject;
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
			}
		}

		public void CancelNew ()
		{
			if (IsAddingNew) {
				((IList) SourceCollection).Remove (CurrentAddItem);
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
			}
		}

		public void CommitNew ()
		{
			if (IsAddingNew) {
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
		}

		public void Remove (object item)
		{
			((IList) SourceCollection).Remove (item);
		}

		public void RemoveAt (int index)
		{
			((IList) SourceCollection).RemoveAt (index);
		}
	}
}
