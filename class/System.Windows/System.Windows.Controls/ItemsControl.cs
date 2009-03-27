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
using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Media;
using System.Collections.Specialized;

namespace System.Windows.Controls {

	[ContentPropertyAttribute("Items", true)]
	public class ItemsControl : Control {

		public static readonly DependencyProperty DisplayMemberPathProperty =
			DependencyProperty.Register ("DisplayMemberPath", typeof (string), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (DisplayMemberPathChanged)));
		public static readonly DependencyProperty ItemsPanelProperty =
			DependencyProperty.Register ("ItemsPanel", typeof (ItemsPanelTemplate), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemsSourceProperty =
			DependencyProperty.Register ("ItemsSource", typeof (IEnumerable), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (ItemsSourceChanged)));
		public static readonly DependencyProperty ItemTemplateProperty =
			DependencyProperty.Register ("ItemTemplate", typeof (DataTemplate), typeof (ItemsControl), null);

		private bool itemsIsDataBound;
		private ItemCollection items;
		private ItemsPresenter _presenter;

		public ItemsControl ()
		{
			DefaultStyleKey = typeof (ItemsControl);
		}

		internal override void InvokeLoaded ()
		{
			base.InvokeLoaded ();

			// effectively apply our default template
			// (which is nothing but an ItemsPresenter)
			// here.  but only do it if we don't have a
			// template in our style.
			if (Template == null)
				SetItemsPresenter (new ItemsPresenter ());
		}
		
		internal void SetItemsPresenter (ItemsPresenter presenter)
		{
			_presenter = presenter;
			NativeMethods.uielement_element_added (native, _presenter.native);
			NativeMethods.uielement_set_subtree_object (native, _presenter.native);

			AddItemsToPresenter (Items, 0);
		}

		static void ItemsSourceChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((ItemsControl) o).OnItemsSourceChanged (e.OldValue as IEnumerable,
								 e.NewValue as IEnumerable);
		}

		void OnItemsSourceChanged (IEnumerable oldSource, IEnumerable newSource)
		{
			if (oldSource is INotifyCollectionChanged) {
				((INotifyCollectionChanged)oldSource).CollectionChanged -= OnSourceCollectionChanged;
			}


			if (newSource != null) {

				if (newSource is INotifyCollectionChanged) {
					((INotifyCollectionChanged)newSource).CollectionChanged += OnSourceCollectionChanged;
				}

				itemsIsDataBound = true;

				items = new ItemCollection ();
				items.SetReadOnly();
				items.ItemsChanged += delegate (object o, NotifyCollectionChangedEventArgs e) {
					OnItemsChanged (e);
				};

				foreach (var v in newSource) {
					items.AddImpl (v);
				}
			}
		}

		void OnSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				for (int i = 0; i < e.NewItems.Count; i ++)
					items.InsertImpl (e.NewStartingIndex + i, e.NewItems[i]);
				break;
			case NotifyCollectionChangedAction.Remove:
				for (int i = 0; i < e.OldItems.Count; i ++)
					items.RemoveAtImpl (e.OldStartingIndex);
				break;
			case NotifyCollectionChangedAction.Replace:
				for (int i = 0; i < e.NewItems.Count; i++)
					items.SetItemImpl (e.NewStartingIndex+i, e.NewItems[i]);
				break;
			case NotifyCollectionChangedAction.Reset:
				items.ClearImpl ();
				break;
			}
		}
		
		static void DisplayMemberPathChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((ItemsControl) o).OnDisplayMemberPathChanged (e.OldValue as string,
								       e.NewValue as string);
		}

		void OnDisplayMemberPathChanged (string oldPath, string newPath)
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
			if (_presenter == null || _presenter._elementRoot == null)
				return;

			StackPanel panel = _presenter._elementRoot;
			for (int i = 0; i < newItems.Count; i ++) {
				object item = newItems[i];
				object element;

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
			if (_presenter == null || _presenter._elementRoot == null)
				return;

			StackPanel panel = _presenter._elementRoot;
			for (int i = 0; i < oldItems.Count; i ++)
				panel.Children.RemoveAt (oldIndex);
		}

		protected virtual void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			ContentPresenter presenter = element as ContentPresenter;

			if (presenter != null && presenter != item) {

				bool setContent = true;

				if (ItemTemplate != null) {
					presenter.ContentTemplate = ItemTemplate;
				}
				else if (!string.IsNullOrEmpty (DisplayMemberPath)) {
					Binding binding = new Binding (DisplayMemberPath);
					binding.Converter = new DisplayMemberValueConverter ();
					// XXX I'm thinking this next line shouldn't be necessary.  The CP should be setting its DataContext
					// property when DisplayMemberPath is in use, right?
					binding.Source = item;
					presenter.SetBinding (ContentPresenter.ContentProperty,
							      binding);
					setContent = false;
				}

				if (setContent)
					presenter.Content = item;
			}
		}

		public ItemCollection Items {
			get {
				if (items == null) {
					items = new ItemCollection ();
					itemsIsDataBound = false;
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
			set {
				if (!itemsIsDataBound && items != null && items.Count > 0) {
					throw new InvalidOperationException ("Items collection must be empty before using ItemsSource");
				}
				items = null;
				SetValue (ItemsSourceProperty, value);
			}
		}

		public DataTemplate ItemTemplate { 
			get { return (DataTemplate) GetValue (ItemTemplateProperty); }
			set { SetValue (ItemTemplateProperty, value); }
		}
	}
}
