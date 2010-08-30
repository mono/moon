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

