using System;
namespace System.Collections.Specialized {

	interface IListenCollectionChanged {
		void CollectionChanged (object sender, NotifyCollectionChangedEventArgs e);
	}
}
