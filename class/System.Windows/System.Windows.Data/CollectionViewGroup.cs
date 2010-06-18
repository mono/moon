//
// CollectionViewGroup.cs
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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace System.Windows.Data {
#if NET_2_1
	[TypeForwardedFrom ("System.Windows.Data, Version=2.0.5.0, Culture=Neutral, PublicKeyToken=31bf3856ad364e35")]
#endif
	public abstract class CollectionViewGroup : INotifyPropertyChanged {

		protected event PropertyChangedEventHandler PropertyChanged;

		event PropertyChangedEventHandler System.ComponentModel.INotifyPropertyChanged.PropertyChanged {
			add { PropertyChanged += value; }
			remove { PropertyChanged -= value; }
		}

		public abstract bool IsBottomLevel {
			get;
		}

		public int ItemCount {
			get; private set;
		}

		public object Name {
			get; private set;
		}

		protected int ProtectedItemCount {
			get {
				return ItemCount;
			}
			set {
				if (ItemCount != value) {
					ItemCount = value;
					OnPropertyChanged (new PropertyChangedEventArgs ("ItemCount"));
				}
			}
		}

		protected ObservableCollection<object> ProtectedItems {
			get; private set;
		}

		public ReadOnlyObservableCollection<object> Items {
			get; private set;
		}

		protected CollectionViewGroup (object name)
		{
			Name = name;
			ProtectedItems = new ObservableCollection<object> ();
			Items = new ReadOnlyObservableCollection<object> (ProtectedItems);
		}

		protected virtual void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			var h = PropertyChanged;
			if (h != null)
				h (this, e);
		}
	}
}

