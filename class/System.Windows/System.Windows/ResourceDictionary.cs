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
using Mono;

namespace System.Windows {

public class ResourceDictionary : DependencyObject, IDictionary<Object, Object>, ICollection<KeyValuePair<Object, Object>>, IEnumerable<KeyValuePair<Object, Object>>, IEnumerable {

		public ResourceDictionary ()  : base (NativeMethods.resource_dictionary_new ())
		{
		}
		
		internal ResourceDictionary (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.RESOURCE_DICTIONARY;
		}

		public void Add(string key, Object value) {
			throw new NotImplementedException();
		}

		public int Count { get {throw new NotImplementedException();} }

		public bool IsReadOnly { get {throw new NotImplementedException();} }

		public Object this[Object key] { 
			get {throw new NotImplementedException();}
			set {throw new NotImplementedException();}
		}

		void ICollection<KeyValuePair<Object, Object>>.Add(KeyValuePair<Object, Object> item) {
			throw new NotImplementedException();
		}

		void IDictionary<Object, Object>.Add(Object key, Object value) {
			throw new NotImplementedException();
		}

		void ICollection<KeyValuePair<Object, Object>>.Clear() {
			throw new NotImplementedException();
		}

		bool ICollection<KeyValuePair<Object, Object>>.Contains(KeyValuePair<Object, Object> item)  {
			throw new NotImplementedException();
		}

		bool IDictionary<Object, Object>.ContainsKey(Object key) {
			throw new NotImplementedException();
		}

		void ICollection<KeyValuePair<Object, Object>>.CopyTo(KeyValuePair<Object, Object>[] array, int arrayIndex) {
			throw new NotImplementedException();
		}

		int ICollection<KeyValuePair<Object, Object>>.Count { get {throw new NotImplementedException();} }

		IEnumerator<KeyValuePair<Object, Object>> IEnumerable<KeyValuePair<Object, Object>>.GetEnumerator() {
			throw new NotImplementedException();
		}

		IEnumerator IEnumerable.GetEnumerator() {
			throw new NotImplementedException();
		}

		bool ICollection<KeyValuePair<Object, Object>>.IsReadOnly { get {throw new NotImplementedException();} }

		Object IDictionary<Object, Object>.this[ Object key ] { 
			get {throw new NotImplementedException();}
			set {throw new NotImplementedException();}
		}

		ICollection<Object> IDictionary<Object, Object>.Keys { get {throw new NotImplementedException();} }

		bool ICollection<KeyValuePair<Object, Object>>.Remove( KeyValuePair<Object, Object> item) {
			throw new NotImplementedException();
		}

		bool IDictionary<Object, Object>.Remove( Object key) {
			throw new NotImplementedException();
		}

		bool IDictionary<Object, Object>.TryGetValue( Object key, out Object value) {
			throw new NotImplementedException();
		}

		ICollection<Object> IDictionary<Object, Object>.Values { get {throw new NotImplementedException();} }



	}
}
