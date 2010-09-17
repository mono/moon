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
		Uri source;

		Dictionary<object, object> managedDict;

		private new void Initialize ()
		{
			managedDict = new Dictionary<object,object>();
			// FIXME we need to populate the managed dictionary from values that might exist in unmanaged
		}

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
				return managedDict.Keys;
			}
		}

		public ICollection Values {
			get {
				return managedDict.Values;
			}
		}
		
		public object this[object key] { 
			get {
				if (!(key is string || key is Type))
					throw new ArgumentException ("Key must be a string or Type");

				// check our cache
				if (managedDict.ContainsKey (key))
					return managedDict[key];

				// now iterate over the merged dictionaries
				PresentationFrameworkCollection<ResourceDictionary> col = MergedDictionaries;
				for (int i = col.Count - 1; i >= 0; i --) {
					ResourceDictionary rd = col[i];

					if (rd.Contains (key))
						return rd[key];
				}

				return null;
			}
			set {
				// this setter only permits string
				// keys.  since unmanaged needs to
				// access them we send it along to
				// unmanaged as well as update our
				// cache.
				var str_key = ToStringKey (key);
				
				using (var val = Value.FromObject (value, true)) {
					var v = val;
					if (NativeMethods.resource_dictionary_set (native, str_key, ref v))
						managedDict.Add (str_key, value);
				}
			}
		}
		
		public Uri Source {
			get { return (source = source ?? new Uri ("", UriKind.Relative)); }
			set {
				if (source == value)
					return;
				
				Clear ();
				
				source = value;
				
				if (source == null)
					return;

				if (!Application.IsAbsoluteResourceStreamLocator (source))
					source = Application.MergeResourceStreamLocators (ResourceBase, source);

				var stream = Application.GetResourceStream (source);
				if (stream == null) {
					Console.Error.WriteLine ("Unable to find resource stream: '{0}'", source);
					Console.Error.WriteLine ("Unmerged source value: '{0}'", value);
					throw new Exception ("Could not find the resource at the given uri");
				}

				try {
					XamlLoader loader = XamlLoaderFactory.CreateLoader (Deployment.Current.EntryAssembly, source, Deployment.Current.Surface.Native, PluginHost.Handle);
					loader.Hydrate (this, stream.Stream, true, false, true);
				} catch (Exception e) {
					Console.Error.WriteLine ("Error while parsing xaml referred to in ResourceDictionary::Source property '{0}'.", source);
					Console.Error.WriteLine (e);

					throw e;
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
			
			if (key is string) {
				bool rv = NativeMethods.resource_dictionary_remove (native, (string)key);
				managedDict.Remove (key);
				return rv;
			}
			else if (key is Type) {
				managedDict.Remove (key);
				return true;
			}
			else
				throw new ArgumentException ("Key must be a string");
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
			
			using (var val = Value.FromObject (value, true)) {
				var v = val;
				if (NativeMethods.resource_dictionary_add (native, key, ref v))
					managedDict[key] = value;
			}
		}
		
		public void Add (object key, object value)
		{
			if (key == null)
				throw new ArgumentNullException ("key");
			if (value == null)
				throw new NotSupportedException ("value");

			if (key is string) {
				Add (ToStringKey (key), value);
				return;
			}
			else if (!(key is Type)) {
				throw new ArgumentException ("Key must be a string or Type");
			}

			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();

			Style s = value as Style;
			if (s == null || s.TargetType != key)
				throw new ArgumentException ("Type as key can only be used with Styles whose target type is the same as the key");

			// we only add it to the managed dictionary,
			// since unmanaged doesn't know about type
			// keys at all.
			managedDict[key] = value;
		}
		
		public void Clear ()
		{
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();
			
			managedDict.Clear ();
			NativeMethods.resource_dictionary_clear (native);
		}
		
		public bool Contains (object key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");

			if (!(key is string || key is Type))
				throw new ArgumentException ("Key must be a string or Type");

			if (managedDict.ContainsKey (key))
				return true;
			PresentationFrameworkCollection<ResourceDictionary> col = MergedDictionaries;
			for (int i = col.Count - 1; i >= 0; i --) {
				ResourceDictionary rd = col[i];

				if (rd.Contains (key))
					return true;
			}
			return false;
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
			return managedDict.GetEnumerator ();
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
			get { return managedDict; }
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
		}
		
		int ICollection<KeyValuePair<object, object>>.Count {
			get {
				// Note: This is always supposed to throw NotImplementedException according to MSDN.
				throw new NotImplementedException ();
				
				//return Count;
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
			// FIXME does this Type-as-key-ifying?
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
				
				//return Keys as ICollection<object>;
			}
		}
		
		ICollection<object> IDictionary<object, object>.Values {
			get {
				// Note: Silverlight doesn't seem to implement this property. Lame.
				throw new NotImplementedException ();
				
				//return Values as ICollection<object>;
			}
		}
		
		//
		// IEnumerator<KeyValuePair<object, object>> implementation
		//
		
		IEnumerator<KeyValuePair<object, object>> IEnumerable<KeyValuePair<object, object>>.GetEnumerator ()
		{
			// Note: Silverlight doesn't seem to implement this method. Lame.
			throw new NotImplementedException ();
		}
	}
}
