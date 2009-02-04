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

using Mono;
using System.Collections;
using System.Windows.Markup;
using System.Windows.Media;
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
		private ItemsPresenter _presenter;

		public ItemsControl ()
		{
			DefaultStyleKey = typeof (ItemsControl);
		}

		internal override void InvokeLoaded ()
		{
			base.InvokeLoaded ();

			// XXX
			SetItemsPresenter (new ItemsPresenter ());
		}
		
		internal void SetItemsPresenter (ItemsPresenter presenter)
		{
			_presenter = presenter;
			NativeMethods.uielement_element_added (native, _presenter.native);
			NativeMethods.uielement_set_subtree_object (native, _presenter.native);

			AddItemsToPresenter (Items, 0);
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
			if (_presenter == null || _presenter._elementRoot == null)
				return;

			StackPanel panel = _presenter._elementRoot;

			switch (e.Action) {
			case NotifyCollectionChangedAction.Reset:
				// the list has gone away, so clear the children of the panel
				panel.Children.Clear ();
				break;
			case NotifyCollectionChangedAction.Add:
				AddItemsToPresenter (e.NewItems, e.NewStartingIndex);
				break;
			case NotifyCollectionChangedAction.Remove:
				RemoveItemsFromPresenter (e.OldItems, e.OldStartingIndex);
				break;
			case NotifyCollectionChangedAction.Replace:
				DependencyObject element = panel.Children[e.NewStartingIndex];
				break;
			}
		}

		void AddItemsToPresenter (IList newItems, int newIndex)
		{
			StackPanel panel = _presenter._elementRoot;
			for (int i = 0; i < newItems.Count; i ++) {
				object item = newItems[i];
				object element;

				Console.WriteLine ("adding item of type {0}", item.GetType());

				if (IsItemItsOwnContainerOverride (item)) {
					Console.WriteLine("item is its own container");
					element = item;
				}
				else {
					Console.WriteLine ("creating a container for it.");
					element = GetContainerForItemOverride ();
				}

				PrepareContainerForItemOverride (element as DependencyObject, item);

				if (element is UIElement) {
					Console.WriteLine ("inserting {0} at index {1}", element, newIndex + i);
					panel.Children.Insert (newIndex + i, (UIElement)element);
				}
			}
		}

		void RemoveItemsFromPresenter (IList oldItems, int oldIndex)
		{
			StackPanel panel = _presenter._elementRoot;
			for (int i = 0; i < oldItems.Count; i ++)
				panel.Children.RemoveAt (oldIndex);
		}

		protected virtual void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			ContentPresenter presenter = element as ContentPresenter;

			Console.WriteLine ("presenter = {0}, item = {1}", presenter, item);

			if (presenter != null && presenter != item) {
				Console.WriteLine (" + setting .Content");
				presenter.Content = item;
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

		public string DisplayMemberPath { 
			get { return (string) GetValue (DisplayMemberPathProperty); }
			set { SetValue (DisplayMemberPathProperty, value); }
		}

		public ItemsPanelTemplate ItemsPanel { 
			get { return (ItemsPanelTemplate) GetValue (ItemsPanelProperty); }
			set { SetValue (ItemsPanelProperty, value); }
		}

		public IEnumerable ItemsSource { 
			get { return (IEnumerable) GetValue (ItemsSourceProperty); }
			set { SetValue (ItemsSourceProperty, value); }
		}

		public DataTemplate ItemTemplate { 
			get { return (DataTemplate) GetValue (ItemTemplateProperty); }
			set { SetValue (ItemTemplateProperty, value); }
		}
	}
}
