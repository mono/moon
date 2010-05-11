using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Globalization;

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

		public StandardCollectionViewGroup (StandardCollectionViewGroup parent, object name, int depth, bool isBottomLevel)
			: base (name)
		{
			this.isBottomLevel = isBottomLevel;
			Depth = depth;
			Parent = parent;
		}

		internal void AddItem (object item)
		{
			ProtectedItems.Add (item);
			if (!(item is StandardCollectionViewGroup))
				IncrementCount ();
		}

		internal void AddInSubtree (object item, CultureInfo culture, IList<GroupDescription> descriptions)
		{
			if (IsBottomLevel) {
				AddItem (item);
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
					subGroup = new StandardCollectionViewGroup (this, name, depth, depth == descriptions.Count);
					AddItem (subGroup);
				}

				subGroup.AddInSubtree (item, culture, descriptions);
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

