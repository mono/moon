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

// users of the desktop moonlight libraries needs to use the "standard" isolated storage from the .net framework
#if NET_2_1

using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Text;

namespace System.IO.IsolatedStorage {

	public sealed class IsolatedStorageSettings : IDictionary<string, object>, IDictionary,
		ICollection<KeyValuePair<string, object>>, ICollection,
		IEnumerable<KeyValuePair<string, object>>, IEnumerable {

		static private IsolatedStorageSettings application_settings;
		static private IsolatedStorageSettings site_settings;

		private IsolatedStorageFile container;
		private Dictionary<string, object> settings;

		// SL2 use a "well known" name and it's readable (and delete-able) directly by isolated storage
		private const string LocalSettings = "__LocalSettings";

		internal IsolatedStorageSettings (IsolatedStorageFile isf)
		{
			container = isf;

			if (!isf.FileExists (LocalSettings)) {
				settings = new Dictionary<string, object> ();
				return;
			}

			using (IsolatedStorageFileStream fs = isf.OpenFile (LocalSettings, FileMode.Open)) {
				using (StreamReader sr = new StreamReader (fs)) {
					// first line contains a fully qualified type + CRLF (System.Object)
					string header = sr.ReadLine ();
					if (header == null || header.StartsWith ("<"))
						fs.Position = 0;
					DataContractSerializer reader = new DataContractSerializer (typeof (Dictionary<string, object>));
					try {
						settings = (Dictionary<string, object>) reader.ReadObject (fs);
					} catch (Xml.XmlException) {
						settings = new Dictionary<string, object> ();
					}
				}
			}
		}

		~IsolatedStorageSettings ()
		{
			// settings are automatically saved if the application close normally
			Save ();
		}

		// static properties

		// per application, per-computer, per-user
		public static IsolatedStorageSettings ApplicationSettings {
			get {
				if (application_settings == null) {
					application_settings = new IsolatedStorageSettings (
						IsolatedStorageFile.GetUserStoreForApplication ());
				}
				return application_settings;
			}
		}

		// per domain, per-computer, per-user
		public static IsolatedStorageSettings SiteSettings {
			get {
				if (site_settings == null) {
					site_settings = new IsolatedStorageSettings (
						IsolatedStorageFile.GetUserStoreForSite ());
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

		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public void Clear ()
		{
			settings.Clear ();
		}

		public bool Contains (string key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			return settings.ContainsKey (key);
		}

		public bool Remove (string key)
		{
			return settings.Remove (key);
		}

		public void Save ()
		{
			using (IsolatedStorageFileStream fs = container.CreateFile (LocalSettings)) {
				// note: SL seems to prepend a line with a fully qualified name for System.Object + CRLF
				byte[] header = System.Text.Encoding.UTF8.GetBytes (typeof (object).AssemblyQualifiedName);
				fs.Write (header, 0, header.Length);
				fs.WriteByte (13);
				fs.WriteByte (10);
				// and does not seems to need it when reading back...
				DataContractSerializer ser = new DataContractSerializer (settings.GetType ());
				ser.WriteObject (fs, settings);
			}
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
			(settings as ICollection<KeyValuePair<string, object>>).CopyTo (array, arrayIndex);
		}

		bool ICollection<KeyValuePair<string, object>>.Remove (KeyValuePair<string, object> item)
		{
			return settings.Remove (item.Key);
		}


		ICollection<string> IDictionary<string, object>.Keys {
			get { return settings.Keys; }
		}

		ICollection<object> IDictionary<string, object>.Values {
			get { return settings.Values; }
		}

		bool IDictionary<string, object>.ContainsKey (string key)
		{
			return settings.ContainsKey (key);
		}

		bool IDictionary<string, object>.TryGetValue (string key, out object value)
		{
			return settings.TryGetValue (key, out value);
		}


		private string ExtractKey (object key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			return (key as string);
		}

		void IDictionary.Add (object key, object value)
		{
			string s = ExtractKey (key);
			if (s == null)
				throw new ArgumentException ("key");

			settings.Add (s, value);
		}

		void IDictionary.Clear ()
		{
			settings.Clear ();
		}

		bool IDictionary.Contains (object key)
		{
			string skey = ExtractKey (key);
			if (skey == null)
				return false;
			return settings.ContainsKey (skey);
		}

		object IDictionary.this [object key] {
			get {
				string s = ExtractKey (key);
				return (s == null) ? null : settings [s];
			}
			set {
				string s = ExtractKey (key);
				if (s == null)
					throw new ArgumentException ("key");
				settings [s] = value;
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
			string s = ExtractKey (key);
			if (s != null)
				settings.Remove (s);
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
	}
}

#endif
