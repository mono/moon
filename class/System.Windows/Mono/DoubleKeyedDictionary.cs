//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Collections.Generic;

namespace System.Windows
{
	class DoubleKeyedDictionary <K1, K2> : IEnumerable <KeyValuePair<K1, K2>>
	{
		Dictionary <K1, K2> forwards;
		Dictionary <K2, K1> backwards;
		
		public DoubleKeyedDictionary ()
		{
			forwards = new Dictionary <K1, K2> ();
			backwards = new Dictionary <K2, K1> ();
		}

		public K1 this[K2 key] {
			get { return backwards [key]; }
		}

		public K2 this [K1 key] {
			get { return forwards [key]; }
		}

		public void Add (K1 key1, K2 key2)
		{
			Add (key1, key2, false);
		}

		public void Add (K1 key1, K2 key2, bool ignoreExisting)
		{
			if (!ignoreExisting && (forwards.ContainsKey (key1) || backwards.ContainsKey (key2)))
				throw new InvalidOperationException ("Dictionary already contains this key pair");
			forwards [key1] = key2;
			backwards [key2] = key1;
		}

		public void Clear ()
		{
			forwards.Clear ();
			backwards.Clear ();
		}

		public void Remove (K1 key1, K2 key2)
		{
			if (!forwards.ContainsKey (key1) || !backwards.ContainsKey (key2))
				throw new InvalidOperationException ("Dictionary does not contain this key pair");
			forwards.Remove (key1);
			backwards.Remove (key2);
		}

		public void Remove (K1 key1, K2 key2, bool ignoreExisting)
		{
			if (!ignoreExisting && (!forwards.ContainsKey (key1) || !backwards.ContainsKey (key2)))
				throw new InvalidOperationException ("Dictionary does not contain this key pair");
			forwards.Remove (key1);
			backwards.Remove (key2);
		}

		public bool TryMap (K1 key, out K2 value)
		{
			return forwards.TryGetValue (key, out value);
		}

		public bool TryMap (K2 key, out K1 value)
		{
			return backwards.TryGetValue (key, out value);
		}

		public IEnumerator<KeyValuePair<K1, K2>> GetEnumerator ()
		{
			return forwards.GetEnumerator ();
		}

		System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator ()
		{
			return GetEnumerator ();
		}
	}
}
