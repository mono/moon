// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Collections; 
using System.Collections.Generic; 
using System.Collections.ObjectModel;
using System.Collections.Specialized; 
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media; 
using System.Windows.Automation.Peers; 
 
namespace System.Windows.Controls
{
    /// <summary>
    /// Control that implements a list of selectable items. 
    /// </summary> 
    [TemplatePart(Name = Selector.TemplateScrollViewerName, Type = typeof(ScrollViewer))]
    public class ListBox : Selector
    {
        /// <summary>
        /// Gets or sets the Style that is applied to the container element generated for each item.
        /// </summary> 
        public Style ItemContainerStyle 
        {
            get { return (Style)GetValue(ItemContainerStyleProperty); } 
            set { SetValue(ItemContainerStyleProperty, value); }
        }
 
        /// <summary>
        /// Identifies the ItemContainerStyle dependency property.
        /// </summary> 
        public static readonly DependencyProperty ItemContainerStyleProperty = DependencyProperty.RegisterCore( 
                "ItemContainerStyle", typeof(Style), typeof(ListBox),
                new PropertyMetadata(new PropertyChangedCallback(OnItemContainerStyleChanged))); 

        /// <summary>
        /// Identifies the IsSelectionActive dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsSelectionActiveProperty = DependencyProperty.RegisterReadOnlyCore (
            "IsSelectionActive", typeof(bool), typeof(ListBox), 
            new PropertyMetadata(new PropertyChangedCallback(OnIsSelectionActiveChanged))); 

        /// <summary>
        /// Tracks the ListBoxItem that just lost focus. 
        /// </summary>
        private ListBoxItem _listBoxItemOldFocus;
 
        /// <summary> 
        /// Tracks whether to suppress the next "lost focus" event because it was self-caused.
        /// </summary> 
        private bool _suppressNextLostFocus;

        /// <summary> 
        /// Tracks the index of the focused element.
        /// </summary>
        private int _focusedIndex = -1; 
 
        /// <summary>
        /// Gets a value that indicates whether the keyboard focus is within the ListBox. 
        /// </summary>
        /// <param name="element">The element from which to read the attached property.</param>
        /// <returns>Value of the property, true if the keyboard focus is within the Selector.</returns> 
        public static bool GetIsSelectionActive(DependencyObject element)
        {
            if (null == element) 
            { 
                throw new ArgumentNullException("element");
            } 
            return (bool)element.GetValue(IsSelectionActiveProperty);
        }
 
        /// <summary>
        /// Sets a value that indicates whether the keyboard focus is within the ListBox.
        /// </summary> 
        /// <param name="element">The element on which to set the attached property.</param> 
        /// <param name="value">The value to set.</param>
        private static void SetIsSelectionActive(ListBox box, bool value) 
        {
            box.SetValueImpl (IsSelectionActiveProperty, value);
        }
 
        /// <summary> 
        /// Initializes a new instance of the ListBox class.
        /// </summary> 
        public ListBox()
        {
        DefaultStyleKey = typeof (ListBox);
#if WPF 
            KeyboardNavigation.SetDirectionalNavigation(this, KeyboardNavigationMode.Contained);
            KeyboardNavigation.SetTabNavigation(this, KeyboardNavigationMode.Once);
            Focusable = true; 
#else 
            // DirectionalNavigation not supported by Silverlight
            // Focusable not supported by Silverlight
#endif
        } 

        /// <summary>
        /// Determines if the specified item is (or is eligible to be) its own container. 
        /// </summary>
        /// <param name="item">The item to check.</param>
        /// <returns>Returns true if the item is (or is eligible to be) its own container; otherwise, false.</returns> 
        protected override bool IsItemItsOwnContainerOverride(object item) 
        {
            return (item is ListBoxItem); 
        }

        /// <summary> 
        /// Creates or identifies the element that is used to display the given item.
        /// </summary>
        /// <returns>The element that is used to display the given item.</returns> 
        protected override DependencyObject GetContainerForItemOverride() 
        {
#if WPF 
            return new ListBoxItem();
#else
            // 
            ListBoxItem listBoxItem = new ListBoxItem();
            if (null != ItemContainerStyle)
            { 
                listBoxItem.Style = ItemContainerStyle; 
            }
            return listBoxItem; 
#endif
        }
 
        /// <summary>
        /// Prepares the specified element to display the specified item.
        /// </summary> 
        /// <param name="element">Element used to display the specified item.</param> 
        /// <param name="item">Specified item.</param>
        protected override void PrepareContainerForItemOverride(DependencyObject element, object item) 
        {
            base.PrepareContainerForItemOverride(element, item);
            ListBoxItem listBoxItem = (ListBoxItem) element; 
            Debug.Assert(null != listBoxItem);
            // Prepare the ListBoxItem state
            listBoxItem.ParentSelector = this; 
            // Prepare the ListBoxItem wrapper 
            bool setContent = true;
            listBoxItem.Item = item;
            if (listBoxItem != item) 
            {
                // If not a ListBoxItem, propagate the ListBox's ItemTemplate
                if (null != ItemTemplate) 
                {
                    // ItemsControl owns recreating containers if ItemTemplate ever changes
                    listBoxItem.ContentTemplate = ItemTemplate; 
                } 
#if !WPF
                else if (!string.IsNullOrEmpty(DisplayMemberPath)) 
                {
                    // Create a binding for displaying the DisplayMemberPath (which always renders as a string)
                    Binding binding = new Binding(DisplayMemberPath); 
                    binding.Converter = new DisplayMemberValueConverter();
                    listBoxItem.SetBinding(ContentControl.ContentProperty, binding);
                    setContent = false; 
                } 
#endif
                // Push the item into the ListBoxItem container 
                if (setContent)
                { 
                    listBoxItem.Content = item;
                }
            } 
            // Apply ItemContainerStyle
            if ((null != ItemContainerStyle) && (null == listBoxItem.Style)) 
            {
                // Silverlight does not support cascading styles, so only use ItemContainerStyle
                // if a style is not already set on the ListBoxItem 
                listBoxItem.Style = ItemContainerStyle;
            }
            // If IsSelected, select the new item 
            if (listBoxItem.IsSelected) 
            {
                SelectedItem = listBoxItem.Item; 
            }
        }

        /// <summary> 
        /// Undoes the effects of PrepareContainerForItemOverride.
        /// </summary>
        /// <param name="element">The container element.</param> 
        /// <param name="item">The item.</param> 
        protected override void ClearContainerForItemOverride(DependencyObject element, object item)
        { 
            base.ClearContainerForItemOverride(element, item);
            ListBoxItem listBoxItem = element as ListBoxItem;
            Debug.Assert(null != listBoxItem); 
#if !WPF
            // Silverlight bug workaround
            if (null == item) 
            { 
                item = listBoxItem.Item;
            } 
#endif
            // If necessary, unselect the selected item that is being removed
            if (listBoxItem.Item == SelectedItem) 
            {
                SelectedItem = null;
            } 
            // Clear the ListBoxItem state 
            listBoxItem.IsSelected = false;
            listBoxItem.ParentSelector = null; 
        } 

        /// <summary> 
        /// Called when the control got focus.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected override void OnGotFocus(RoutedEventArgs e) 
        {
            SetIsSelectionActive(this, true); 
        }

        /// <summary> 
        /// Called when the control lost focus.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected override void OnLostFocus(RoutedEventArgs e) 
        {
            SetIsSelectionActive(this, false); 
            if (_suppressNextLostFocus)
            {
                // Suppressed 
                _suppressNextLostFocus = false;
            }
            else 
            { 
                // Focus is leaving the ListBox; stop tracking the previous ListBoxItem
                _listBoxItemOldFocus = null; 
            }
        }

        /// <summary> 
        /// Causes the object to scroll into view.
        /// </summary>
        /// <param name="item">Object to scroll.</param> 
        public void ScrollIntoView(object item)
        {
            if ((null != TemplateScrollViewer) && Items.Contains(item)) 
            { 
                Rect itemsHostRect;
                Rect listBoxItemRect; 
                if (!IsOnCurrentPage(item, out itemsHostRect, out listBoxItemRect))
                {
                    if (IsVerticalOrientation()) 
                    {
                        // Scroll into view vertically (first make the right bound visible, then the left)
                        double verticalOffset = TemplateScrollViewer.VerticalOffset; 
                        double verticalDelta = 0; 
                        if (itemsHostRect.Bottom < listBoxItemRect.Bottom)
                        { 
                            verticalDelta = listBoxItemRect.Bottom - itemsHostRect.Bottom;
                            verticalOffset += verticalDelta;
                        } 
                        if (listBoxItemRect.Top - verticalDelta < itemsHostRect.Top)
                        {
                            verticalOffset -= itemsHostRect.Top - (listBoxItemRect.Top - verticalDelta); 
                        } 
                        TemplateScrollViewer.ScrollToVerticalOffset(verticalOffset);
                    } 
                    else
                    {
                        // Scroll into view horizontally (first make the bottom bound visible, then the top) 
                        double horizontalOffset = TemplateScrollViewer.HorizontalOffset;
                        double horizontalDelta = 0;
                        if (itemsHostRect.Right < listBoxItemRect.Right) 
                        { 
                            horizontalDelta = listBoxItemRect.Right - itemsHostRect.Right;
                            horizontalOffset += horizontalDelta; 
                        }
                        if (listBoxItemRect.Left - horizontalDelta < itemsHostRect.Left)
                        { 
                            horizontalOffset -= itemsHostRect.Left - (listBoxItemRect.Left - horizontalDelta);
                        }
                        TemplateScrollViewer.ScrollToHorizontalOffset(horizontalOffset); 
                    } 
                }
            } 
        }

        /// <summary> 
        /// Called by ListBoxItem instances when they are clicked.
        /// </summary>
        /// <param name="listBoxItem">The ListBoxItem.</param> 
        internal override void NotifyListItemClicked(ListBoxItem listBoxItem) 
        {
            if (listBoxItem.IsSelected)
            {
                Console.WriteLine ("I'm already selected");
                if (ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)) 
                {
                    Console.WriteLine ("Unselected");
                    SelectedItem = null;
                } 
            } 
            else
            { 
                Console.WriteLine ("Selecting: {0}/{1}", listBoxItem.Name, listBoxItem.Content);
                object item = listBoxItem.Item;
                SelectedItem = item;
                ScrollIntoView(item); 
            }
        }
 
        /// <summary> 
        /// Called by ListBoxItem instances when they get focus
        /// </summary> 
        /// <param name="listBoxItemNewFocus">ListBoxItem that got focus</param>
        internal override void NotifyListItemGotFocus(ListBoxItem listBoxItemNewFocus)
        { 
            // Track the focused index 
            _focusedIndex = Items.IndexOf(listBoxItemNewFocus.Item);
            _listBoxItemOldFocus = null;
        } 
 
        /// <summary>
        /// Called by ListBoxItem instances when they lose focus 
        /// </summary>
        /// <param name="listBoxItemOldFocus">ListBoxItem that lost focus</param>
        internal override void NotifyListItemLostFocus(ListBoxItem listBoxItemOldFocus) 
        {
            // Stop tracking state
            _focusedIndex = -1; 
            _listBoxItemOldFocus = listBoxItemOldFocus; 
        }

        /// <summary>
        /// Responds to the KeyDown event. 
        /// </summary> 
        /// <param name="e">Provides data for KeyEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity", Justification="Straightforward switch-based key handling method that barely triggers the warning")] 
        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (!e.Handled) 
            {
                bool handled = false;
                int newFocusedIndex = -1; 
                switch (e.Key) 
                {
                    case Key.Space: 
                    case Key.Enter:
                        if ((Key.Enter != e.Key) || KeyboardNavigation.GetAcceptsReturn(this))
                        { 
                            if (ModifierKeys.Alt != (Keyboard.Modifiers & (ModifierKeys.Control | ModifierKeys.Alt)))
                            {
                                // KeyEventArgs.OriginalSource (used by WPF) isn't available in Silverlight; use FocusManager.GetFocusedElement instead 
                                ListBoxItem listBoxItem = FocusManager.GetFocusedElement() as ListBoxItem;
                                if (null != listBoxItem) 
                                {
                                    if ((ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)) && listBoxItem.IsSelected)
                                    { 
                                        SelectedItem = null; 
                                    }
                                    else 
                                    {
                                        SelectedItem  = listBoxItem.Item;
                                    } 
                                    handled = true;
                                }
                            }
                        } 
                        break;
                    case Key.Home: 
                        newFocusedIndex = 0;
                        break;
                    case Key.End: 
                        newFocusedIndex = Items.Count - 1;
                        break;
                    case Key.PageUp: 
                        newFocusedIndex = NavigateByPage(false); 
                        break;
                    case Key.PageDown: 
                        newFocusedIndex = NavigateByPage(true);
                        break;
                    case Key.Left: 
                        if (IsVerticalOrientation())
                        {
                            ElementScrollViewerScrollInDirection(Key.Left); 
                        } 
                        else
                        { 
                            newFocusedIndex = _focusedIndex - 1;
                        }
                        break; 
                    case Key.Up:
                        if (IsVerticalOrientation())
                        { 
                            newFocusedIndex = _focusedIndex - 1; 
                        }
                        else 
                        {
                            ElementScrollViewerScrollInDirection(Key.Up);
                        } 
                        break;
                    case Key.Right:
                        if (IsVerticalOrientation()) 
                        { 
                            ElementScrollViewerScrollInDirection(Key.Right);
                        } 
                        else
                        {
                            newFocusedIndex = _focusedIndex + 1; 
                        }
                        break;
                    case Key.Down: 
                        if (IsVerticalOrientation()) 
                        {
                            newFocusedIndex = _focusedIndex + 1; 
                        }
                        else
                        { 
                            ElementScrollViewerScrollInDirection(Key.Down);
                        }
                        break; 
                    // case Key.Divide:  // Feature only used by SelectionMode.Extended 
                    // case Key.Oem2:  // Key not supported by Silverlight
                    //    break; 
                    // case Key.Oem5:  // Key not supported by Silverlight
                    //     break;
                    default: 
                        Debug.Assert(!handled);
                        break;
                } 
                if ((-1 != newFocusedIndex) && 
                    (-1 != _focusedIndex) &&
                    (newFocusedIndex != _focusedIndex) && 
                    (0 <= newFocusedIndex) &&
                    (newFocusedIndex < Items.Count))
                { 
                    // A key press will change the focused ListBoxItem
                    ListBoxItem listBoxItem = GetContainerItem (newFocusedIndex);
                    Debug.Assert(null != listBoxItem); 
                    ScrollIntoView(listBoxItem.Item);
                    _suppressNextLostFocus = true;
                    if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control) {
                        listBoxItem.Focus();
                    } else {
                        SelectedItem = listBoxItem.Item;
                    }
                    handled = true;
                } 
                if (handled)
                {
                    e.Handled = true; 
                } 
            }
        } 

        /// <summary>
        /// Call ElementScrollViewer.ScrollInDirection if possible. 
        /// </summary>
        /// <param name="key">Key corresponding to the direction.</param>
        private void ElementScrollViewerScrollInDirection(Key key) 
        { 
            if (null != TemplateScrollViewer)
            { 
                TemplateScrollViewer.ScrollInDirection(key);
            }
        } 

        /// <summary>
        /// Implements the ItemContainerStyleProperty PropertyChangedCallback. 
        /// </summary> 
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnItemContainerStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(Style).IsInstanceOfType(e.OldValue) || (null == e.OldValue));
            Debug.Assert(typeof(Style).IsInstanceOfType(e.NewValue) || (null == e.NewValue)); 
            listBox.OnItemContainerStyleChanged((Style)e.OldValue, (Style)e.NewValue); 
        }
 
        /// <summary>
        /// Called when the ItemContainerStyle property has changed.
        /// </summary> 
        /// <param name="oldItemContainerStyle">The value of the property before the change.</param>
        /// <param name="newItemContainerStyle">The value of the property after the change.</param>
        void OnItemContainerStyleChanged(Style oldItemContainerStyle, Style newItemContainerStyle) 
        { 
            for (int i = 0; i < Items.Count; i++)
            { 
                ListBoxItem listBoxItem = GetContainerItem (i);
                if (null != listBoxItem)  // May be null if GetContainerForItemOverride has not been called yet
                { 
                    if ((null == listBoxItem.Style) || (oldItemContainerStyle == listBoxItem.Style))
                    {
                        // Silverlight does not support cascading styles, so only use the new value 
                        // if it will replace the old value 
#if !WPF
                        if (null != listBoxItem.Style) 
                        {
                            throw new NotSupportedException(Resource.ListBox_OnItemContainerStyleChanged_CanNotSetStyle);
                        } 
#endif
                        listBoxItem.Style = newItemContainerStyle;
                    } 
                } 
            }
        } 

        /// <summary>
        /// Implements the IsSelectionActive PropertyChangedCallback. 
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsSelectionActiveChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ListBox listBox = d as ListBox; 
            if (listBox == null)
                return;
            
            Debug.Assert(typeof(bool).IsInstanceOfType(e.OldValue));
            Debug.Assert(typeof(bool).IsInstanceOfType(e.NewValue)); 
            listBox.OnIsSelectionActiveChanged((bool)e.OldValue, (bool)e.NewValue);
        }
 
        /// <summary> 
        /// Called when the IsSelectionActive property has changed.
        /// </summary> 
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param>
        void OnIsSelectionActiveChanged(bool oldValue, bool newValue) 
        {
            if (null != SelectedItem) 
            {
                ListBoxItem selectedListBoxItem = GetContainerItem (Items.IndexOf (SelectedItem)); 
                if (null != selectedListBoxItem)
                {
                    selectedListBoxItem.ChangeVisualState(); 
                 }
            }
        }

        /// <summary>
        /// Indicate whether the orientation of the ListBox's items is vertical.
        /// </summary> 
        /// <returns>True if the orientation is vertical; false otherwise.</returns>
        private bool IsVerticalOrientation()
        {
            DependencyObject x = VisualTreeHelper.GetChild (this, 0);
            x = VisualTreeHelper.GetChild(x, 0);
            StackPanel stackPanel = x as StackPanel; 
            return (null == stackPanel) ? true : (stackPanel.Orientation == Orientation.Vertical);
        } 

        /// <summary>
        /// Indicate whether the specified item is currently visible. 
        /// </summary>
        /// <param name="item">The item.</param>
        /// <returns>True if the item is visible; false otherwise.</returns> 
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks> 
        private bool IsOnCurrentPage(object item)
        { 
            Rect itemsHostRect;
            Rect listBoxItemRect;
            return IsOnCurrentPage(item, out itemsHostRect, out listBoxItemRect); 
        }

        /// <summary> 
        /// Indicate whether the specified item is currently visible. 
        /// </summary>
        /// <param name="item">The item.</param> 
        /// <param name="itemsHostRect">Rect for the item host element.</param>
        /// <param name="listBoxItemRect">Rect for the ListBoxItem element.</param>
        /// <returns>True if the item is visible; false otherwise.</returns> 
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks>
        private bool IsOnCurrentPage(object item, out Rect itemsHostRect, out Rect listBoxItemRect)
        { 
            ListBoxItem listBoxItem = GetContainerItem (Items.IndexOf (item));
            Debug.Assert(null != listBoxItem);
            // Get Rect for item host element 
            DependencyObject ItemsHost = VisualTreeHelper.GetChild(this, 0);
            ItemsHost = VisualTreeHelper.GetChild(ItemsHost, 0);
            FrameworkElement itemsHost =
                (null != TemplateScrollViewer) ?
                    ((null != TemplateScrollViewer.ElementScrollContentPresenter) ? TemplateScrollViewer.ElementScrollContentPresenter as FrameworkElement : TemplateScrollViewer as FrameworkElement) : 
                    ItemsHost as FrameworkElement;
            Debug.Assert(null != itemsHost);
            itemsHostRect = new Rect(new Point(), new Point(itemsHost.RenderSize.Width, itemsHost.RenderSize.Height)); 
            // Adjust Rect to account for padding 
            Control itemsHostControl = itemsHost as Control;
            if (null != itemsHostControl) 
            {
                Thickness padding = itemsHostControl.Padding;
                itemsHostRect = new Rect( 
                    itemsHostRect.Left + padding.Left,
                    itemsHostRect.Top + padding.Top,
                    itemsHostRect.Width - padding.Left - padding.Right, 
                    itemsHostRect.Height - padding.Top - padding.Bottom); 
            }
            // Get relative Rect for ListBoxItem 
            GeneralTransform generalTransform = listBoxItem.TransformToVisual(itemsHost);
            listBoxItemRect = new Rect(generalTransform.Transform(new Point()), generalTransform.Transform(new Point(listBoxItem.RenderSize.Width, listBoxItem.RenderSize.Height)));
            // Return result 
            return (IsVerticalOrientation() ?
                (itemsHostRect.Top <= listBoxItemRect.Top) && (listBoxItemRect.Bottom <= itemsHostRect.Bottom) :
                (itemsHostRect.Left <= listBoxItemRect.Left) && (listBoxItemRect.Right <= itemsHostRect.Right)); 
        } 

        /// <summary> 
        /// Get the first visible item.
        /// </summary>
        /// <param name="startingIndex">Starting index to search from.</param> 
        /// <param name="forward">Search forward if true; backward otherwise.</param>
        /// <returns>Index of first visible item.</returns>
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks> 
        private int GetFirstItemOnCurrentPage(int startingIndex, bool forward) 
        {
            int delta = (forward ? 1 : -1); 
            int firstItemOnCurrentPage = -1;
            int probeIndex = startingIndex;
            // Scan looking for the first visible element 
            while ((0 <= probeIndex) && (probeIndex < Items.Count) && !IsOnCurrentPage(Items[probeIndex]))
            {
                firstItemOnCurrentPage = probeIndex; 
                probeIndex += delta; 
            }
            // Then scan looking for the last visible element 
            while ((0 <= probeIndex) && (probeIndex < Items.Count) && IsOnCurrentPage(Items[probeIndex]))
            {
                firstItemOnCurrentPage = probeIndex; 
                probeIndex += delta;
            }
            return firstItemOnCurrentPage; 
        } 

        /// <summary> 
        /// Move the focus forward/backward one page.
        /// </summary>
        /// <param name="forward">Forward if true; backward otherwise</param> 
        /// <returns>New focused index.</returns>
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks>
        private int NavigateByPage(bool forward) 
        { 
            int newFocusedIndex = -1;
            object item = (-1 != _focusedIndex) ? Items[_focusedIndex] : null; 
            // Get it visible to start with
            if ((null != item) && !IsOnCurrentPage(item))
            { 
                ScrollIntoView(item);
                if (null != TemplateScrollViewer)
                { 
                    TemplateScrollViewer.UpdateLayout(); 
                }
            } 
            // Inlined implementation of NavigateByPageInternal
            if (null == item)
            { 
                // Select something
                newFocusedIndex = GetFirstItemOnCurrentPage(_focusedIndex, forward);
            } 
            else 
            {
                int firstItemOnCurrentPage = GetFirstItemOnCurrentPage(_focusedIndex, forward); 
                if (firstItemOnCurrentPage != _focusedIndex)
                {
                    // Select the "edge" element 
                    newFocusedIndex = firstItemOnCurrentPage;
                }
                else 
                { 
                    if (null != TemplateScrollViewer)
                    { 
                        // Scroll a page in the relevant direction
                        if (IsVerticalOrientation())
                        { 
                            TemplateScrollViewer.ScrollToVerticalOffset(Math.Max(0, Math.Min(TemplateScrollViewer.ScrollableHeight,
                                TemplateScrollViewer.VerticalOffset + (TemplateScrollViewer.ViewportHeight * (forward ? 1 : -1)))));
                        } 
                        else 
                        {
                            TemplateScrollViewer.ScrollToHorizontalOffset(Math.Max(0, Math.Min(TemplateScrollViewer.ScrollableWidth, 
                                TemplateScrollViewer.HorizontalOffset + (TemplateScrollViewer.ViewportWidth * (forward ? 1 : -1)))));
                        }
                        TemplateScrollViewer.UpdateLayout(); 
                    }
                    // Select the "edge" element
                    newFocusedIndex = GetFirstItemOnCurrentPage(_focusedIndex, forward); 
                } 
            }
            return newFocusedIndex; 
        }

#if WPF 
        /// <summary>
        /// Expose WPF's internal ItemsControl.ItemsHost member for private use (it is public in Silverlight).
        /// </summary> 
        private Panel ItemsHost 
        {
            get 
            {
                System.Reflection.PropertyInfo propertyInfo = GetType().BaseType.GetProperty("ItemsHost", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
                return propertyInfo.GetValue(this, null) as Panel; 
            }
        }
#endif 

	protected override AutomationPeer OnCreateAutomationPeer ()
	{
		return new ListBoxAutomationPeer (this);
	}
    } 
}
