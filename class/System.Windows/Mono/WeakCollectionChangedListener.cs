using System;
using System.Collections.Specialized;

namespace Mono
{
	class WeakCollectionChangedListener : IWeakListener
	{
		WeakReference listener;
		IListenCollectionChanged Listener {
			get { return (IListenCollectionChanged) listener.Target; }
			set { listener = new WeakReference (value); }
		}

		INotifyCollectionChanged Source {
			get; set;
		}

		public WeakCollectionChangedListener (INotifyCollectionChanged source, IListenCollectionChanged listener)
		{
			Source = source;
			Listener = listener;
			Source.CollectionChanged += OnCollectionChanged;;
		}

		public void Detach ()
		{
			Source.CollectionChanged -= OnCollectionChanged;;
			Source = null;
		}

		void OnCollectionChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			var l = Listener;
			if (l == null)
				Source.CollectionChanged -= OnCollectionChanged;
			else
				l.CollectionChanged (o, e);
		}
	}
}

