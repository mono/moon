//
// ResourceDictionary.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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

using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Windows.Interop;
using System.Windows.Resources;
using System.Reflection;
using System.Resources;
using Mono;
using Mono.Xaml;

namespace System.Windows {

	public partial class ResourceDictionary	: DependencyObject, IDictionary, IDictionary<object, object> {
		object sync_root = new object ();
		Uri source;
		
		//
		// Properties
		//
		
		public int Count {
			get { return NativeMethods.collection_get_count (native); }
		}
		
		public bool IsReadOnly {
			get { return false; }
		}
		
		public bool IsFixedSize {
			get { return false; }
		}
		
		public ICollection Keys {
			get {
				object[] keys = new object[Count];
				int i = 0;
				
				foreach (DictionaryEntry entry in this)
					keys[i++] = entry.Key;
				
				return Array.AsReadOnly<object> (keys);
			}
		}
		
		public ICollection Values {
			get {
				object[] values = new object[Count];
				int i = 0;
				
				foreach (DictionaryEntry entry in this)
					values[i++] = entry.Value;
				
				return Array.AsReadOnly<object> (values);
			}
		}
		
		public object this[object key] { 
			get {
				bool exists;
				IntPtr val = NativeMethods.resource_dictionary_get (native, ToStringKey (key), out exists);
				if (val == IntPtr.Zero)
					return null;
				return Value.ToObject (null, val);
			}
			set {
				var str_key = ToStringKey (key);
				
				Value v = Value.FromObject (value, true);
				try {
					NativeMethods.resource_dictionary_set (native, str_key, ref v);
				} finally {
					NativeMethods.value_free_value (ref v);
				}
			}
		}
		
		public Uri Source {
			get { return source; }
			set {
				if (source == value)
					return;
				
				Clear ();
				
				source = value;
				
				var stream = Application.GetResourceStream (value);
				
				using (StreamReader sr = new StreamReader (stream.Stream)) {
					string xaml = sr.ReadToEnd ();
						
					Value v = Value.FromObject (this);
					ManagedXamlLoader loader = new ManagedXamlLoader (Deployment.Current.EntryAssembly, value.ToString (), Deployment.Current.Surface.Native, PluginHost.Handle);
					loader.Hydrate (v, xaml, true, false, true);
				}
			}
		}
		
		
		//
		// Helper Methods
		//
		
		static string ToStringKey (object key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			
			var str_key = key as string;
			if (str_key == null)
				throw new ArgumentException ("Key must be a string");
			
			return str_key;
		}
		
		bool RemoveInternal (object key)
		{
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();
			
			if (key == null)
				return false;
			
			string str = key as string;
			if (str == null)
				throw new ArgumentException ("Key must be a string");
			
			return NativeMethods.resource_dictionary_remove (native, str);
		}
		
		
		//
		// Methods
		//
		
		public void Add (string key, object value)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			if (value == null)
				throw new NotSupportedException ("value");
			
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();
			
			Value v = Value.FromObject (value, true);
			try {
				NativeMethods.resource_dictionary_add (native, key, ref v);
			} finally {
				NativeMethods.value_free_value (ref v);
			}
		}
		
		public void Add (object key, object value)
		{
			Add (ToStringKey (key), value);
		}
		
		public void Clear ()
		{
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();
			
			NativeMethods.resource_dictionary_clear (native);
		}
		
		public bool Contains (object key)
		{
			return NativeMethods.resource_dictionary_contains_key (native, ToStringKey (key));
		}
		
		public void CopyTo (Array array, int index)
		{
			if (array == null)
				throw new ArgumentNullException ("array");
			
			if (index < 0)
				throw new ArgumentOutOfRangeException ("index");
			
			if (array.Rank > 0 || Count > array.Length - index)
				throw new ArgumentException ("array");
			
			foreach (DictionaryEntry entry in this)
				array.SetValue (entry, index++);
		}
		
		public IDictionaryEnumerator GetEnumerator ()
		{
			return new ResourceDictionaryIterator (NativeMethods.collection_get_iterator (native));
		}
		
		public void Remove (string key)
		{
			RemoveInternal (key);
		}
		
		public void Remove (object key)
		{
			RemoveInternal (key);
		}
		
		
		//
		// ICollection implementation
		//
		
		bool ICollection.IsSynchronized {
			get { return false; }
		}
		
		object ICollection.SyncRoot {
			get { return sync_root; }
		}
		
		IEnumerator IEnumerable.GetEnumerator ()
		{
			return GetEnumerator ();
		}
		
		
		//
		// ICollection<KeyValuePair<object, object>> implementation
		//
		
		void ICollection<KeyValuePair<object, object>>.Add (KeyValuePair<object, object> item)
		{
			Add (ToStringKey (item.Key), item.Value);
		}
		
		void ICollection<KeyValuePair<object, object>>.Clear ()
		{
			Clear ();
		}
		
		bool ICollection<KeyValuePair<object, object>>.Contains (KeyValuePair<object, object> item)
		{
			return Contains (item.Key);
		}
		
		void ICollection<KeyValuePair<object, object>>.CopyTo (KeyValuePair<object, object>[] array, int index)
		{
			// Note: Silverlight doesn't seem to implement this method. Lame.
			throw new NotImplementedException ();
			
			if (array == null)
				throw new ArgumentNullException ("array");
			
			if (index < 0)
				throw new ArgumentOutOfRangeException ("index");
			
			if (array.Rank > 0 || Count > array.Length - index)
				throw new ArgumentException ("array");
			
			foreach (KeyValuePair<object, object> pair in this)
				array[index++] = pair;
		}
		
		int ICollection<KeyValuePair<object, object>>.Count {
			get {
				// Note: This is always supposed to throw NotImplementedException according to MSDN.
				throw new NotImplementedException ();
				
				return Count;
			}
		}
		
		bool ICollection<KeyValuePair<object, object>>.IsReadOnly {
			get { return IsReadOnly; }
		}
		
		bool ICollection<KeyValuePair<object, object>>.Remove (KeyValuePair<object, object> item)
		{
			RemoveInternal (item.Key);
			return false;
		}
		
		
		//
		// IDictionary<object, object> implementation
		//
		
		void IDictionary<object, object>.Add(object key, object value)
		{
			Add (ToStringKey (key), value);
		}

		bool IDictionary<object, object>.ContainsKey(object key)
		{
			return Contains (key);
		}

		object IDictionary<object, object>.this [object key] { 
			get { return this [key]; }
			set { this [key] = value; }
		}

		bool IDictionary<object, object>.Remove (object key)
		{
			return RemoveInternal (key);
		}

		bool IDictionary<object, object>.TryGetValue (object key, out object value)
		{
			bool exists;

			IntPtr val = NativeMethods.resource_dictionary_get (native, ToStringKey (key), out exists);

			value = null;

			if (exists)
				value = Value.ToObject (null, val);

			return exists;
		}
		
		ICollection<object> IDictionary<object, object>.Keys {
			get {
				// Note: Silverlight doesn't seem to implement this property. Lame.
				throw new NotImplementedException ();
				
				return Keys as ICollection<object>;
			}
		}
		
		ICollection<object> IDictionary<object, object>.Values {
			get {
				// Note: Silverlight doesn't seem to implement this property. Lame.
				throw new NotImplementedException ();
				
				return Values as ICollection<object>;
			}
		}
		
		//
		// IEnumerator<KeyValuePair<object, object>> implementation
		//
		
		IEnumerator<KeyValuePair<object, object>> IEnumerable<KeyValuePair<object, object>>.GetEnumerator ()
		{
			// Note: Silverlight doesn't seem to implement this method. Lame.
			throw new NotImplementedException ();
			
			return new GenericResourceDictionaryIterator (NativeMethods.collection_get_iterator (native));
		}
		
		
		//
		// Enumerator implementations
		//
		
		internal sealed class ResourceDictionaryIterator : IDictionaryEnumerator, IDisposable {
			IntPtr native_iter;
			
			public ResourceDictionaryIterator (IntPtr native_iter)
			{
				this.native_iter = native_iter;
			}
			
			~ResourceDictionaryIterator ()
			{
				Dispose ();
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
				get { return (object) Entry; }
			}
			
			public DictionaryEntry Entry {
				get { return new DictionaryEntry (Key, Value); }
			}
			
			public object Key {
				get {
					return NativeMethods.resource_dictionary_iterator_get_current_key (native_iter);
				}
			}
			
			public object Value {
				get {
					IntPtr val = NativeMethods.collection_iterator_get_current (native_iter);
					if (val == IntPtr.Zero)
						return null;
					
					return Mono.Value.ToObject (null, val);
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
		}
		
		internal sealed class GenericResourceDictionaryIterator : IEnumerator<KeyValuePair<object, object>>, IDisposable {
			IntPtr native_iter;
			
			public GenericResourceDictionaryIterator (IntPtr native_iter)
			{
				this.native_iter = native_iter;
			}
			
			~GenericResourceDictionaryIterator ()
			{
				Dispose ();
			}
			
			object Key {
				get {
					return NativeMethods.resource_dictionary_iterator_get_current_key (native_iter);
				}
			}
			
			object Value {
				get {
					IntPtr val = NativeMethods.collection_iterator_get_current (native_iter);
					if (val == IntPtr.Zero)
						return null;
					
					return Mono.Value.ToObject (null, val);
				}
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
			
			KeyValuePair<object, object> GetCurrent ()
			{
				return new KeyValuePair<object, object> (Key, Value);
			}
			
			public KeyValuePair<object, object> Current {
				get {
					return GetCurrent ();
				}
			}
			
			object IEnumerator.Current {
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
		}
	}
}
