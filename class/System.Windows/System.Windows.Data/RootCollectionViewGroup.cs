using System;
using System.Collections.Generic;
using System.Collections.Specialized;

using Mono;
using System.Windows.Data;
using System.ComponentModel;
using System.Globalization;
using System.Collections;

namespace System.Windows {

	class RootCollectionViewGroup : StandardCollectionViewGroup, INotifyCollectionChanged {

		public event NotifyCollectionChangedEventHandler CollectionChanged;

		IList WrappedList {
			get; set;
		}

		public RootCollectionViewGroup (IList wrappedList, StandardCollectionViewGroup parent, object name, int depth, bool isBottomLevel, SortDescriptionCollection sorters)
			: base (parent, name, depth, isBottomLevel, sorters)
		{
			WrappedList = wrappedList;
		}


		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions)
		{
			AddInSubtree (item, culture, descriptions, true);
		}

		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions, bool allowSorting)
		{
			AddInSubtree (this, item, culture, descriptions, allowSorting);
		}

		void AddInSubtree (StandardCollectionViewGroup group, object item, CultureInfo culture, IList<GroupDescription> descriptions, bool allowSorting)
		{
			int depth = group.Depth;
			if (group.IsBottomLevel) {
				group.AddItem (item, allowSorting, WrappedList);
				CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, item, IndexOfSubtree (item)));
			} else {
				var desc = descriptions [group.Depth];
				var groupNames = desc.GroupNameFromItem (item, group.Depth, culture);
				if (groupNames is IList) {
					foreach (var name in (IList) groupNames)
						AddInSubtree (group, item, culture, descriptions, allowSorting, name);
				} else {
					AddInSubtree (group, item, culture, descriptions, allowSorting, groupNames);
				}
			}
		}

		void AddInSubtree (StandardCollectionViewGroup group, object item, CultureInfo culture, IList<GroupDescription> descriptions, bool allowSorting, object name)
		{
			bool added = false;
			foreach (StandardCollectionViewGroup g in group.Items) {
				var desc = descriptions [group.Depth];
				if (desc.NamesMatch (g.Name, name)) {
					AddInSubtree (g, item, culture, descriptions, allowSorting);
					added = true;
				}
			}

			if (!added) {
				int depth = group.Depth + 1;
				var g = new StandardCollectionViewGroup (group, name, depth, depth == descriptions.Count, group.Sorters);
				group.AddItem (g, allowSorting, WrappedList);
				AddInSubtree (g, item, culture, descriptions, allowSorting);
			}
		}


		internal void ClearSubtree ()
		{
			ProtectedItems.Clear ();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
			return;
//			if (IsBottomLevel) {
//				ClearItems ();
//			} else {
//				foreach (StandardCollectionViewGroup g in Items)
//					g.ClearSubtree ();
//			}
		}

		public int IndexOfSubtree (object item)
		{
			int index = 0;
			return IndexOfSubtree (this, item, ref index) ? index : -1;
		}

		bool IndexOfSubtree (StandardCollectionViewGroup group, object item, ref int index)
		{
			for (int i = 0; i < group.Items.Count; i ++) {
				var v = group.Items [i];
				if (v == item) {
					return true;;
				} else if (v is StandardCollectionViewGroup) {
					if (IndexOfSubtree ((StandardCollectionViewGroup) v, item, ref index))
						return true;
				} else {
					index ++;
				}
			}
			return false;
		}

		public bool RemoveInSubtree (object item)
		{
			int index = 0;
			return RemoveInSubtree (this, item, ref index);
		}

		bool RemoveInSubtree (StandardCollectionViewGroup group, object item, ref int index)
		{
			// The same item can exist in multiple groups. We need to remove it from every group
			// and emit the CollectionChanged event with the correct index.
			bool removed = false;

			for (int i = 0; i < group.Items.Count; i++) {
				var groupItem = group.Items [i];
				if (groupItem is StandardCollectionViewGroup) {
					var subGroup = (StandardCollectionViewGroup) groupItem;
					removed |= RemoveInSubtree (subGroup, item, ref index);
					if (subGroup.Items.Count == 0) {
						i--;
						group.RemoveItem (subGroup);
					}
				} else if (groupItem == item) {
					// If we remove the item, the next item we check will be at
					// the same index, so do not increment it here.
					removed = true;
					group.RemoveItem (item);
					i--;
					CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, item, index));
				} else {
					// We only need to increment the index if we do *not* remove the item.
					index ++;
				}
			}

			return removed;
		}

		public override bool IsBottomLevel {
			get { return false; }
		}
	}
}

