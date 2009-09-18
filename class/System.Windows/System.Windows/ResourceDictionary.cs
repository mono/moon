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

	public partial class ResourceDictionary	: DependencyObject, IDictionary<Object, Object> {

		public void Add (string key, object value)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			if (value == null)
				throw new NotSupportedException ("value");

			Value v = Value.FromObject (value, true);
			try {
				NativeMethods.resource_dictionary_add (native, key, ref v);
			} finally {
				NativeMethods.value_free_value (ref v);
			}
		}

		public void Clear ()
		{
			NativeMethods.resource_dictionary_clear (native);
		}

		private bool ContainsKey (string key)
		{
			return NativeMethods.resource_dictionary_contains_key (native, key);
		}

		public bool Contains (object key)
		{
			return ContainsKey (ToStringKey (key));
		}

		static string ToStringKey (object key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");

			var str_key = key as string;
			if (str_key == null)
				throw new ArgumentException ("Key must be a string");

			return str_key;
		}

		public void Remove (string key)
		{
			RemoveInternal (key);
		}

		private Uri source;
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

		private bool RemoveInternal (string key)
		{
			return NativeMethods.resource_dictionary_remove (native, key);
		}

		private bool TryGetValue (string key, out object value)
		{
			bool exists;

			IntPtr val = NativeMethods.resource_dictionary_get (native, key, out exists);

			value = null;

			if (exists)
				value = Value.ToObject (null, val);

			return exists;
		}

		public int Count {
			get { return NativeMethods.collection_get_count (native); }
		}

		public bool IsReadOnly {
			get { return false; }
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

		IEnumerator IEnumerable.GetEnumerator()
		{
			throw new NotImplementedException();
		}

		// IDictionary<object, object>  implementation
		//
		void IDictionary<object, object>.Add(object key, object value)
		{
			Add (ToStringKey (key), value);
		}

		bool IDictionary<object, object>.ContainsKey(object key)
		{
			return ContainsKey (ToStringKey (key));
		}

		object IDictionary<object, object>.this [object key] { 
			get { return this [key]; }
			set { this [key] = value; }
		}

		bool IDictionary<object, object>.Remove (object key)
		{
			return RemoveInternal (ToStringKey (key));
		}

		bool IDictionary<object, object>.TryGetValue (object key, out object value)
		{
			return TryGetValue (ToStringKey (key), out value);
		}

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

		void ICollection<KeyValuePair<object, object>>.CopyTo(KeyValuePair<object, object>[] array, int arrayIndex)
		{
			throw new NotImplementedException();
		}

		int ICollection<KeyValuePair<object, object>>.Count {
			get { throw new NotImplementedException (); }
		}

		bool ICollection<KeyValuePair<object, object>>.IsReadOnly {
			get { return IsReadOnly; }
		}

		bool ICollection<KeyValuePair<object, object>>.Remove (KeyValuePair<object, object> item)
		{
			Remove (ToStringKey (item.Key));
			return false;
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
