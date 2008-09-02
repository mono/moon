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
using System.Security;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Collections;
using System.Collections.Generic;
using Mono;

namespace System.Windows {

	public partial class ResourceDictionary	: DependencyObject,
		IDictionary<Object, Object>,
		ICollection<KeyValuePair<Object, Object>>,
		IEnumerable<KeyValuePair<Object, Object>>,
		IEnumerable {

		public void Add (string key, object value)
		{
			Value v = DependencyObject.GetAsValue (value, true);
			try {
				NativeMethods.resource_dictionary_add (native, key, ref v);
			} finally {
				NativeMethods.value_free_value (ref v);
			}
		}

		private bool Clear ()
		{
			return NativeMethods.resource_dictionary_clear (native);
		}

		private bool ContainsKey (string key)
		{
			return NativeMethods.resource_dictionary_contains_key (native, key);
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Remove (string key)
		{
			_Remove (key);
		}

		private bool _Remove (string key)
		{
			return NativeMethods.resource_dictionary_remove (native, key);
		}

		private bool TryGetValue (string key, out object value)
		{
			bool exists;

			IntPtr val = NativeMethods.resource_dictionary_get (native, key, out exists);

			value = null;

			if (exists)
				value = DependencyObject.ValueToObject (null, val);

			return exists;
		}

		public int Count { get { return 0; } }

		public bool IsReadOnly { get {throw new NotImplementedException();} }

		public object this[object key] { 
			get {
				if (!(key is string))
					throw new ArgumentException ("Key must be a string");

				bool exists;
				IntPtr val = NativeMethods.resource_dictionary_get (native, (string)key, out exists);
				if (val == IntPtr.Zero)
					return null;
				return DependencyObject.ValueToObject (null, val);
			}
			set {
				if (!(key is string))
					throw new ArgumentException ("Key must be a string");

				Value v = DependencyObject.GetAsValue (value, true);
				try {
					NativeMethods.resource_dictionary_set (native, (string)key, ref v);
				} finally {
					NativeMethods.value_free_value (ref v);
				}
			}
		}


		IEnumerator IEnumerable.GetEnumerator()
		{
			throw new NotImplementedException();
		}

		// IDictionary<object, object>  implementation
		//
		void IDictionary<object, object>.Add(object key, object value)
		{
			Add ((string)key, value);
		}

		bool IDictionary<object, object>.ContainsKey(object key)
		{
			return ContainsKey ((string)key);
		}

		object IDictionary<object, object>.this[ object key ] { 
			get { return this[key]; }
			set { this[key] = value; }
		}

		bool IDictionary<object, object>.Remove (object key)
		{
			return _Remove ((string)key);
		}

		bool IDictionary<object, object>.TryGetValue (object key, out object value)
		{
			return TryGetValue ((string)key, out value);
		}

		// ICollection<KeyValuePair<object, object>> implementation
		//
		void ICollection<KeyValuePair<object, object>>.Add(KeyValuePair<object, object> item)
		{
			Add ((string)item.Key, item.Value);
		}

		void ICollection<KeyValuePair<object, object>>.Clear()
		{
			Clear();
		}

		bool ICollection<KeyValuePair<object, object>>.Contains(KeyValuePair<object, object> item)
		{
			throw new NotImplementedException();
		}

		void ICollection<KeyValuePair<object, object>>.CopyTo(KeyValuePair<object, object>[] array, int arrayIndex)
		{
			throw new NotImplementedException();
		}

		int ICollection<KeyValuePair<object, object>>.Count {
			get { return Count; }
		}

		bool ICollection<KeyValuePair<object, object>>.IsReadOnly {
			get { return IsReadOnly; }
		}

		bool ICollection<KeyValuePair<object, object>>.Remove (KeyValuePair<object, object> item)
		{
			return _Remove ((string)item.Key);
		}

		// IDictionary<object, object> implementation
		//
		ICollection<object> IDictionary<object, object>.Keys {
			get {throw new NotImplementedException();}
		}

		ICollection<object> IDictionary<object, object>.Values {
			get {throw new NotImplementedException();}
		}

		// IEnumerator<KeyValuePair<object, objct>> implementation
		//
		IEnumerator<KeyValuePair<object, object>> IEnumerable<KeyValuePair<object, object>>.GetEnumerator()
		{
			throw new NotImplementedException();
		}
	}
}
