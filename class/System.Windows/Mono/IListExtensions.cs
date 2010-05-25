using System;
using System.Collections;
using System.Collections.Generic;

namespace Mono
{
	static class IListExtensions
	{
		public static void AddRange<T> (this IList<T> list, IEnumerable<T> items)
		{
			if (list is List<T>) {
				((List<T>) list).AddRange (list);
			} else {
				foreach (var v in items)
					list.Add (v);
			}
		}

		public static int BinarySearch<T> (this IList<T> list, T item, IComparer<T> comparer)
		{
			// cache this in case we need it
			if (comparer == null)
				comparer = Comparer<T>.Default;

			int iMin = 0;
			int iMax = list.Count - 1;
			int iCmp = 0;
			try {
				while (iMin <= iMax) {
					// Be careful with overflow
					// http://googleresearch.blogspot.com/2006/06/extra-extra-read-all-about-it-nearly.html
					int iMid = iMin + ((iMax - iMin) / 2);
					T elt = list [iMid];

					iCmp = comparer.Compare (elt, item);

					if (iCmp == 0)
						return iMid;
					else if (iCmp > 0)
						iMax = iMid - 1;
					else
						iMin = iMid + 1; // compensate for the rounding down
				}
			}
			catch (Exception e) {
				throw new InvalidOperationException ("Comparer threw an exception.", e);
			}

			return ~iMin;
		}
	}
}

