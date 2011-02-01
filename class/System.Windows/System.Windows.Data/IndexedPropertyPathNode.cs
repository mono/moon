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
using System.ComponentModel;

using Mono;

namespace System.Windows.Data
{
	class IndexedPropertyPathNode : PropertyPathNode, IListenCollectionChanged {
		static readonly PropertyInfo IListIndexer = GetIndexer (true, typeof (IList));

		bool isBroken;
		public override bool IsBroken {
			get {
				return isBroken || base.IsBroken;
			}
		}

		public object Index {
			get; private set;
		}

		IWeakListener Listener {
			get; set;
		}

		public IndexedPropertyPathNode (string index)
		{
			int val;
			if (int.TryParse (index, out val))
				Index = val;
			else
				Index = index;
		}

		void GetIndexer ()
		{
			PropertyInfo = null;
			if (Source != null) {
				PropertyInfo = GetIndexer (Index is int, Source.GetType ());
				if (PropertyInfo == null && Source is IList)
					PropertyInfo = IListIndexer;
			}
		}

		static PropertyInfo GetIndexer (bool allowIntIndexer, Type type)
		{
			PropertyInfo propInfo = null;
			var members = type.GetDefaultMembers ();
			foreach (PropertyInfo member in members) {
				var param = member.GetIndexParameters ();
				if (param.Length == 1) {
					if (allowIntIndexer && param[0].ParameterType == typeof (int)) {
						propInfo = member;
						break;
					} else if (param [0].ParameterType == typeof (string)) {
						propInfo = member;
					}
				}
			}

			return propInfo;
		}

		void IListenCollectionChanged.CollectionChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			UpdateValue ();
			if (Next != null)
				Next.SetSource (Value);
		}

		protected override void OnSourcePropertyChanged (object o, PropertyChangedEventArgs e)
		{
			UpdateValue ();
			if (Next != null)
				Next.SetSource (Value);
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			base.OnSourceChanged (oldSource, newSource);

			if (Listener != null) {
				Listener.Detach ();
				Listener = null;
			}

			if (newSource is INotifyCollectionChanged)
				Listener = new WeakCollectionChangedListener ((INotifyCollectionChanged) newSource, this);

			GetIndexer ();
		}

		public override void SetValue (object value)
		{
			if (PropertyInfo != null)
				PropertyInfo.SetValue (Source, value, new object[] { Index });
		}

		public override void UpdateValue ()
		{
			isBroken = true;
			if (PropertyInfo == null) {
				ValueType = null;
				Value = null;
				return;
			}

			try {
				object newVal = PropertyInfo.GetValue (Source, new object [] { Index });
				isBroken = false;
				if (Value != newVal) {
					ValueType = PropertyInfo.PropertyType;
					Value = newVal;
				}
			} catch {
				ValueType = null;
				Value = null;
			}
		}
	}
}
