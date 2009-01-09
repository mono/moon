//
// ItemsControl.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008-2009 Novell, Inc (http://www.novell.com)
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

		public static readonly DependencyProperty DisplayMemberPathProperty =
			DependencyProperty.Register ("DisplayMemberPath", typeof (string), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemsPanelProperty =
			DependencyProperty.Register ("ItemsPanel", typeof (ItemsPanelTemplate), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemsSourceProperty =
			DependencyProperty.Register ("ItemsSource", typeof (IEnumerable), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemTemplateProperty =
			DependencyProperty.Register ("ItemTemplate", typeof (DataTemplate), typeof (ItemsControl), null);

		private ItemCollection items;


		public ItemsControl ()
		{
		}

		protected virtual void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			// nothing to undo by default (since nothing was prepared)
		}

		protected virtual DependencyObject GetContainerForItemOverride ()
		{
			return new ContentPresenter ();
		}

		protected virtual bool IsItemItsOwnContainerOverride (object item)
		{
			return (item is FrameworkElement);
		}

		protected virtual void OnItemsChanged (NotifyCollectionChangedEventArgs e)
		{
		}

		protected virtual void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			// nothing is prepared by default
		}

		public string DisplayMemberPath { 
			get {
				return (string) GetValue (DisplayMemberPathProperty);
			}
			set {
				SetValue (DisplayMemberPathProperty, value);
			}
		}

		public ItemCollection Items {
			get {
				if (items == null) {
					items = new ItemCollection ();
					items.ItemsChanged += delegate (object o, NotifyCollectionChangedEventArgs e) {
						OnItemsChanged (e);
					};
				}
				return items;
			}
		}

		public ItemsPanelTemplate ItemsPanel { 
			get {
				return (ItemsPanelTemplate) GetValue (ItemsPanelProperty);
			}
			set {
				SetValue (ItemsPanelProperty, value);
			}
		}

		public IEnumerable ItemsSource { 
			get {
				return (IEnumerable) GetValue (ItemsSourceProperty);
			}
			set {
				SetValue (ItemsSourceProperty, value);
			}
		}

		public DataTemplate ItemTemplate { 
			get {
				return (DataTemplate) GetValue (ItemTemplateProperty);
			}
			set {
				SetValue (ItemTemplateProperty, value);
			}
		}
	}
}
