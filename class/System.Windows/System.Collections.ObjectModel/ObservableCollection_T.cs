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
using System.Collections.Specialized;
using System.ComponentModel;

namespace System.Collections.ObjectModel {

	public class ObservableCollection<T> : Collection<T>, INotifyCollectionChanged, INotifyPropertyChanged
	{
		public ObservableCollection ()
		{
		}

		protected override void ClearItems ()
		{
			base.ClearItems ();
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
		}

		protected override void InsertItem (int index, T item)
		{
			base.InsertItem (index, item);
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add,
										   item,
										   index));
		}

		protected virtual void OnCollectionChanged (NotifyCollectionChangedEventArgs e)
		{
			if (CollectionChanged != null)
				CollectionChanged (this, e);
		}

		protected virtual void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			if (PropertyChanged != null)
				PropertyChanged (this, e);
		}

		protected override void RemoveItem (int index)
		{
			T old_item = this[index];
			base.RemoveItem (index);
			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove,
										   old_item,
										   index));
		}

		protected override void SetItem (int index, T item)
		{
			T old_item = this[index];
			base.SetItem (index, item);

			OnCollectionChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace,
										   item,
										   old_item,
										   index));
		}

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event PropertyChangedEventHandler PropertyChanged;
	}
}
