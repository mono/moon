//
// IndexedPropertyPathNode.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Collections.Specialized;
using System.Reflection;

namespace System.Windows.Data
{
	class IndexedPropertyPathNode : PropertyPathNode {

		public int Index {
			get; private set;
		}

		public IndexedPropertyPathNode (int index)
		{
			Index = index;
		}

		bool GetIndexer ()
		{
			var members = Source.GetType ().GetDefaultMembers ();
			if (members.Length == 1  && members [0] is PropertyInfo) {
				PropertyInfo = (PropertyInfo) members [0];
				return true;
			}
			return false;
		}

		void OnCollectionChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			IList source = (IList) Source;
			if (Index >= source.Count)
				return;

			object newVal = PropertyInfo.GetValue (source, new object [] { Index });
			if (Value != newVal) {
				Value = newVal;
				if (Next != null)
					Next.SetSource (Value);
			}
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			base.OnSourceChanged (oldSource, newSource);

			if (oldSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) oldSource).CollectionChanged -= OnCollectionChanged;
			if (newSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) newSource).CollectionChanged += OnCollectionChanged;

			IList source = Source as IList;

			if (source == null || !GetIndexer () || Index >= source.Count) {
				PropertyInfo = null;
				ValueType = null;
				Value = null;
			} else {
				ValueType = PropertyInfo.PropertyType;
				Value = PropertyInfo.GetValue (source, new object [] { Index });
			}
		}

		public override void SetValue (object value)
		{
			if (PropertyInfo != null)
				((IList) Source)[Index] = value;
		}
	}
}
