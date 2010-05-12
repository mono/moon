using System;
using System.Collections;
using System.ComponentModel;
using System.Windows.Data;

namespace System.Windows {

	static class CollectionView {
		public static ICollectionView Create (IEnumerable collection)
		{
			if (collection is IList)
				return new StandardCollectionView ((IList) collection);
			return new EnumerableCollectionView (collection);
		}
	}
}

