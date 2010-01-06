//
// Selector.cs
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
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windows.Input;

namespace System.Windows.Controls.Primitives {
	public abstract class Selector : ItemsControl {
		internal const string TemplateScrollViewerName = "ScrollViewer";

		internal static readonly DependencyProperty IsSelectionActiveProperty =
			DependencyProperty.RegisterReadOnlyCore ("IsSelectionActive", typeof(bool), typeof(Selector), null); 
		
		public static readonly DependencyProperty IsSynchronizedWithCurrentItemProperty =
			DependencyProperty.Register ("IsSynchronizedWithCurrentItem", typeof(bool?), typeof(Selector),
						     new PropertyMetadata (null, new PropertyChangedCallback (OnIsSynchronizedWithCurrentItemChanged)));
		
		static void OnIsSynchronizedWithCurrentItemChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((Selector) o).IsSynchronizedWithCurrentItemChanged (o, e);
		}
		
		public static readonly DependencyProperty SelectedIndexProperty =
			DependencyProperty.RegisterCore ("SelectedIndex", typeof(int), typeof(Selector),
						     new PropertyMetadata(-1, new PropertyChangedCallback(OnSelectedIndexChanged)));

		static void OnSelectedIndexChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((Selector) o).SelectedIndexChanged (o, e);
		}

		// This is not a core property because it is a non-parenting property
		public static readonly DependencyProperty SelectedItemProperty =
			DependencyProperty.Register ("SelectedItem", typeof(object), typeof(Selector),
						     new PropertyMetadata(new PropertyChangedCallback(OnSelectedItemChanged_cb)));

		
		static void OnSelectedItemChanged_cb (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((Selector) o).SelectedItemChanged (o, e);
		}

		internal static void OnItemContainerStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			Selector s = (Selector) d;
			Style style = (Style) e.NewValue;

			int count = s.Items.Count;
			for (int i = 0; i < count; i++)
			{ 
				ListBoxItem item = (ListBoxItem) s.GetContainerItem (i);
				if (item != null)  // May be null if GetContainerForItemOverride has not been called yet
					item.Style = style;
			}	
		}

		internal Selector ()
		{
			// Set default values for ScrollViewer attached properties 
			ScrollViewer.SetHorizontalScrollBarVisibility(this, ScrollBarVisibility.Auto);
			ScrollViewer.SetVerticalScrollBarVisibility(this, ScrollBarVisibility.Auto);
		}
		
		bool SynchronizeWithCurrentItem {
			get {
				bool? sync = IsSynchronizedWithCurrentItem;
				
				return (ItemsSource is ICollectionView) && (!sync.HasValue || sync.Value);
			}
		}
		
		bool Changing {
			get; set;
		}

		internal bool IsSelectionActive {
			get { return (bool) GetValue (IsSelectionActiveProperty); }
			set { SetValueImpl (IsSelectionActiveProperty, value); }
		}

		[TypeConverter (typeof (NullableBoolConverter))]
		public bool? IsSynchronizedWithCurrentItem {
			get { return (bool?) GetValue (IsSynchronizedWithCurrentItemProperty); }
			set {
				if (value.HasValue && value.Value)
					throw new ArgumentException ();
				
				SetValue (IsSynchronizedWithCurrentItemProperty, value);
			}
		}

		[Mono.Xaml.SetPropertyDelayed]
		public int SelectedIndex {
			get { return (int)GetValue(SelectedIndexProperty); }
			set { SetValue (SelectedIndexProperty, value); }
		}

		public object SelectedItem {
			get { return GetValue (SelectedItemProperty); }
			set { SetValue (SelectedItemProperty, value); }
		}

		internal ScrollViewer TemplateScrollViewer {
			get; private set;
		}
		
		void OnCurrentItemChanged (object sender, EventArgs args)
		{
			if (SynchronizeWithCurrentItem)
				SelectedItem = (ItemsSource as ICollectionView).CurrentItem;
		}
		
		internal override void OnItemsSourceChanged (IEnumerable oldSource, IEnumerable newSource)
		{
			if (oldSource is ICollectionView)
				(oldSource as ICollectionView).CurrentChanged -= OnCurrentItemChanged;
			
			if (newSource is ICollectionView)
				(newSource as ICollectionView).CurrentChanged += OnCurrentItemChanged;
			
			base.OnItemsSourceChanged (oldSource, newSource);
		}
		
		public event SelectionChangedEventHandler SelectionChanged;
		
		void IsSynchronizedWithCurrentItemChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			bool? sync = (bool?) e.NewValue;
			
			if ((!sync.HasValue || sync.Value) && ItemsSource is ICollectionView) {
				ICollectionView view = ItemsSource as ICollectionView;
				
				// synchronize with the current item
				view.MoveCurrentTo (SelectedItem);
			}
		}
		
		void SelectedIndexChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			object oldItem = SelectedItem;
			int newVal = (int) e.NewValue;
			if (newVal == (int) e.OldValue || Changing) {
				SelectedIndex = newVal;
				return;
			}

			Changing = true;
			try {
				if (newVal < 0)
					ClearValue (SelectedItemProperty);
				else if (newVal < Items.Count)
					SelectedItem = Items [newVal];
			} finally {
				Changing = false;
			}
			RaiseSelectionChanged (o, new SelectionChangedEventArgs (oldItem, SelectedItem));
		}
		
		void SelectedItemChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			if (e.NewValue == e.OldValue || Changing) {
				SelectedItem = e.NewValue;
				return;
			}
			
			Changing = true;
			try {
				int index = e.NewValue == null ? -1 : Items.IndexOf (e.NewValue);
				if (index == -1 && e.NewValue != null) {
					SelectedIndex = e.OldValue == null ? -1 : Items.IndexOf (e.OldValue);
					if (e.OldValue == null)
						ClearValue (SelectedItemProperty);
					else
						SelectedItem = e.OldValue;
				}
				else {
					SelectedItem = e.NewValue;
					SelectedIndex = index;
					RaiseSelectionChanged (o, new SelectionChangedEventArgs (e.OldValue, e.NewValue));
				}
			} finally {
				Changing = false;
			}
		}
		
		void OnSelectedItemChanged (object oldValue, object newValue)
		{
			if (oldValue != null) {
				ListBoxItem oldItem;
				if (oldValue is ListBoxItem && IsItemItsOwnContainerOverride (oldValue))
					oldItem = (ListBoxItem) oldValue;
				else
					oldItem = (ListBoxItem) GetContainerItem (Items.IndexOf (oldValue));

				if (oldItem != null)
					oldItem.IsSelected = false;
			}

			if (newValue != null) {
				ListBoxItem newItem;
				if (newValue is ListBoxItem && IsItemItsOwnContainerOverride (newValue))
					newItem = (ListBoxItem) newValue;
				else
					newItem = (ListBoxItem) GetContainerItem (Items.IndexOf (newValue));

				if (newItem != null) {
					newItem.IsSelected = true;
					// FIXME: Sometimes the item should be focused and sometimes it shouldn't
					// I think that the selector won't steal focus from an element which isn't
					// a child of the selector.
					// Testcase:
					// 1) Open the Controls Toolkit.
					// 2) Click on a demo in the treeview
					// 3) Try to shrink the source textbox view.
					// Result: The view requires 2 clicks to collapse it. Subsequent attempts work on the first click.
					// This 'bug' should only happen if you change the source view tab manually, i.e. if you change the
					// source file being displayed you will need two clicks to collapse the view.
					newItem.Focus ();
				}
			}
			
			if (SynchronizeWithCurrentItem)
				(ItemsSource as ICollectionView).MoveCurrentTo (newValue);
		}

		void RaiseSelectionChanged (object o, SelectionChangedEventArgs e)
		{
			object oldVal = e.RemovedItems.Count == 1 ? e.RemovedItems [0] : null;
			object newVal = e.AddedItems.Count == 1 ? e.AddedItems [0] : null;
			OnSelectedItemChanged (oldVal, newVal);
			
			SelectionChangedEventHandler h = SelectionChanged;
			if (h != null)
				h (o, e);
		}

		public static bool GetIsSelectionActive (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			return (bool) element.GetValue (ListBox.IsSelectionActiveProperty);
		}

		protected override void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			base.ClearContainerForItemOverride (element, item);
			ListBoxItem lbItem = (ListBoxItem) element;
			lbItem.ParentSelector = null;
			if (element != item)
				lbItem.Content = null;
			if (SelectedItem == item && GetContainerItem (SelectedIndex) != null)
				SelectedItem = null;
		}

		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			base.PrepareContainerForItemOverride (element, item);
			ListBoxItem listBoxItem = (ListBoxItem) element; 
			listBoxItem.ParentSelector = this; 
			listBoxItem.Item = item;
			if (listBoxItem.IsSelected && GetContainerItem (SelectedIndex) != null)
				SelectedItem = item;
		}

		public override void OnApplyTemplate ()
		{
			base.OnApplyTemplate ();
			TemplateScrollViewer = GetTemplateChild("ScrollViewer") as ScrollViewer;
			
			if (TemplateScrollViewer != null)
			{
				TemplateScrollViewer.TemplatedParentHandlesScrolling = true;
				// Update ScrollViewer values
				TemplateScrollViewer.HorizontalScrollBarVisibility = ScrollViewer.GetHorizontalScrollBarVisibility(this); 
				TemplateScrollViewer.VerticalScrollBarVisibility = ScrollViewer.GetVerticalScrollBarVisibility(this); 
			}
		}

		protected override void OnItemsChanged (NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				ListBoxItem item = e.NewItems [0] as ListBoxItem;
				if (item != null && item.IsSelected) {
					SelectedItem = item;
				} else {
					// Ensure we don't fire a SelectionChanged event when we're just updating the index
					Changing = true;
					if (e.NewStartingIndex <= SelectedIndex)
						SelectedIndex ++;
					Changing = false;
				}
				break;
			case NotifyCollectionChangedAction.Reset:
				SelectedIndex = -1;
				break;
				
			case NotifyCollectionChangedAction.Remove:
				if (e.OldItems [0] == SelectedItem) {
					SelectedItem = null;
					SelectedIndex = -1;
				} else if (e.OldStartingIndex <= SelectedIndex) {
					SelectedIndex --;
				}
				break;
			case NotifyCollectionChangedAction.Replace:
				if (e.OldItems [0] == SelectedItem)
					SelectedItem = null;
				break;
			default:
				throw new NotSupportedException (string.Format ("Collection changed action '{0}' not supported", e.Action));
				break;
			}
			base.OnItemsChanged (e);
		}
		
		internal virtual void NotifyListItemClicked(ListBoxItem listBoxItem) 
		{
			if (ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)) {
				if (SelectedItem == listBoxItem.Item)
					SelectedItem = null;
			} else {
				SelectedItem = listBoxItem.Item;
			}
		}
		
		internal virtual void NotifyListItemLoaded (ListBoxItem listBoxItem)
		{
			if (listBoxItem.Item == SelectedItem) {
				listBoxItem.IsSelected = true;
				listBoxItem.Focus ();
			}
		}
		
		internal virtual void NotifyListItemGotFocus(ListBoxItem listBoxItemNewFocus)
		{
			
		}
		
		internal virtual void NotifyListItemLostFocus(ListBoxItem listBoxItemOldFocus)
		{
			
		}
	}
}
