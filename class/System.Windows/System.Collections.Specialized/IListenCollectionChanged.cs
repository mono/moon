using System;
namespace System.Collections.Specialized {

	public interface IListenCollectionChanged {
		void CollectionChanged (object sender, NotifyCollectionChangedEventArgs e);
	}
}
