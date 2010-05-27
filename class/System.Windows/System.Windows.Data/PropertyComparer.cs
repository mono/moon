using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;

namespace System.Windows.Data {

	class PropertyComparer : IComparer, IComparer <object> {

		List<ListSortDirection> Directions {
			get; set;
		}

		List <PropertyPathWalker> Walkers {
			get; set;
		}

		public PropertyComparer (SortDescriptionCollection sortDescriptions)
		{
			Directions = new List <ListSortDirection> ();
			Walkers = new List <PropertyPathWalker> ();

			foreach (var sorter in sortDescriptions) {
				Directions.Add (sorter.Direction);
				if (string.IsNullOrEmpty (sorter.PropertyName))
					Walkers.Add (null);
				else
					Walkers.Add (new PropertyPathWalker (sorter.PropertyName));
			}

			if (sortDescriptions.Count == 0) {
				Directions.Add (ListSortDirection.Ascending);
				Walkers.Add (null);
			}
		}

		public int Compare (object x, object y)
		{
			object left = null;
			object right = null;

			for (int i = 0; i < Directions.Count; i++) {
				var walker = Walkers [i];
				if (walker == null) {
					left = x;
					right = y;
				} else {
					walker.Update (x);
					left = walker.Value;
	
					walker.Update (y);
					right = walker.Value;

					walker.Update (null);
				}

				int result = Comparer.Default.Compare (left, right);
				if (result != 0)
					return Directions [i] == ListSortDirection.Ascending ? result : -result;
			}

			return 0;
		}
	}
}

