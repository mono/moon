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

				Console.WriteLine ("Comparing: {0} to {1}. {2} to {3}", x, y, left, right);
				int result = Comparer.Default.Compare (left, right);
				if (result != 0)
					return Directions [i] == ListSortDirection.Ascending ? result : -result;
			}

			return 0;
		}
	}
}

