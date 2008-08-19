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

		static private IsolatedStorageSettings application_settings;
		static private IsolatedStorageSettings site_settings;

		private IsolatedStorageFile storage;
		private Dictionary<string, object> settings;

		internal IsolatedStorageSettings (string filename)
		{
			settings = new Dictionary<string, object> ();
		}

		~IsolatedStorageSettings ()
		{
			settings.Clear ();
		}

		// static properties

		[MonoTODO ("not loaded from file, isolated or not")]
		public static IsolatedStorageSettings ApplicationSettings {
			get {
				if (application_settings == null) {
					// FIXME: supply a constant, mangled, filename for the application
					application_settings = new IsolatedStorageSettings (null);
				}
				return application_settings;
			}
		}

		[MonoTODO ("not loaded from file, isolated or not")]
		public static IsolatedStorageSettings SiteSettings {
			get {
				if (site_settings == null) {
					// FIXME: supply a constant, mangled, filename for the site
					site_settings = new IsolatedStorageSettings (null);
				}
				return site_settings;
			}
		}

		// properties

		public int Count {
			get { return settings.Count; }
		}

		public ICollection Keys {
			get { return settings.Keys; }
		}

		public ICollection Values {
			get { return settings.Values; }
		}

		public object this [string key] {
			get {
				return settings [key];
			}
			set {
				settings [key] = value;
			}
		}

		// methods

		public void Add (string key, object value)
		{
			settings.Add (key, value);
		}

		public void Clear ()
		{
			settings.Clear ();
		}

		public bool Contains (string key)
		{
			return settings.ContainsKey (key);
		}

		public bool Remove (string key)
		{
			return settings.Remove (key);
		}

		[MonoTODO ("not saved to file, isolated or not")]
		public void Save ()
		{
			throw new NotImplementedException ();
		}

		public bool TryGetValue<T> (string key, out T value)
		{
			object v;
			if (!settings.TryGetValue (key, out v)) {
				value = default (T);
				return false;
			}
			value = (T) v;
			return true;
		}

		// explicit interface implementations

		int ICollection<KeyValuePair<string, object>>.Count {
			get { return settings.Count; }
		}

		bool ICollection<KeyValuePair<string, object>>.IsReadOnly {
			get { return false; }
		}

		ICollection<string> IDictionary<string, object>.Keys {
			get { return settings.Keys; }
		}

		ICollection<object> IDictionary<string, object>.Values {
			get { return settings.Values; }
		}

		void ICollection<KeyValuePair<string, object>>.Add (KeyValuePair<string, object> item)
		{
			settings.Add (item.Key, item.Value);
		}

		void ICollection<KeyValuePair<string, object>>.Clear ()
		{
			settings.Clear ();
		}

		bool ICollection<KeyValuePair<string, object>>.Contains (KeyValuePair<string, object> item)
		{
			return settings.ContainsKey (item.Key);
		}

		void ICollection<KeyValuePair<string, object>>.CopyTo (KeyValuePair<string, object> [] array, int arrayIndex)
		{
			throw new NotImplementedException ();
		}

		bool ICollection<KeyValuePair<string, object>>.Remove (KeyValuePair<string, object> item)
		{
			return settings.Remove (item.Key);
		}


		bool IDictionary<string, object>.ContainsKey (string key)
		{
			return settings.Remove (key);
		}

		bool IDictionary<string, object>.TryGetValue (string key, out object value)
		{
			return settings.TryGetValue (key, out value);
		}

		void IDictionary.Add (object key, object value)
		{
			settings.Add ((key as string), value);
		}

		void IDictionary.Clear ()
		{
			settings.Clear ();
		}

		bool IDictionary.Contains (object key)
		{
			return settings.ContainsKey (key as string);
		}


		IEnumerator<KeyValuePair<string, object>> IEnumerable<KeyValuePair<string, object>>.GetEnumerator ()
		{
			return settings.GetEnumerator ();
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			return settings.GetEnumerator ();
		}


		IDictionaryEnumerator IDictionary.GetEnumerator ()
		{
			return settings.GetEnumerator ();
		}

		object IDictionary.this [object key] {
			get {
				return settings [(key as string)];
			}
			set {
				settings [(key as string)] = value;
			}
		}

		bool IDictionary.IsFixedSize {
			get { return false; }
		}

		bool IDictionary.IsReadOnly {
			get { return false; }
		}

		void IDictionary.Remove (object key)
		{
			settings.Remove (key as string);
		}


		void ICollection.CopyTo (Array array, int index)
		{
			(settings as ICollection).CopyTo (array, index);
		}

		bool ICollection.IsSynchronized {
			get { return (settings as ICollection).IsSynchronized; }
		}

		object ICollection.SyncRoot {
			get { return (settings as ICollection).SyncRoot; }
		}
	}
}
