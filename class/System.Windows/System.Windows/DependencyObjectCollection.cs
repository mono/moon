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

		public event NotifyCollectionChangedEventHandler CollectionChanged;

		public int Count {
			get { return Mono.NativeMethods.collection_get_count (native); }
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

		void IList.Insert (int index, object value)
		{
			Insert (index, (T) value);
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

		public void RemoveAt (int index)
		{
			var oldItem = this [index];
			Mono.NativeMethods.collection_remove_at (native, index);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Remove, oldItem, index);
		}

		public void Add (T value)
		{
			Value v;
			using ((v = Value.FromObject (value)))
				Mono.NativeMethods.collection_add (native, ref v);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Add, value, Count - 1);
		}

		public void Insert (int index, T value)
		{
			Value v;
			using ((v = Value.FromObject (value)))
				Mono.NativeMethods.collection_insert (native, index, ref v);
			CollectionChanged.Raise (this, NotifyCollectionChangedAction.Add, value, index);
		}

		public bool Remove (T value)
		{
			Value v;
			var oldIndex = IndexOf (value);
			using ((v = Value.FromObject (value))) {
				if (Mono.NativeMethods.collection_remove (native, ref v)) {
					CollectionChanged.Raise (this, NotifyCollectionChangedAction.Remove, value, oldIndex);
					return true;
				}
			}
			return false;
		}

		public T this [int index] {
			get { return (T) Value.ToObject (typeof (object), Mono.NativeMethods.collection_get_value_at (native, index)); }
			set {
				Value v;
				using ((v = Value.FromObject (value))) {
					var oldItem = this [index];
					Mono.NativeMethods.collection_set_value_at (native, index, ref v);
					CollectionChanged.Raise (this, NotifyCollectionChangedAction.Replace, value, oldItem, index);
				}
			}
		}

		public bool Contains (T value)
		{
			Value v;
			using ((v = Value.FromObject (value)))
				return NativeMethods.collection_contains (native, ref v);
		}

		public int IndexOf (T value)
		{
			Value v;
			using ((v = Value.FromObject (value)))
				return NativeMethods.collection_index_of (native, ref v);
		}

		public void CopyTo (T [] array, int index)
		{
			foreach (var v in this)
				array [index ++] = v;
		}

		public IEnumerator<T> GetEnumerator ()
		{
			return new PresentationFrameworkCollection<T>.GenericCollectionIterator (NativeMethods.collection_get_iterator (native));
		}
	}
}
