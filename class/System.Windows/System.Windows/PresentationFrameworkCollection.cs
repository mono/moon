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

namespace System.Windows {

	public abstract partial class PresentationFrameworkCollection<T> : DependencyObject, IList<T>, IList {

		public static readonly System.Windows.DependencyProperty CountProperty =
			DependencyProperty.Lookup (Kind.COLLECTION, "Count", typeof (int));
		
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
			NativeMethods.collection_clear (native);
		}
		
		public void RemoveAt (int index)
		{
			NativeMethods.collection_remove_at (native, index);
		}

		public void Add (T value)
		{
			AddImpl (value);
		}
		
		public void Insert (int index, T value)
		{
			InsertImpl (index, value);
		}
		
		public bool Remove (T value)
		{
			return RemoveImpl (value);
		}
		
		public T this [int index] {
			get {
				return GetItemImpl (index);
			}
			set {
				SetItemImpl (index, value);
			}
		}

		public bool Contains (T value)
		{
			return ContainsImpl (value);
		}
		
		public int IndexOf (T value)
		{
			return IndexOfImpl (value);
		}


		internal void AddImpl (T value)
		{
			if (value == null)
				throw new ArgumentNullException ();

			Value v = Value.FromObject (value);
			NativeMethods.collection_add (native, ref v);
			NativeMethods.value_free_value (ref v);
		}

		internal void InsertImpl (int index, T value)
		{
			if (value == null)
				throw new ArgumentNullException ();
			if (index < 0)
				throw new ArgumentOutOfRangeException ();

			Value v = Value.FromObject (value);
			NativeMethods.collection_insert (native, index, ref v);
			NativeMethods.value_free_value (ref v);
		}

		internal bool RemoveImpl (T value)
		{
			if (value == null)
				return false;

			Value v = Value.FromObject (value);
			bool rv = NativeMethods.collection_remove (native, ref v);
			NativeMethods.value_free_value (ref v);
			return rv;
		}

		internal T GetItemImpl (int index)
		{
			IntPtr val = NativeMethods.collection_get_value_at (native, index);
			if (val == IntPtr.Zero)
				return default(T);
			return (T) Value.ToObject (typeof (T), val);
		}

		internal void SetItemImpl (int index, T value)
		{
			Value v = Value.FromObject (value);
			NativeMethods.collection_set_value_at (native, index, ref v);
			NativeMethods.value_free_value (ref v);
		}

		internal int IndexOfImpl (T value)
		{
			if (value == null)
				return -1;

			Value v = Value.FromObject (value);
			int rv = NativeMethods.collection_index_of (native, ref v);
			NativeMethods.value_free_value (ref v);
			return rv;
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
		
		static internal Exception GetInvalid ()
		{
			return new InvalidOperationException ("The underlying collection has mutated");
		}
		
		internal class CollectionIterator : System.Collections.IEnumerator {
			IntPtr native_iter;
			Type type;
			
			public CollectionIterator(Type type, IntPtr native_iter)
			{
				this.native_iter = native_iter;
			}
			
			public bool MoveNext ()
			{
				int r = NativeMethods.collection_iterator_next (native_iter);

				if (r == -1)
					throw GetInvalid ();
				
				return r == 1;
			}
			
			public void Reset ()
			{
				if (NativeMethods.collection_iterator_reset (native_iter))
					return;

				throw GetInvalid ();
			}

			public object Current {
				get {
					int error;
					IntPtr val = NativeMethods.collection_iterator_get_current (native_iter, out error);

					if (error == 1)
						throw GetInvalid ();
					
					if (val == IntPtr.Zero)
						return null;
					
					return Value.ToObject (type, val);
				}
			}

			~CollectionIterator ()
			{
				// This is safe, as it only does a "delete" in the C++ side
				NativeMethods.collection_iterator_destroy (native_iter);
			}
		}
		
		internal class GenericCollectionIterator : IEnumerator<T> {
			IntPtr native_iter;
			
			public GenericCollectionIterator(IntPtr native_iter)
			{
				this.native_iter = native_iter;
			}
			
			public bool MoveNext ()
			{
				int r = NativeMethods.collection_iterator_next (native_iter);

				if (r == -1)
					throw GetInvalid ();
				
				return r == 1;
			}
			
			public void Reset ()
			{
				if (NativeMethods.collection_iterator_reset (native_iter))
					return;

				throw GetInvalid ();
			}

			T GetCurrent ()
			{
				int error;
				IntPtr val = NativeMethods.collection_iterator_get_current (native_iter, out error);

				if (error == 1)
					throw GetInvalid ();
				
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
				if (native_iter != IntPtr.Zero){
					// This is safe, as it only does a "delete" in the C++ side
					NativeMethods.collection_iterator_destroy (native_iter);
					native_iter = IntPtr.Zero;
				}
			}
			
			~GenericCollectionIterator ()
			{
				if (native_iter != IntPtr.Zero){
					// This is safe, as it only does a "delete" in the C++ side
					NativeMethods.collection_iterator_destroy (native_iter);
					native_iter = IntPtr.Zero;
				}
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
		
		internal virtual bool ContainsImpl (object value)
		{
			if (value == null)
				return false;

			Value v = Value.FromObject (value);
			return NativeMethods.collection_index_of (native, ref v) != -1;
		}
		
		public bool IsFixedSize {
			get {
				return false;
			}
		}

		public bool IsReadOnly {
			get {
				return false;
			}
		}

	}
}
