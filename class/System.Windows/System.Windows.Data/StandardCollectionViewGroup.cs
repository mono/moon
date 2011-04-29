using System;
using System.ComponentModel;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;

using Mono;
using System.Collections.Specialized;

namespace System.Windows.Data {

	class StandardCollectionViewGroup : CollectionViewGroup {

		bool isBottomLevel;

		public int Depth {
			get; private set;
		}

		public override bool IsBottomLevel {
			get { return isBottomLevel; }
		}

		StandardCollectionViewGroup Parent {
			get; set;
		}

		public SortDescriptionCollection Sorters {
			get; private set;
		}

		public StandardCollectionViewGroup (StandardCollectionViewGroup parent, object name, int depth, bool isBottomLevel, SortDescriptionCollection sorters)
			: base (name)
		{
			this.isBottomLevel = isBottomLevel;
			Depth = depth;
			Parent = parent;
			Sorters = sorters;
		}

		internal void AddItem (object item, bool allowSorting, IList wrappedList)
		{
			int index = ProtectedItems.Count;
			if (allowSorting && !(item is CollectionViewGroup)) {
				var comparer = Sorters.Count > 0 ? new PropertyComparer (Sorters) : null;

				for (int i = 0; i < ProtectedItems.Count; i++) {
					int comparison;
					if (comparer != null)
						comparison = comparer.Compare (item, ProtectedItems [i]);
					else
						comparison = wrappedList.IndexOf (item).CompareTo (wrappedList.IndexOf (ProtectedItems [i]));

					if (comparison < 0) {
						index = i;
						break;
					}
				}
			}

			ProtectedItems.Insert (index, item);
			if (!(item is StandardCollectionViewGroup))
				IncrementCount ();
		}

		internal void ClearItems ()
		{
			ProtectedItems.Clear ();
			ProtectedItemCount = 0;
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

		internal bool RemoveItem (object item)
		{
			if (ProtectedItems.Remove (item)) {
				if (!(item is StandardCollectionViewGroup))
					DecrementCount ();
				return true;
			}

			return false;
		}
	}
}

