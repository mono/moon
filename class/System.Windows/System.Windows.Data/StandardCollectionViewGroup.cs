using System;
using System.ComponentModel;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;

using Mono;

namespace System.Windows.Data {

	class StandardCollectionViewGroup : CollectionViewGroup {

		bool isBottomLevel;

		int Depth {
			get; set;
		}

		public override bool IsBottomLevel {
			get { return isBottomLevel; }
		}

		StandardCollectionViewGroup Parent {
			get; set;
		}

		SortDescriptionCollection Sorters {
			get; set;
		}

		public StandardCollectionViewGroup (StandardCollectionViewGroup parent, object name, int depth, bool isBottomLevel, SortDescriptionCollection sorters)
			: base (name)
		{
			this.isBottomLevel = isBottomLevel;
			Depth = depth;
			Parent = parent;
			Sorters = sorters;
		}

		internal void AddItem (object item, bool allowSorting)
		{
			int index = ProtectedItems.Count;
			if (allowSorting && Sorters.Count > 0 && !(item is CollectionViewGroup)) {
				var comparer = new PropertyComparer (Sorters);
				for (int i = 0; i < ProtectedItems.Count; i++) {
					if (comparer.Compare (item, ProtectedItems [i]) < 0) {
						index = i;
						break;
					}
				}
			}

			ProtectedItems.Insert (index, item);
			if (!(item is StandardCollectionViewGroup))
				IncrementCount ();
		}

		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions)
		{
			AddInSubtree (item, culture, descriptions, true);
		}

		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions, bool allowSorting)
		{
			if (IsBottomLevel) {
				AddItem (item, allowSorting);
			} else {
				var desc = descriptions [Depth];
				var groupNames = desc.GroupNameFromItem (item, Depth, culture);
				if (groupNames is IList) {
					foreach (var name in (IList) groupNames)
						AddInSubtree (item, culture, descriptions, allowSorting, name);
				} else {
					AddInSubtree (item, culture, descriptions, allowSorting, groupNames);
				}
			}
		}

		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions, bool allowSorting, object name)
		{
			bool added = false;
			foreach (StandardCollectionViewGroup group in Items) {
				var desc = descriptions [Depth];
				if (desc.NamesMatch (group.Name, name)) {
					group.AddInSubtree (item, culture, descriptions, allowSorting);
					added = true;
				}
			}

			if (!added) {
				int depth = Depth + 1;
				var group = new StandardCollectionViewGroup (this, name, depth, depth == descriptions.Count, Sorters);
				AddItem (group, allowSorting);
				group.AddInSubtree (item, culture, descriptions, allowSorting);
			}
		}

		internal void ClearItems ()
		{
			ProtectedItems.Clear ();
			ProtectedItemCount = 0;
		}

		internal void ClearSubtree ()
		{
			if (IsBottomLevel) {
				ClearItems ();
			} else {
				foreach (StandardCollectionViewGroup g in Items)
					g.ClearSubtree ();
			}
		}

		internal void DecrementCount ()
		{
			ProtectedItemCount --;
			if (Parent != null)
				Parent.DecrementCount ();
		}

		internal void IncrementCount ()
		{
			ProtectedItemCount ++;
			if (Parent != null)
				Parent.IncrementCount ();
		}

		internal int IndexOf (object item)
		{
			return ProtectedItems.IndexOf (item);
		}

		internal int IndexOfSubtree (object item)
		{
			// FIXME: Enumerating might not be optimal but it's much easier to get right
			int i = 0;
			var enumerator = new GroupEnumerator (this);
			while (enumerator.MoveNext ()) {
				if (enumerator.Current == item)
					return i;
				i++;
			}
			return -1;
		}

		internal bool RemoveItem (object item)
		{
			if (ProtectedItems.Remove (item)) {
				if (!(item is StandardCollectionViewGroup))
					DecrementCount ();
				return true;
			}

			return false;
		}

		internal bool RemoveInSubtree (object item)
		{
			bool removed = false;
			if (IsBottomLevel) {
				removed |= RemoveItem (item);
			} else {
				foreach (StandardCollectionViewGroup group in Items) {
					if (group.RemoveInSubtree (item)) {
						removed = true;
					}
				}

				for (int i = 0; i < ProtectedItems.Count; i ++) {
					var g = ProtectedItems [i] as StandardCollectionViewGroup;
					if (g != null && g.ProtectedItems.Count == 0) {
						ProtectedItems.Remove (g);
						i --;
					}
				}
			}
			return removed;
		}
	}
}

