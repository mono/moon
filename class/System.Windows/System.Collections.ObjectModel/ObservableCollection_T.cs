//
// ObservableCollection.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;

namespace System.Collections.ObjectModel {

	public class ObservableCollection<T> : Collection<T>, INotifyCollectionChanged, INotifyPropertyChanged
	{
		public event NotifyCollectionChangedEventHandler CollectionChanged;
		protected event PropertyChangedEventHandler PropertyChanged;

		bool in_handler;

		event PropertyChangedEventHandler INotifyPropertyChanged.PropertyChanged {
			add { PropertyChanged += value; }
			remove { PropertyChanged -= value; }
		}

		public ObservableCollection ()
		{
		}

		public ObservableCollection (IEnumerable<T> collection)
		{
			foreach (var v in collection)
				base.InsertItem (Count, v);
		}
		
		public ObservableCollection (List<T> list)
		{
			foreach (var v in list)
				base.InsertItem (Count, v);
		}

		protected override void ClearItems ()
		{
			if (in_handler)
				throw new InvalidOperationException ("You cannot modify an ObservableCollection in a change handler");

			in_handler = true;

			base.ClearItems ();

			RaiseCountChanged ();
			RaiseItemsChanged ();
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));

			in_handler = false;
		}

		protected override void InsertItem (int index, T item)
		{
			if (in_handler)
				throw new InvalidOperationException ("You cannot modify an ObservableCollection in a change handler");

			in_handler = true;

			base.InsertItem (index, item);
			RaiseCountChanged ();
			RaiseItemsChanged ();
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add,
										   item,
										   index));

			in_handler = false;
		}

		protected virtual void OnCollectionChanged (NotifyCollectionChangedEventArgs e)
		{
			CollectionChanged.Raise (this, e);
		}

		protected virtual void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			if (PropertyChanged != null)
				PropertyChanged (this, e);
		}

		protected override void RemoveItem (int index)
		{
			if (in_handler)
				throw new InvalidOperationException ("You cannot modify an ObservableCollection in a change handler");

			in_handler = true;

			T old_item = this[index];
			base.RemoveItem (index);
			RaiseCountChanged ();
			RaiseItemsChanged ();
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove,
										   old_item,
										   index));

			in_handler = false;
		}

		protected override void SetItem (int index, T item)
		{
			if (in_handler)
				throw new InvalidOperationException ("You cannot modify an ObservableCollection in a change handler");

			in_handler = true;

			T old_item = this[index];
			base.SetItem (index, item);

			RaiseItemsChanged ();
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace,
										   item,
										   old_item,
										   index));

			in_handler = false;
		}

		void RaiseCountChanged ()
		{
			RaisePropertyChanged ("Count");
		}

		void RaiseItemsChanged ()
		{
			RaisePropertyChanged ("Item[]");
		}

		void RaisePropertyChanged (string property)
		{
			var h = PropertyChanged;
			if (h != null)
				h (this, new PropertyChangedEventArgs (property));
		}
	}
}
