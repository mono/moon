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

namespace System.Windows {
	public partial class DependencyObjectCollection<T> : DependencyObject, IList<T>, IList, INotifyCollectionChanged {
#region Interface implementations
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
			Console.WriteLine ("System.Windows.DependencyObjectCollection.<ICollection>.CopyTo: NIEX");
			throw new NotImplementedException ();
		}

		object ICollection.SyncRoot {
			get {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.<ICollection>.SyncRoot: NIEX");
				throw new NotImplementedException ();
			}
		}

		bool ICollection.IsSynchronized {
			get {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.<ICollection>.IsSynchronized: NIEX");
				throw new NotImplementedException ();
			}
		}
		
		bool System.Collections.IList.IsFixedSize {
			get {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.<IList>.IsFixedSize: NIEX");
				throw new NotImplementedException ();
			}
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.<IEnumerable>.GetEnumerator: NIEX");
			throw new NotImplementedException ();
		}
#endregion

		public void Clear ()
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.Clear (): NIEX");
			throw new NotImplementedException ();
		}

		public void RemoveAt (int index)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.RemoveAt (): NIEX");
			throw new NotImplementedException ();
		}

		public void Add (T value)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.Add (): NIEX");
			throw new NotImplementedException ();
		}

		public void Insert (int index, T value)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.Insert (): NIEX");
			throw new NotImplementedException ();
		}

		public bool Remove (T value)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.Remove (): NIEX");
			throw new NotImplementedException ();
		}

		public T this [int index] {
			get {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.get_this (): NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.set_this (): NIEX");
				throw new NotImplementedException ();
			}
		}

		public bool Contains (T value)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.Add (): NIEX");
			throw new NotImplementedException ();
		}

		public int IndexOf (T value)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.IndexOf (): NIEX");
			throw new NotImplementedException ();
		}

		public int Count {
			get {
				return NativeMethods.collection_get_count (native);
			}
		}

		public void CopyTo (T [] array, int index)
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.CopyTo: NIEX");
			throw new NotImplementedException ();
		}

		public bool IsReadOnly {
			get {
				Console.WriteLine ("System.Windows.DependencyObjectCollection.IsReadOnly: NIEX");
				throw new NotImplementedException ();
			}
		}
		
		public IEnumerator<T> GetEnumerator ()
		{
			Console.WriteLine ("System.Windows.DependencyObjectCollection.GetEnumerator: NIEX");
			throw new NotImplementedException ();
		}

		public event NotifyCollectionChangedEventHandler CollectionChanged;
	}
}
