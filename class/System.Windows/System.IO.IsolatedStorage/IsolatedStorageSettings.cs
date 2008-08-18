//
// System.IO.IsolatedStorage.IsolatedStorageSettings
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

namespace System.IO.IsolatedStorage {

	public sealed class IsolatedStorageSettings : IDictionary<string, object>, IDictionary,
		ICollection<KeyValuePair<string, object>>, ICollection,
		IEnumerable<KeyValuePair<string, object>>, IEnumerable {

		internal IsolatedStorageSettings ()
		{
		}

		~IsolatedStorageSettings ()
		{
		}

		// static properties

		public static IsolatedStorageSettings ApplicationSettings {
			get { throw new NotImplementedException (); }
		}

		public static IsolatedStorageSettings SiteSettings {
			get { throw new NotImplementedException (); }
		}

		// properties

		public int Count {
			get { throw new NotImplementedException (); }
		}

		public ICollection Keys {
			get { throw new NotImplementedException (); }
		}

		public ICollection Values {
			get { throw new NotImplementedException (); }
		}

		public object this [string key] {
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

		// methods

		public void Add (string key, object value)
		{
			throw new NotImplementedException ();
		}

		public void Clear ()
		{
			throw new NotImplementedException ();
		}

		public bool Contains (string key)
		{
			throw new NotImplementedException ();
		}

		public bool Remove (string key)
		{
			throw new NotImplementedException ();
		}

		public void Save ()
		{
			throw new NotImplementedException ();
		}

		public bool TryGetValue<T> (string key, out T value)
		{
			throw new NotImplementedException ();
		}

		// explicit interface implementations

		int ICollection<KeyValuePair<string, object>>.Count {
			get { throw new NotImplementedException (); }
		}

		bool ICollection<KeyValuePair<string, object>>.IsReadOnly {
			get { throw new NotImplementedException (); }
		}

		ICollection<string> IDictionary<string, object>.Keys {
			get { throw new NotImplementedException (); }
		}

		ICollection<object> IDictionary<string, object>.Values {
			get { throw new NotImplementedException (); }
		}

		void ICollection<KeyValuePair<string, object>>.Add (KeyValuePair<string, object> item)
		{
			throw new NotImplementedException ();
		}

		void ICollection<KeyValuePair<string, object>>.Clear ()
		{
			throw new NotImplementedException ();
		}

		bool ICollection<KeyValuePair<string, object>>.Contains (KeyValuePair<string, object> item)
		{
			throw new NotImplementedException ();
		}

		void ICollection<KeyValuePair<string, object>>.CopyTo (KeyValuePair<string, object> [] array, int arrayIndex)
		{
			throw new NotImplementedException ();
		}

		bool ICollection<KeyValuePair<string, object>>.Remove (KeyValuePair<string, object> item)
		{
			throw new NotImplementedException ();
		}


		bool IDictionary<string, object>.ContainsKey (string key)
		{
			throw new NotImplementedException ();
		}

		bool IDictionary<string, object>.TryGetValue (string key, out object value)
		{
			throw new NotImplementedException ();
		}

		void IDictionary.Add (object key, object value)
		{
			throw new NotImplementedException ();
		}

		void IDictionary.Clear ()
		{
			throw new NotImplementedException ();
		}

		bool IDictionary.Contains (object key)
		{
			throw new NotImplementedException ();
		}


		IEnumerator<KeyValuePair<string, object>> IEnumerable<KeyValuePair<string, object>>.GetEnumerator ()
		{
			throw new NotImplementedException ();
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			throw new NotImplementedException ();
		}


		IDictionaryEnumerator IDictionary.GetEnumerator ()
		{
			throw new NotImplementedException ();
		}

		object IDictionary.this [object key] {
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

		bool IDictionary.IsFixedSize {
			get { throw new NotImplementedException (); }
		}

		bool IDictionary.IsReadOnly {
			get { throw new NotImplementedException (); }
		}

		void IDictionary.Remove (object key)
		{
			throw new NotImplementedException ();
		}


		void ICollection.CopyTo (Array array, int index)
		{
			throw new NotImplementedException ();
		}

		bool ICollection.IsSynchronized {
			get { throw new NotImplementedException (); }
		}

		object ICollection.SyncRoot {
			get { throw new NotImplementedException (); }
		}
	}
}
