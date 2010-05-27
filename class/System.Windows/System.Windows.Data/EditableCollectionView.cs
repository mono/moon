using System;
using System.Collections;
using System.ComponentModel;
using System.Windows.Data;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;

namespace System.Windows {

	abstract class EditableCollectionView : CollectionView, IEditableCollectionView {

		INPCProperty<bool> canAddNew;
		INPCProperty<bool> canCancelEdit;
		INPCProperty<bool> canRemove;
		INPCProperty<object> currentAddItem;
		INPCProperty<object> currentEditItem;
		INPCProperty<bool> isAddingNew;
		INPCProperty<bool> isEditingItem;
		INPCProperty<NewItemPlaceholderPosition> newItemPlaceholderPosition;

		public bool CanAddNew {
			get { return canAddNew.Value; }
			protected set { canAddNew.Value = value;}
		}

		public bool CanCancelEdit {
			get { return canCancelEdit.Value; }
			protected set {
				if (CanCancelEdit != value)
					canCancelEdit.Value = value;
			}
		}

		public bool CanRemove {
			get { return canRemove.Value; }
			protected set { canRemove.Value = value;}
		}

		public object CurrentAddItem {
			get { return currentAddItem.Value; }
			protected set { currentAddItem.Value = value; }
		}

		public object CurrentEditItem {
			get { return currentEditItem.Value; }
			protected set { currentEditItem.Value = value; }
		}

		public bool IsAddingNew {
			get { return isAddingNew.Value; }
			protected set { isAddingNew.Value = value;}
		}

		public bool IsEditingItem {
			get { return isEditingItem.Value; }
			protected set { isEditingItem.Value = value;}
		}

		public NewItemPlaceholderPosition NewItemPlaceholderPosition {
			get { return newItemPlaceholderPosition.Value; }
			set { newItemPlaceholderPosition.Value = value;}
		}

		protected EditableCollectionView (IEnumerable collection)
			: base (collection)
		{
			canAddNew = INPCProperty.Create (() => CanAddNew, PropertyChangedFunc);
			canCancelEdit = INPCProperty.Create (() => CanCancelEdit, PropertyChangedFunc);
			canRemove = INPCProperty.Create (() => CanRemove, PropertyChangedFunc);
			currentAddItem = INPCProperty.Create (() => CurrentAddItem, PropertyChangedFunc);
			currentEditItem = INPCProperty.Create (() => CurrentEditItem, PropertyChangedFunc);
			isAddingNew = INPCProperty.Create (() => IsAddingNew, PropertyChangedFunc);
			isEditingItem = INPCProperty.Create (() => IsEditingItem, PropertyChangedFunc);
			newItemPlaceholderPosition = INPCProperty.Create (() => NewItemPlaceholderPosition, PropertyChangedFunc);
		}

		public abstract object AddNew ();
		public abstract void CancelEdit ();
		public abstract void CancelNew ();
		public abstract void CommitEdit ();
		public abstract void CommitNew ();
		public abstract void EditItem (object item);
		public abstract void Remove (object item);
		public abstract void RemoveAt (int index);
	}
}

