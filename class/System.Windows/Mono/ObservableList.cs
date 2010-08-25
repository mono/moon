using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Mono {

	class ObservableList<T> : IList<T>, IList, INotifyPropertyChanged, INotifyCollectionChanged {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event PropertyChangedEventHandler PropertyChanged;

		public int Count {
			get { return List.Count; }
		}

		public bool IsReadOnly {
			get { return ((IList<T>)List).IsReadOnly; }
		}

		List<T> List {
			get; set;
		}

		public ObservableList ()
		{
			List = new List<T> ();
		}

		public T this[int index] {
			get { return List [index]; }
			set {
				var old = List[index];
				List[index] = value;
				RaiseItemsChanged ();
				CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace, old, value, index));
			}
		}

		public void Add (T item)
		{
			List.Add (item);
			RaiseCountChanged ();
			RaiseItemsChanged();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, item, List.Count - 1));
		}

		public int BinarySearch (int index, int count, T item, IComparer<T> comparer)
		{
			return List.BinarySearch (index, count, item, comparer);
		}

		public void Clear ()
		{
			List.Clear ();
			RaiseCountChanged ();
			RaiseItemsChanged();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		public bool Contains (T item)
		{
			return List.Contains (item);
		}

		public void CopyTo (T[] array, int arrayIndex)
		{
			List.CopyTo (array, arrayIndex);
		}

		public int IndexOf (T item)
		{
			return List.IndexOf (item);
		}

		public void Insert (int index, T item)
		{
			List.Insert (index, item);
			RaiseCountChanged ();
			RaiseItemsChanged();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, item, index));
		}

		public void RemoveAt (int index)
		{
			var old = List[index];
			List.RemoveAt (index);
			RaiseCountChanged ();
			RaiseItemsChanged();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, old, index));
		}

		public bool Remove (T item)
		{
			int index = List.IndexOf (item);
			if (index != -1)
				RemoveAt (index);
			return index != -1;
		}

		public void Sort ()
		{
			List.Sort ();
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		public void Sort (IComparer<T> comparer)
		{
			List.Sort (comparer);
			CollectionChanged.Raise (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			return GetEnumerator ();
		}

		public IEnumerator<T> GetEnumerator ()
		{
			return List.GetEnumerator ();
		}

		int IList.Add (object value)
		{
			Add ((T)value);
			return List.Count - 1;
		}

		bool IList.Contains (object value)
		{
			return Contains ((T) value);
		}

		int IList.IndexOf (object value)
		{
			return IndexOf ((T) value);
		}

		void IList.Insert (int index, object value)
		{
			Insert (index, (T) value);
		}

		void IList.Remove (object value)
		{
			Remove ((T) value);
		}

		void IList.RemoveAt (int index)
		{
			RemoveAt (index);
		}

		bool IList.IsFixedSize {
			get { return ((IList) List).IsFixedSize; }
		}

		object IList.this[int index] {
			get { return (T) this[index]; }
			set { this[index] = (T) value; }
		}

		public void CopyTo (Array array, int index)
		{
			((IList) List).CopyTo (array, index);
		}

		public bool IsSynchronized {
			get { return ((IList) List).IsSynchronized;}
		}

		public object SyncRoot {
			get { return ((IList) List).SyncRoot;}
		}

		void RaiseCountChanged ()
		{
			RaisePropertyChanged ("Count");
		}

		void RaiseItemsChanged ()
		{
			RaisePropertyChanged ("Item[]");
		}

		void RaisePropertyChanged (string property)
		{
			var h = PropertyChanged;
			if (h != null)
				h (this, new PropertyChangedEventArgs (property));
		}
	}
}

