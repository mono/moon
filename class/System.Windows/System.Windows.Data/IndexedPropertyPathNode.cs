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

namespace System.Windows.Data
{
	class IndexedPropertyPathNode : PropertyPathNode {

		public object Index {
			get; private set;
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
				var members = Source.GetType ().GetDefaultMembers ();
				foreach (PropertyInfo member in members) {
					var param = member.GetIndexParameters ();
					if (param.Length == 1) {
						if (Index is int && param[0].ParameterType == typeof (int)) {
							PropertyInfo = member;
							return;
						} else if (param [0].ParameterType == typeof (string)) {
							PropertyInfo = member;
						}
					}
				}
			}
		}

		void OnCollectionChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			UpdateValue ();
			if (Next != null)
				Next.SetSource (Value);
		}

		void OnPropertyChanged (object o, PropertyChangedEventArgs e)
		{
			UpdateValue ();
			if (Next != null)
				Next.SetSource (Value);
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			base.OnSourceChanged (oldSource, newSource);

			if (oldSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) oldSource).CollectionChanged -= OnCollectionChanged;
			if (newSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) newSource).CollectionChanged += OnCollectionChanged;

			if (oldSource is INotifyPropertyChanged)
				((INotifyPropertyChanged) oldSource).PropertyChanged -= OnPropertyChanged;
			if (newSource is INotifyPropertyChanged)
				((INotifyPropertyChanged) newSource).PropertyChanged += OnPropertyChanged;
			
			GetIndexer ();
		}

		public override void SetValue (object value)
		{
			if (PropertyInfo != null)
				PropertyInfo.SetValue (Source, value, new object[] { Index });
		}

		public override void UpdateValue ()
		{
			if (PropertyInfo == null) {
				ValueType = null;
				Value = null;
				return;
			}

			try {
				object newVal = PropertyInfo.GetValue (Source, new object [] { Index });
				if (Value != newVal) {
					ValueType = newVal == null ? null : newVal.GetType ();
					Value = newVal;
				}
			} catch {
				// Ignore
			}
		}
	}
}
