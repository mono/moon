//
// ItemsControl.cs
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

using System.Collections;
using System.Windows.Markup;
using System.Collections.Specialized;

namespace System.Windows.Controls {

	[ContentPropertyAttribute("Items", true)]
	public class ItemsControl : Control {
		public static readonly DependencyProperty DisplayMemberPathProperty;
		public static readonly DependencyProperty ItemsPanelProperty;
		public static DependencyProperty ItemsSourceProperty;
		public static readonly DependencyProperty ItemTemplateProperty;

		public ItemsControl() {
			throw new NotImplementedException ();
		}

		protected virtual void ClearContainerForItemOverride(DependencyObject element, Object item) {
			throw new NotImplementedException ();
		}

		protected virtual DependencyObject GetContainerForItemOverride() {
			throw new NotImplementedException ();
		}

		protected virtual bool IsItemItsOwnContainerOverride(Object item) {
			throw new NotImplementedException ();
		}

		protected virtual void OnItemsChanged(NotifyCollectionChangedEventArgs e) {
			throw new NotImplementedException ();
		}

		protected virtual void PrepareContainerForItemOverride(DependencyObject element, Object item) {
			throw new NotImplementedException ();
		}

		public string DisplayMemberPath { 
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

		public ItemCollection Items {
			get {
				throw new NotImplementedException ();
			}
		}

		public ItemsPanelTemplate ItemsPanel { 
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

		public IEnumerable ItemsSource { 
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

		public DataTemplate ItemTemplate { 
			get {
				throw new NotImplementedException ();
			}
			set {
				throw new NotImplementedException ();
			}
		}

	}
}
