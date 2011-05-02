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

	internal class InternalResourceDictionaryChangedEventArgs : RoutedEventArgs {

		internal InternalResourceDictionaryChangedEventArgs (IntPtr raw, bool drop_ref) : base (raw, drop_ref)
		{
		}

		internal InternalResourceDictionaryChangedEventArgs (IntPtr raw) : this (raw, false)
		{
		}

		public CollectionChangedAction ChangedAction {
			get {
				return NativeMethods.resource_dictionary_changed_event_args_get_changed_action (NativeHandle);
			}
		}

		public object GetNewItem ()
		{
			return Value.ToObject (null, NativeMethods.resource_dictionary_changed_event_args_get_new_item (NativeHandle));
		}

		public string Key {
			get {
				return NativeMethods.resource_dictionary_changed_event_args_get_key (NativeHandle);
			}
		}
	}

	public partial class ResourceDictionary	: DependencyObject, IDictionary, IDictionary<object, object> {

		internal const string INTERNAL_TYPE_KEY_MAGIC_COOKIE = "___internal___moonlight___key___do___not__use___it___will___kill__cats__";

		PresentationFrameworkCollection<ResourceDictionary> mergedDictionaries;
		Uri source;

		Dictionary<object, object> managedDict;

		static UnmanagedEventHandlerInvoker resource_dictionary_changed = (_sender, _event_id, _token, _calldata, _closure) =>
			Events.SafeDispatcher (
			 (IntPtr target, IntPtr calldata, IntPtr closure) => {
				 var args = NativeDependencyObjectHelper.Lookup (calldata) as InternalResourceDictionaryChangedEventArgs;
				 if (args == null)
					 args = new InternalResourceDictionaryChangedEventArgs (calldata);
				 ((ResourceDictionary) NativeDependencyObjectHelper.FromIntPtr (closure)).InternalResourceDictionaryChanged (args);
			 }) (_sender, _calldata, _closure);

		void InternalResourceDictionaryChanged (InternalResourceDictionaryChangedEventArgs args)
		{
			if (managedDict == null)
				managedDict = new Dictionary<object, object> ();

			switch (args.ChangedAction) {
			case CollectionChangedAction.Add:
#if DEBUG_REF
				Console.WriteLine ("rd {0}/{1} adding ref to {2}/{3}", GetHashCode(), this, args.GetNewItem().GetHashCode(), args.GetNewItem());
#endif
				managedDict[args.Key] = args.GetNewItem ();
				break;
			case CollectionChangedAction.Remove:
#if DEBUG_REF
				Console.WriteLine ("rd {0}/{1} removing ref to {2}/{3}", GetHashCode(), this, managedDict[args.Key].GetHashCode(), managedDict[args.Key]);
#endif
				managedDict.Remove (args.Key);
				break;
			case CollectionChangedAction.Replace:
#if DEBUG_REF
				Console.WriteLine ("rd {0}/{1} replacing ref from {2}/{3} to {4}/{5}", GetHashCode(), this, managedDict[args.Key].GetHashCode(), managedDict[args.Key], args.GetNewItem(), args.GetNewItem().GetHashCode());
#endif
				managedDict[args.Key] = args.GetNewItem ();
				break;
			case CollectionChangedAction.Clearing:
				// nothing to do
				break;
			case CollectionChangedAction.Cleared:
#if DEBUG_REF
				foreach (var k in managedDict.Keys)
					Console.WriteLine (" rd {0}/{1} removing ref to {2} {3}/{4}", GetHashCode(), this, k, managedDict[k].GetHashCode(), managedDict[k]);
#endif
				managedDict.Clear();
				break;
			}
		}

		private new void Initialize ()
		{
			int c = Count;
			for (int i = 0; i < c; i ++) {
				// FIXME
			}

			// set up a handler to track changes to the unmanaged dictionary
			Events.AddHandler (this, EventIds.ResourceDictionary_ChangedEvent, resource_dictionary_changed);
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
				if (managedDict == null)
					managedDict = new Dictionary<object, object> ();
				return managedDict.Keys;
			}
		}

		public ICollection Values {
			get {
				if (managedDict == null)
					managedDict = new Dictionary<object, object> ();
				return managedDict.Values;
			}
		}
		
		public object this[object key] { 
			get {
				if (!(key is string || key is Type))
					throw new ArgumentException ("Key must be a string or Type");

				// check our cache
				if (managedDict != null && managedDict.ContainsKey (key))
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
					if (NativeMethods.resource_dictionary_set (native, str_key, ref v)) {
						if (managedDict == null)
							managedDict = new Dictionary<object, object> ();
						managedDict.Add (str_key, value);
					}
				}
			}
		}
		internal override void AddStrongRef (IntPtr id, object value)
		{
			if (id == ResourceDictionary.MergedDictionariesProperty.Native)
				mergedDictionaries = (PresentationFrameworkCollection<ResourceDictionary>) value;
			else
				base.AddStrongRef (id, value);
		}

		internal override void ClearStrongRef (IntPtr id, object value)
		{
			if (id == ResourceDictionary.MergedDictionariesProperty.Native)
				mergedDictionaries = null;
			else
				base.ClearStrongRef (id, value);
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

				if (Deployment.Current.ParseUriStack.Contains (source))
					throw new InvalidOperationException ();

				NativeMethods.resource_dictionary_set_internal_source (native, source.ToString());

				var stream = Application.GetResourceStream (source);
				if (stream == null) {
					Console.Error.WriteLine ("Unable to find resource stream: '{0}'", source);
					Console.Error.WriteLine ("Unmerged source value: '{0}'", value);
					throw new Exception ("Could not find the resource at the given uri");
				}

				try {
					Deployment.Current.ParseUriStack.Push (source);

					XamlLoader loader = XamlLoaderFactory.CreateLoader (Deployment.Current.EntryAssembly, source, Deployment.Current.Surface.Native, PluginHost.Handle);
					loader.Hydrate (this, stream.Stream, true, false, true);

				} catch (Exception e) {
					Console.Error.WriteLine ("Error while parsing xaml referred to in ResourceDictionary::Source property '{0}'.", source);
					Console.Error.WriteLine (e);

					throw e;
				}
				finally {
					Deployment.Current.ParseUriStack.Pop ();
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

		static string TypeToString (Type type)
		{
			return INTERNAL_TYPE_KEY_MAGIC_COOKIE + type.FullName;
		}
		
		bool RemoveInternal (object key)
		{
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();
			
			if (key == null)
				throw new ArgumentNullException ("key");
			
			if (key is string) {
				if (((string)key).StartsWith (INTERNAL_TYPE_KEY_MAGIC_COOKIE))
					return false;
				if (managedDict != null)
					managedDict.Remove (key);
				return NativeMethods.resource_dictionary_remove (native, (string)key);
			}
			else if (key is Type) {
				if (managedDict != null)
					managedDict.Remove (key);
				return NativeMethods.resource_dictionary_remove (native, TypeToString ((Type) key));
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
				if (NativeMethods.resource_dictionary_add (native, key, ref v)) {
					if (managedDict == null)
						managedDict = new Dictionary<object, object> ();
					managedDict[key] = value;
				}
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

			if (managedDict != null && (managedDict.ContainsKey (key) || (key is Type && managedDict.ContainsKey (key.ToString ()))))
				throw new ArgumentException ("An item with the same key has already been added");

			using (var val = Value.FromObject (value, true)) {
				var v = val;

				// we have to add this first because in the case of implicit styles the resource_dictionary_add can make us re-enter.
				if (managedDict == null)
					managedDict = new Dictionary<object, object> ();
				managedDict[key] = value;

				if (!NativeMethods.resource_dictionary_add (native,
									    TypeToString ((Type) key),
									    ref v))
					managedDict.Remove (key);
			}
		}
		
		public void Clear ()
		{
			if (IsReadOnly || IsFixedSize)
				throw new NotSupportedException ();

			if (managedDict != null)
					managedDict.Clear ();
			NativeMethods.resource_dictionary_clear (native);
		}
		
		public bool Contains (object key)
		{
			if (key == null)
				throw new ArgumentNullException ("key");

			if (!(key is string || key is Type))
				throw new ArgumentException ("Key must be a string or Type");

			if (managedDict != null && managedDict.ContainsKey (key))
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
			if (managedDict == null)
				managedDict = new Dictionary<object, object> ();
			return ((IDictionary)managedDict).GetEnumerator ();
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
			get { return this; }
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
			if (managedDict == null)
				managedDict = new Dictionary<object, object> ();
			return managedDict.GetEnumerator();
		}
	}
}
