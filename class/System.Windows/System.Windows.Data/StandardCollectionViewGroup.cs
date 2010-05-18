using System;
using System.ComponentModel;
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
				var name = desc.GroupNameFromItem (item, Depth, culture);
				StandardCollectionViewGroup subGroup = null;
				foreach (StandardCollectionViewGroup group in Items) {
					if (desc.NamesMatch (group.Name, name)) {
						subGroup = group;
						break;
					}
				}
				if (subGroup == null) {
					int depth = Depth + 1;
					subGroup = new StandardCollectionViewGroup (this, name, depth, depth == descriptions.Count, Sorters);
					AddItem (subGroup, allowSorting);
				}

				subGroup.AddInSubtree (item, culture, descriptions, allowSorting);
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
			int overallIndex = 0;
			foreach (var o in ProtectedItems) {
				int index = -1;
				var group = o as StandardCollectionViewGroup;
				if (group != null) {
					index = group.IndexOfSubtree (item);
				} else {
					index = IndexOf (item);
				}
				if (index > 0)
					return index + overallIndex;
				if (group != null)
					overallIndex += group.ItemCount;
				else
					overallIndex ++;
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
			if (IsBottomLevel) {
				if (RemoveItem (item))
					return true;
			} else {
				foreach (StandardCollectionViewGroup group in Items)
					group.RemoveInSubtree (item);
			}
			return false;
		}
	}
}

