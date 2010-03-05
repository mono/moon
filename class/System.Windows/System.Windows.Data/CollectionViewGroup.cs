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

namespace System.Windows.Data {
	public abstract class CollectionViewGroup : INotifyPropertyChanged {
		private object name;

		protected CollectionViewGroup (object name)
		{
			this.name = name;

			Console.WriteLine ("System.Windows.Data.CollectionViewGroup..ctor: NIEX");
			throw new NotImplementedException ();
		}

		protected virtual void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			Console.WriteLine ("System.Windows.Data.CollectionViewGroup.OnPropertyChanged: NIEX");
			throw new NotImplementedException ();
		}

		public abstract bool IsBottomLevel { get; }

		public int ItemCount {
			get {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.get_ItemCount: NIEX");
				throw new NotImplementedException ();
			}
		}

		public ReadOnlyObservableCollection<object> Items {
			get {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.get_Items: NIEX");
				throw new NotImplementedException ();
			}
		}

		public object Name {
			get {
				return name;
			}
		}

		protected int ProtectedItemCount {
			get {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.get_ProtectedItemCount: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.set_ProtectedItemCount: NIEX");
				throw new NotImplementedException ();
			}
		}

		protected ObservableCollection<object> ProtectedItems {
			get {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.get_ProtectedItems: NIEX");
				throw new NotImplementedException ();
			}
		}

		protected event PropertyChangedEventHandler PropertyChanged;
	
		event PropertyChangedEventHandler System.ComponentModel.INotifyPropertyChanged.PropertyChanged {
			add {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.add_PropertyChanged: NIEX");
				throw new NotImplementedException ();
			}
			remove {
				Console.WriteLine ("System.Windows.Data.CollectionViewGroup.remove_PropertyChanged: NIEX");
				throw new NotImplementedException ();
			}
		}
	}
}

