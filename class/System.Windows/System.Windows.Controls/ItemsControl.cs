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
using System.Collections.Generic;

namespace System.Windows.Controls {

	[ContentPropertyAttribute("Items", true)]
	public class ItemsControl : Control {
		
		public static readonly DependencyProperty DisplayMemberPathProperty =
			DependencyProperty.RegisterCore ("DisplayMemberPath", typeof (string), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (DisplayMemberPathChanged)));
		public static readonly DependencyProperty ItemsPanelProperty =
			DependencyProperty.RegisterCore ("ItemsPanel", typeof (ItemsPanelTemplate), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemsSourceProperty =
			DependencyProperty.RegisterCore ("ItemsSource", typeof (IEnumerable), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (ItemsSourceChanged)));
		public static readonly DependencyProperty ItemTemplateProperty =
			DependencyProperty.RegisterCore ("ItemTemplate", typeof (DataTemplate), typeof (ItemsControl), null);

		DataTemplate displayMemberTemplate;
		private bool itemsIsDataBound;
		private ItemCollection items;
		private ItemsPresenter _presenter;
		Dictionary <DependencyObject, object> ContainerToItems {
			get; set;
		}
		
		DataTemplate DisplayMemberTemplate {
			get {
				if (displayMemberTemplate == null) {
					displayMemberTemplate = (DataTemplate) XamlReader.Load (@"
<DataTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Grid>
		<TextBlock Text=""{Binding " + DisplayMemberPath + @"}"" />
	</Grid>
</DataTemplate>");
				}
				return displayMemberTemplate;
			}
		}

		public ItemsControl ()
		{
			ContainerToItems = new Dictionary<DependencyObject, object> ();
			DefaultStyleKey = typeof (ItemsControl);
			ItemContainerGenerator = new ItemContainerGenerator (this);
		}

		internal override UIElement GetDefaultTemplate ()
		{
			if (_presenter == null) {
				_presenter = new ItemsPresenter ();
				_presenter.TemplateOwner = this;
			}
			return _presenter;
		}

		internal void SetItemsPresenter (ItemsPresenter presenter)
		{
			_presenter = presenter;
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
				Items.ClearImpl ();
				Items.SetIsReadOnly (true);
				
				foreach (var v in newSource)
					Items.AddImpl (v);
				
				// Setting itemsIsDataBound to true prevents normal notifications from propagating, so do it manually here
				OnItemsChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
			} else {
				itemsIsDataBound = false;
				Items.SetIsReadOnly (false);		
				Items.ClearImpl ();
			}
		}

		void OnSourceCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				for (int i = 0; i < e.NewItems.Count; i ++)
					Items.InsertImpl (e.NewStartingIndex + i, e.NewItems[i]);
				break;
			case NotifyCollectionChangedAction.Remove:
				for (int i = 0; i < e.OldItems.Count; i ++)
					Items.RemoveAtImpl (e.OldStartingIndex);
				break;
			case NotifyCollectionChangedAction.Replace:
				for (int i = 0; i < e.NewItems.Count; i++)
					Items.SetItemImpl (e.NewStartingIndex+i, e.NewItems[i]);
				break;
			case NotifyCollectionChangedAction.Reset:
				Items.ClearImpl ();
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
			displayMemberTemplate = null;
		}

		protected virtual void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			// nothing to undo by default (since nothing was prepared)
		}

		protected virtual DependencyObject GetContainerForItemOverride ()
		{
			return new ContentPresenter ();
		}

		internal ListBoxItem GetContainerItem (int index)
		{
			if (_presenter == null || _presenter._elementRoot == null)
				return null;

			// If this method is called in response to the collection being cleared,
			// we can get an index of -1 when using GetContainerItem (Items.IndexOf (o))
			if (index < 0 || index >= _presenter._elementRoot.Children.Count)
				return null;

			ListBoxItem item = _presenter._elementRoot.Children [index] as ListBoxItem;
			return item;
		}
		
		protected virtual bool IsItemItsOwnContainerOverride (object item)
		{
			return (item is FrameworkElement);
		}

		protected virtual void OnItemsChanged (NotifyCollectionChangedEventArgs e)
		{
			
		}
		
		void InvokeItemsChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				SetLogicalParent (native, e.NewItems);
				break;
			case NotifyCollectionChangedAction.Remove:
				SetLogicalParent (IntPtr.Zero, e.OldItems);
				break;
			case NotifyCollectionChangedAction.Replace:
				SetLogicalParent (IntPtr.Zero, e.OldItems);
				SetLogicalParent (native, e.NewItems);
				break;
			}
			
			if (_presenter != null && _presenter._elementRoot != null) {

				Panel panel = _presenter._elementRoot;
	
				switch (e.Action) {
				case NotifyCollectionChangedAction.Reset:
					// the list has gone away, so clear the children of the panel
					RemoveItemsFromPresenter (0, panel.Children.Count);
					break;
				case NotifyCollectionChangedAction.Add:
					AddItemsToPresenter (e.NewItems, e.NewStartingIndex);
					break;
				case NotifyCollectionChangedAction.Remove:
					RemoveItemsFromPresenter (e.OldStartingIndex, e.OldItems.Count);
					break;
				case NotifyCollectionChangedAction.Replace:
					RemoveItemsFromPresenter (e.OldStartingIndex, e.OldItems.Count);
					AddItemsToPresenter (e.NewItems, e.NewStartingIndex);
					break;
				}
			}
			
			if (!itemsIsDataBound)
				OnItemsChanged (e);
		}
		
		void SetLogicalParent (IntPtr parent, IList items)
		{
			if (ItemsSource != null)
				return;
			foreach (object o in items) {
				DependencyObject dep = o as DependencyObject;
				if (dep != null)
					Mono.NativeMethods.dependency_object_set_parent (dep.native, parent);
				
				FrameworkElement el = o as FrameworkElement;
				if (el != null)
					Mono.NativeMethods.framework_element_set_logical_parent (el.native, parent);
			}
		}

		void AddItemsToPresenter (IList newItems, int newIndex)
		{
			if (_presenter == null || _presenter._elementRoot == null)
				return;

			Panel panel = _presenter._elementRoot;
			for (int i = 0; i < newItems.Count; i ++) {
				object item = newItems[i];
				DependencyObject container = null;
				
				if (IsItemItsOwnContainerOverride (item)) {
					container = (DependencyObject) item;
				}
				else {
					container = GetContainerForItemOverride ();
					
					ContentControl c = container as ContentControl;
					if (c != null)
						c.ContentSetsParent = false;
					
					FrameworkElement f = container as FrameworkElement;
					if (f != null && !(item is FrameworkElement))
						f.DataContext  = item;
				}

				PrepareContainerForItemOverride (container, item);
				panel.Children.Insert (newIndex + i, (UIElement) container);
				ContainerToItems.Add (container, item);
			}
		}
		
		void RemoveItemsFromPresenter (int index, int count)
		{
			if (_presenter == null || _presenter._elementRoot == null)
				return;

			Panel panel = _presenter._elementRoot;
			while (count-- > 0) {
				DependencyObject container = panel.Children [index + count];
				object item = ContainerToItems [container];
				try {
					ClearContainerForItemOverride (container, item);
				} finally {
					panel.Children.RemoveAt (index + count);
					ContainerToItems.Remove (container);
				}
			}
		}

		protected virtual void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			if (element == item)
				return;

			ContentPresenter presenter = element as ContentPresenter;
			ContentControl control = element as ContentControl;

			if (DisplayMemberPath != null && ItemTemplate != null)
				throw new InvalidOperationException ("Cannot set 'DisplayMemberPath' and 'ItemTemplate' simultaenously");

			DataTemplate template = null;
			if (!(item is UIElement)) {
				template = ItemTemplate;
				if (template == null)
					template = DisplayMemberTemplate;
			}

			if (presenter != null) {
				presenter.ContentTemplate = template;
				presenter.Content = item;
			} else if (control != null) {
				control.ContentTemplate = template;
				control.Content = item;
			}
		}

		public ItemCollection Items {
			get {
				if (items == null) {
					items = new ItemCollection ();
					itemsIsDataBound = false;
					items.ItemsChanged += InvokeItemsChanged;
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
				if (!itemsIsDataBound && Items.Count > 0) {
					throw new InvalidOperationException ("Items collection must be empty before using ItemsSource");
				}
				SetValue (ItemsSourceProperty, value);
			}
		}

		public DataTemplate ItemTemplate { 
			get { return (DataTemplate) GetValue (ItemTemplateProperty); }
			set { SetValue (ItemTemplateProperty, value); }
		}

		public ItemContainerGenerator ItemContainerGenerator { get; private set; }
	}
}
