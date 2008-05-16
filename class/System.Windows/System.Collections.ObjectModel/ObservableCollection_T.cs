//
// ObservableCollection.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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

	public class ObservableCollection<T> : Collection<T>, INotifyCollectionChanged
		where T : DependencyObject
		//  Temporary, need to add back: INotifyPropertyChanged
	{
		public ObservableCollection ()
		{
		}

		[MonoTODO("Currently does not raise any change events")]
		protected override void ClearItems ()
		{
			base.ClearItems ();
		}

		[MonoTODO("Currently does not raise any change events")]
		protected override void InsertItem (int index, T item)
		{
			base.InsertItem (index, item);
		}

		protected virtual void OnCollectionChanged (NotifyCollectionChangedEventArgs e)
		{
		}

		protected virtual void OnPropertyChanged (PropertyChangedEventArgs e)
		{
		}

		[MonoTODO("Currently does not raise any change events")]
		protected override void RemoveItem (int index)
		{
			base.RemoveItem (index);
		}

		[MonoTODO("Currently does not raise any change events")]
		protected override void SetItem (int index, T item)
		{
			base.SetItem (index, item);
		}

		public event NotifyCollectionChangedEventHandler CollectionChanged;

		[MonoTODO]
		protected event PropertyChangedEventHandler PropertyChanged {
			add {
				throw new NotImplementedException ();
			}

			remove {
				throw new NotImplementedException ();
			}
		}
	}
}
