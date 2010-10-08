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
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls {

	[ContentPropertyAttribute("Items", true)]
	public class ItemsControl : Control, IListenCollectionChanged {
		
		public static readonly DependencyProperty DisplayMemberPathProperty =
			DependencyProperty.RegisterCore ("DisplayMemberPath", typeof (string), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (DisplayMemberPathChanged)));
		public static readonly DependencyProperty ItemsPanelProperty =
			DependencyProperty.RegisterCore ("ItemsPanel", typeof (ItemsPanelTemplate), typeof (ItemsControl), null);
		public static readonly DependencyProperty ItemsSourceProperty =
			DependencyProperty.RegisterCore ("ItemsSource", typeof (IEnumerable), typeof (ItemsControl),
						     new PropertyMetadata (null, new PropertyChangedCallback (ItemsSourceChanged)));
		public static readonly DependencyProperty ItemTemplateProperty =
			DependencyProperty.RegisterCore ("ItemTemplate", typeof (DataTemplate), typeof (ItemsControl), new PropertyMetadata (ItemTemplateChanged));

		static void ItemTemplateChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			((ItemsControl) sender).OnItemTemplateChanged ((DataTemplate) e.OldValue, (DataTemplate) e.NewValue);
		}

		public static ItemsControl GetItemsOwner (DependencyObject element)
		{
			Panel panel = element as Panel;
			if (panel == null || !panel.IsItemsHost)
				return null;

			return panel.TemplateOwner as ItemsControl;
		}

		public static ItemsControl ItemsControlFromItemContainer (DependencyObject element)
		{
			var e = element as FrameworkElement;
			if (e == null)
				return null;

			var itctl = e.Parent as ItemsControl;
			if (itctl == null)
				return GetItemsOwner (e.Parent);
			if (itctl.IsItemItsOwnContainer (e))
				return itctl;
			return null;
		}

		DataTemplate displayMemberTemplate;
		private bool itemsIsDataBound;
		private ItemCollection items;
		private ItemsPresenter _presenter;
		
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

		internal Panel Panel {
			get { return _presenter == null ? null : _presenter._elementRoot; }
		}

		IWeakListener CollectionListener {
			get; set;
		}

		public ItemsControl ()
		{
			DefaultStyleKey = typeof (ItemsControl);
			ItemContainerGenerator = new ItemContainerGenerator (this);
			ItemContainerGenerator.ItemsChanged += OnItemContainerGeneratorChanged;
		}

		internal override UIElement GetDefaultTemplate ()
		{
			ItemsPresenter presenter = _presenter;
			if (presenter == null) {
				presenter = new ItemsPresenter ();
				presenter.TemplateOwner = this;
			}
			return presenter;
		}

		internal void SetItemsPresenter (ItemsPresenter presenter)
		{
			if (_presenter != null)
				_presenter._elementRoot.Children.Clear ();
			_presenter = presenter;
			AddItemsToPresenter (new GeneratorPosition (-1, 1), Items.Count);
		}

		static void ItemsSourceChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((ItemsControl) o).OnItemsSourceChanged (e.OldValue as IEnumerable,
								 e.NewValue as IEnumerable);
		}

		internal virtual void OnItemsSourceChanged (IEnumerable oldSource, IEnumerable newSource)
		{
			if (CollectionListener != null) {
				CollectionListener.Detach ();
				CollectionListener = null;
			}

			if (newSource != null) {
				if (newSource is INotifyCollectionChanged) {
					CollectionListener = new WeakCollectionChangedListener (((INotifyCollectionChanged)newSource), this);
				}
				
				Items.ClearImpl ();
				Items.SetIsReadOnly (true);
				itemsIsDataBound = true;
				
				foreach (var v in newSource)
					Items.AddImpl (v);
				
				// Setting itemsIsDataBound to true prevents normal notifications from propagating, so do it manually here
				OnItemsChanged (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
			} else {
				itemsIsDataBound = false;
				Items.SetIsReadOnly (false);		
				Items.ClearImpl ();
			}
			// Yes this is stupid and shouldn't be here, but DRT 348 sets an empty collection as the ItemsSource
			// and expects the LayoutUpdated event to be raised. This is the only way that makes sense for this
			// to happen. It's all very strange.
			InvalidateMeasure ();
		}

		void IListenCollectionChanged.CollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
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
					Items.SetItemImpl (e.NewStartingIndex + i, e.NewItems[i]);
				break;
			case NotifyCollectionChangedAction.Reset:
				Items.ClearImpl ();
				foreach (var v in ItemsSource)
					Items.AddImpl (v);
				break;
			}
			OnItemsChanged (e);
		}
		
		static void DisplayMemberPathChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((ItemsControl) o).OnDisplayMemberPathChanged (e.OldValue as string,
								       e.NewValue as string);
		}

		void OnDisplayMemberPathChanged (string oldPath, string newPath)
		{
			// refresh the display member template.
			displayMemberTemplate = null;
			var newTemplate = DisplayMemberTemplate;

			int count = Items.Count;
			for (int i = 0; i < count; i ++) {
				UpdateContentTemplateOnContainer (ItemContainerGenerator.ContainerFromIndex (i), items [i]);
			}
		}

		internal void ClearContainerForItem (DependencyObject element, object item)
		{
			ClearContainerForItemOverride (element, item);
		}

		protected virtual void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			// nothing to undo by default (since nothing was prepared)
		}

		internal DependencyObject GetContainerForItem ()
		{
			return GetContainerForItemOverride ();
		}

		protected virtual DependencyObject GetContainerForItemOverride ()
		{
			return new ContentPresenter ();
		}

		internal bool IsItemItsOwnContainer (object item)
		{
			return IsItemItsOwnContainerOverride (item);
		}

		protected virtual bool IsItemItsOwnContainerOverride (object item)
		{
			return (item is FrameworkElement);
		}

		protected virtual void OnItemsChanged (NotifyCollectionChangedEventArgs e)
		{
			
		}

		void OnItemsClearing (object o, EventArgs e)
		{
			SetLogicalParent (IntPtr.Zero, Items);
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
			
			ItemContainerGenerator.OnOwnerItemsItemsChanged (o, e);
			if (!itemsIsDataBound)
				OnItemsChanged (e);
		}

		void OnItemContainerGeneratorChanged (object sender, ItemsChangedEventArgs e)
		{
			if (_presenter == null || _presenter._elementRoot is VirtualizingPanel)
				return;

			Panel panel = _presenter._elementRoot;
			switch (e.Action) {
			case NotifyCollectionChangedAction.Reset:
				// the list has gone away, so clear the children of the panel
				if (panel.Children.Count > 0)
					RemoveItemsFromPresenter (new GeneratorPosition (0, 0), panel.Children.Count);
				break;
			case NotifyCollectionChangedAction.Add:
				AddItemsToPresenter (e.Position, e.ItemCount);
				break;
			case NotifyCollectionChangedAction.Remove:
				RemoveItemsFromPresenter (e.Position, e.ItemCount);
				break;
			case NotifyCollectionChangedAction.Replace:
				RemoveItemsFromPresenter (e.Position, e.ItemCount);
				AddItemsToPresenter (e.Position, e.ItemCount);
				break;
			}
		}

		internal virtual void OnItemTemplateChanged (DataTemplate oldValue, DataTemplate newValue)
		{
			int count = Items.Count;
			for (int i = 0; i < count; i++) {
				UpdateContentTemplateOnContainer (ItemContainerGenerator.ContainerFromIndex (i), Items [i]);
			}
		}

		void SetLogicalParent (IntPtr parent, IList items)
		{
			if (ItemsSource != null)
				return;
			foreach (object o in items) {
				FrameworkElement el = o as FrameworkElement;
				if (el != null)
					Mono.NativeMethods.framework_element_set_logical_parent (el.native, parent);
			}
		}

		void AddItemsToPresenter (GeneratorPosition position, int count)
		{
			if (_presenter == null || _presenter._elementRoot == null || _presenter._elementRoot is VirtualizingPanel)
				return;
			
			Panel panel = _presenter._elementRoot;
			int newIndex = ItemContainerGenerator.IndexFromGeneratorPosition (position);

			using (var p = ItemContainerGenerator.StartAt (position, GeneratorDirection.Forward, true))
			for (int i = 0; i < count; i ++) {
				var item = Items [newIndex + i];
				DependencyObject container = null;
				
				bool fresh;
				container = ItemContainerGenerator.GenerateNext (out fresh);
				ContentControl c = container as ContentControl;
				if (c != null)
					c.ContentSetsParent = false;
				
				FrameworkElement f = container as FrameworkElement;
				if (f != null && !(item is FrameworkElement))
					f.DataContext  = item;

				panel.Children.Insert (newIndex + i, (UIElement) container);
				ItemContainerGenerator.PrepareItemContainer (container);
			}
		}
		
		void RemoveItemsFromPresenter (GeneratorPosition position, int count)
		{
			if (_presenter == null || _presenter._elementRoot == null || _presenter._elementRoot is VirtualizingPanel)
				return;

			Panel panel = _presenter._elementRoot;
			while (count-- > 0)
				panel.Children.RemoveAt (position.Index);
		}

		internal void PrepareContainerForItem (DependencyObject element, object item)
		{
			PrepareContainerForItemOverride (element, item);
		}

		protected virtual void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			if (DisplayMemberPath != null && ItemTemplate != null)
				throw new InvalidOperationException ("Cannot set 'DisplayMemberPath' and 'ItemTemplate' simultaenously");

			UpdateContentTemplateOnContainer (element, item);
		}

		void UpdateContentTemplateOnContainer (DependencyObject element, object item)
		{
			if (element == item)
				return;

			ContentPresenter presenter = element as ContentPresenter;
			ContentControl control = element as ContentControl;

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
					items.Clearing +=  OnItemsClearing;
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
