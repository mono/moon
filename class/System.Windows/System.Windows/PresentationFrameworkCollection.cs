//
// PresentationFrameworkCollection.cs: provides a wrapper to the unmanaged collection class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007, 2008 Novell, Inc.
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

namespace System.Windows {

	public abstract partial class PresentationFrameworkCollection<T> : DependencyObject, IList<T>, IList {
		const bool BoxValueTypes = false;

		public static readonly System.Windows.DependencyProperty CountProperty =
			DependencyProperty.Lookup (Kind.COLLECTION, "Count", typeof (double)); // <- double is not a typo
		
		int IList.Add (object value)
		{
			Add ((T)value);
			return Count;
		}
		
		void IList.Remove (object value)
		{
			Remove ((T) value);
		}
		
		void IList.Insert (int index, object value)
		{
			Insert (index, (T)value);
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
		
		
		public void Clear ()
		{
			if (IsReadOnly)
				throw new InvalidOperationException ("the collection is readonly");

			ClearImpl ();
		}
		
		public void RemoveAt (int index)
		{
			if (IsReadOnly)
				throw new InvalidOperationException ("the collection is readonly");
			RemoveAtImpl (index);
		}

		public void Add (T value)
		{
			if (IsReadOnly)
				throw new InvalidOperationException ("the collection is readonly");
			AddImpl (value);
		}
		
		public void Insert (int index, T value)
		{
			if (IsReadOnly)
				throw new InvalidOperationException ("the collection is readonly");
			InsertImpl (index, value);
		}
		
		public bool Remove (T value)
		{
			if (IsReadOnly)
				throw new InvalidOperationException ("the collection is readonly");
			return RemoveImpl (value);
		}
		
		public T this [int index] {
			get {
				return GetItemImpl (index);
			}
			set {
				if (IsReadOnly)
					throw new InvalidOperationException ("the collection is readonly");
				T current = this [index];
				RemoveAt (index);
				Insert (index, value);
				Notify (NotifyCollectionChangedAction.Replace, value, current, 0);
			}
		}

		public bool Contains (T value)
		{
			return (IndexOfImpl (value) != -1);
		}
		
		public int IndexOf (T value)
		{
			return IndexOfImpl (value);
		}

		// most types that inherits from this throws ArgumentNullException when
		// null value are used - except for ItemCollection
		internal virtual bool NullCheck (NotifyCollectionChangedAction action, T value)
		{
			bool result = (value == null);
			if (result && (action == NotifyCollectionChangedAction.Add))
				throw new ArgumentNullException ();
			return result;
		}

		internal event NotifyCollectionChangedEventHandler ItemsChanged;

		internal void Notify (NotifyCollectionChangedAction action, T value, int index)
		{
			NotifyCollectionChangedEventHandler handler = ItemsChanged;
			if (handler != null) {
				handler (this, new NotifyCollectionChangedEventArgs (action, value, index));
			}
		}

		internal void Notify (NotifyCollectionChangedAction action, T new_value, T old_value, int index)
		{
			NotifyCollectionChangedEventHandler handler = ItemsChanged;
			if (handler != null) {
				handler (this, new NotifyCollectionChangedEventArgs (action, new_value, old_value, index));
			}
		}

		//
		// ICollection members
		//
		public int Count {
			get {
				return NativeMethods.collection_get_count (native);
			}
		}
		
		public void CopyTo (Array array, int index)
		{
			if (array == null)
				throw new ArgumentNullException ("array");
			
			if (index < 0)
				throw new ArgumentOutOfRangeException ("index");
			
			int n = Count;
			
			for (int i = 0; i < n; i++)
				array.SetValue (((IList) this)[i], index + i);
		}
		
		public void CopyTo (T [] array, int index)
		{
			if (array == null)
				throw new ArgumentNullException ("array");
			
			if (index < 0)
				throw new ArgumentOutOfRangeException ("index");
			
			int n = Count;
			
			for (int i = 0; i < n; i++)
				array[index + i] = this[i];
		}

		public object SyncRoot {
			get {
				return this;
			}
		}

		public bool IsSynchronized {
			get {
				return false;
			}
		}
		
		internal sealed class CollectionIterator : IEnumerator, IDisposable {
			IntPtr native_iter;
			Type type;
			
			public CollectionIterator(Type type, IntPtr native_iter)
			{
				this.native_iter = native_iter;
				this.type = type;
			}
			
			public bool MoveNext ()
			{
				return NativeMethods.collection_iterator_next (native_iter);
			}
			
			public void Reset ()
			{
				if (!NativeMethods.collection_iterator_reset (native_iter))
					throw new InvalidOperationException ("The underlying collection has mutated");
			}

			public object Current {
				get {
					IntPtr val;
					
					val = NativeMethods.collection_iterator_get_current (native_iter);
					
					if (val == IntPtr.Zero)
						return null;
					
					return Value.ToObject (type, val);
				}
			}
			
			public void Dispose ()
			{
				if (native_iter != IntPtr.Zero) {
					// This is safe, as it only does a "delete" in the C++ side
					NativeMethods.collection_iterator_destroy (native_iter);
					native_iter = IntPtr.Zero;
				}
				
				GC.SuppressFinalize (this);
			}
			
			~CollectionIterator ()
			{
				Dispose ();
			}
		}
		
		internal sealed class GenericCollectionIterator : IEnumerator<T> {
			IntPtr native_iter;
			
			public GenericCollectionIterator(IntPtr native_iter)
			{
				this.native_iter = native_iter;
			}
			
			public bool MoveNext ()
			{
				return NativeMethods.collection_iterator_next (native_iter);
			}
			
			public void Reset ()
			{
				if (!NativeMethods.collection_iterator_reset (native_iter))
					throw new InvalidOperationException ("The underlying collection has mutated");
			}

			T GetCurrent ()
			{
				IntPtr val;
				
				val = NativeMethods.collection_iterator_get_current (native_iter);
				
				if (val == IntPtr.Zero) {
					// not sure if this is valid,
					// as _get_current returns a
					// Value*
					return default(T);
				}
				
				return (T) Value.ToObject (typeof (T), val);
			}
			
			public T Current {
				get {
					return GetCurrent ();
				}
			}
			
			object System.Collections.IEnumerator.Current {
				get {
					return GetCurrent ();
				}
			}
			
			public void Dispose ()
			{
				if (native_iter != IntPtr.Zero) {
					// This is safe, as it only does a "delete" in the C++ side
					NativeMethods.collection_iterator_destroy (native_iter);
					native_iter = IntPtr.Zero;
				}
				
				GC.SuppressFinalize (this);
			}
			
			~GenericCollectionIterator ()
			{
				Dispose ();
			}
		}
		
		public IEnumerator<T> GetEnumerator ()
		{
			return new GenericCollectionIterator (NativeMethods.collection_get_iterator (native));
		}
		
		IEnumerator IEnumerable.GetEnumerator ()
		{
			return new CollectionIterator (typeof (T), NativeMethods.collection_get_iterator (native));
		}
		
		public bool IsFixedSize {
			get {
				return false;
			}
		}

		public bool IsReadOnly {
			get {
				return IsReadOnlyImpl ();
			}
		}


		// the internal implementations.

		internal void ClearImpl ()
		{
			NativeMethods.collection_clear (native);

			NotifyCollectionChangedEventHandler handler = ItemsChanged;
			if (handler != null) {
				handler (this, new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
			}
		}

		internal virtual void AddImpl (T value)
		{
			AddImpl (value, BoxValueTypes);
		}

		internal void AddImpl (T value, bool boxValueTypes)
		{
			if (NullCheck (NotifyCollectionChangedAction.Add, value))
				throw new ArgumentNullException ();

			Value v = Value.FromObject (value, boxValueTypes);
			int index = NativeMethods.collection_add (native, ref v);
			NativeMethods.value_free_value (ref v);

			Notify (NotifyCollectionChangedAction.Add, value, index);
		}

		internal virtual void InsertImpl (int index, T value)
		{
			InsertImpl (index, value, BoxValueTypes);
		}

		internal void InsertImpl (int index, T value, bool boxValueTypes)
		{
			if (NullCheck (NotifyCollectionChangedAction.Add, value))
				throw new ArgumentNullException ();
			if (index < 0)
				throw new ArgumentOutOfRangeException ();

			Value v = Value.FromObject (value, boxValueTypes);
			NativeMethods.collection_insert (native, index, ref v);
			NativeMethods.value_free_value (ref v);

			Notify (NotifyCollectionChangedAction.Add, value, index);
		}

		internal bool RemoveImpl (T value)
		{
			if (NullCheck (NotifyCollectionChangedAction.Remove, value))
				return false;

			int index = IndexOfImpl (value);
			if (index == -1)
				return false;

			NativeMethods.collection_remove_at (native, index);
			Notify (NotifyCollectionChangedAction.Remove, value, index);
			return true;
		}

		internal void RemoveAtImpl (int index)
		{
			T value = GetItemImpl (index);
			NativeMethods.collection_remove_at (native, index);
			Notify (NotifyCollectionChangedAction.Remove, value, index);
		}

		internal T GetItemImpl (int index)
		{
			IntPtr val = NativeMethods.collection_get_value_at (native, index);
			if (val == IntPtr.Zero)
				return default(T);
			return (T) Value.ToObject (typeof (T), val);
		}

		internal virtual void SetItemImpl (int index, T value)
		{
			SetItemImpl (index, value, BoxValueTypes);
		}

		internal void SetItemImpl (int index, T value, bool boxValueTypes)
		{
			T old = GetItemImpl (index);
			Value v = Value.FromObject (value, boxValueTypes);
			NativeMethods.collection_set_value_at (native, index, ref v);
			NativeMethods.value_free_value (ref v);

			Notify (NotifyCollectionChangedAction.Replace, value, old, index);
		}

		internal virtual int IndexOfImpl (T value)
		{
			return IndexOfImpl (value, BoxValueTypes);
		}

		internal int IndexOfImpl (T value, bool boxValueTypes)
		{
			if (value == null)
				return -1;

			Value v = Value.FromObject (value, boxValueTypes);
			int rv = NativeMethods.collection_index_of (native, ref v);
			NativeMethods.value_free_value (ref v);
			return rv;
		}

		internal virtual bool IsReadOnlyImpl ()
		{
			return false;
		}
	}
}
