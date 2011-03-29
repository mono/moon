//
// DependencyObjectCollection.cs:
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


using Mono;
using System;
using System.Windows;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Collections.ObjectModel;

namespace System.Windows {
	public partial class DependencyObjectCollection<T> : DependencyObject, IList<T>, IList, INotifyCollectionChanged {
		List<T> Items;

		public event NotifyCollectionChangedEventHandler CollectionChanged;

		public int Count {
			get { return Items.Count; }
		}

		public bool IsReadOnly {
			get { return false; }
		}

		int IList.Add (object value)
		{
			Add ((T) value);
			return Count;
		}

		void IList.Remove (object value)
		{
			Remove ((T) value);
		}

		void IList.Insert (int index, object item)
		{
			Insert (index, (T) item);
		}

		object IList.this [int index] {
			get { return this[index]; }
			set { this[index] = (T)value; }
		}

		bool IList.Contains (object value)
		{
			return ((IList) this).IndexOf (value) != -1;
		}

		int IList.IndexOf (object value)
		{
			return IndexOf ((T) value);
		}

		void ICollection.CopyTo (Array array, int index)
		{
			foreach (var v in this)
				array.SetValue (v, index ++);
		}

		object ICollection.SyncRoot {
			get { return this; }
		}

		bool ICollection.IsSynchronized {
			get { return false; }
		}
		
		bool System.Collections.IList.IsFixedSize {
			get { return false; }
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			return GetEnumerator ();
		}

		public void Clear ()
		{
			Mono.NativeMethods.collection_clear (native);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Reset);
		}

		void OnCollectionChanged (object sender, NotifyCollectionChangedEventArgs args)
		{
			switch (args.Action) {
			case NotifyCollectionChangedAction.Add:
				Items.Add ((T) args.NewItems[0]);
				break;
			case NotifyCollectionChangedAction.Remove:
				Items.RemoveAt (args.OldStartingIndex);
				break;
			case NotifyCollectionChangedAction.Replace:
				Items[args.OldStartingIndex] = (T) args.NewItems[0];
				break;
			case NotifyCollectionChangedAction.Reset:
				Items.Clear ();
				break;
			}
		}

		void Initialize ()
		{
			NativeMethods.dependency_object_collection_set_sets_parent (native, false);
			CollectionChanged += OnCollectionChanged;
			Items = new List<T> ();
		}

		public void RemoveAt (int index)
		{
			var oldItem = this [index];
			Mono.NativeMethods.collection_remove_at (native, index);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Remove, oldItem, index);
		}

		public void Add (T item)
		{
			Value v;
			using ((v = Value.FromObject (item)))
				Mono.NativeMethods.collection_add (native, ref v);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Add, item, Count - 1);
		}

		public void Insert (int index, T item)
		{
			Value v;
			using ((v = Value.FromObject (item)))
				Mono.NativeMethods.collection_insert (native, index, ref v);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Add, item, index);
		}

		public bool Remove (T item)
		{
			Value v;
			var oldIndex = IndexOf (item);
			using ((v = Value.FromObject (item))) {
				if (Mono.NativeMethods.collection_remove (native, ref v)) {
					CollectionChanged.Raise (this, NotifyCollectionChangedAction.Remove, item, oldIndex);
					return true;
				}
			}
			return false;
		}

		public T this [int index] {
			get { return Items[index]; }
			set {
				Value v;
				using ((v = Value.FromObject (value))) {
					var oldItem = this [index];
					Mono.NativeMethods.collection_set_value_at (native, index, ref v);
					CollectionChanged.Raise (this, NotifyCollectionChangedAction.Replace, value, oldItem, index);
				}
			}
		}

		public bool Contains (T item)
		{
			return Items.Contains (item);
		}

		public int IndexOf (T item)
		{
			return Items.IndexOf (item);
		}

		public void CopyTo (T [] array, int arrayIndex)
		{
			Items.CopyTo (array, arrayIndex);
		}

		public IEnumerator<T> GetEnumerator ()
		{
			return Items.GetEnumerator ();
		}
	}
}
